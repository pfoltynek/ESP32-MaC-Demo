// the setup function runs once when you press reset or power the board
#include <WiFi.h>
#include <SPI.h>
#include <SSD1306.h>
#include <PubSubClient.h>
#include <FS.h>
#include "Profile.h"
#include "CustomDisplay.h"
#include "DataWriter.h"
#include "MPU9250.h"
#include "Esp32Sensor.h"

#define CONFIG_FREERTOS_HZ 1000

#define MESSAGE_INFO "esp/info"
#define MESSAGE_SETTING "esp/setting"
#define MESSAGE_SETTING_FREQUENCY "esp/setting/frequency"
#define MESSAGE_SETTING_COUNT "esp/setting/count"

//#define MQTT_MAX_PACKET_SIZE 512
//#define MQTT_SOCKET_TIMEOUT 60
//#define MQTT_KEEPALIVE 60

#define TRY_COUNT	5
#define DISPLAY_ROW	20

#define SPI_CLOCK 8000000	// 8MHz clock works.
#define SS_PIN    15 
MPU9250 mpu(SPI_CLOCK, SS_PIN);

// Initialize the OLED display using Wire library
bool setup_is_init = false;
SSD1306 display(0x3c, 21, 22);
WiFiClient espClient;
PubSubClient client(espClient);
Profile cardProfile;
DataWriter dataWriter;
wifiProfile wifi;
mqttProfile mqtt;
TaskHandle_t coreTaskMeasurmentHandle = NULL;
SemaphoreHandle_t xMutex;
bool is_measurment = false;
TickType_t frequency = 5;
uint16_t count = 5000;


void clientPublish(const char* topic, const char* payload)
{
	if (client.connected())
		client.publish(topic, payload);
}

void clientSubscribe()
{
	//subscribe
	client.subscribe(MESSAGE_SETTING);
	client.subscribe(MESSAGE_SETTING_FREQUENCY);
	client.subscribe(MESSAGE_SETTING_COUNT);
}

bool getIsMeasurment()
{
	xSemaphoreTake(xMutex, portMAX_DELAY);
	const bool value = is_measurment;
	xSemaphoreGive(xMutex);
	return value;
}

void setIsMeasurment(bool value)
{
	xSemaphoreTake(xMutex, portMAX_DELAY);
	is_measurment = value;
	xSemaphoreGive(xMutex);
}

TickType_t getFrequency()
{
	xSemaphoreTake(xMutex, portMAX_DELAY);
	const TickType_t value = frequency;
	xSemaphoreGive(xMutex);
	return value;
}

void setFrequency(TickType_t value)
{
	xSemaphoreTake(xMutex, portMAX_DELAY);
	frequency = value;
	xSemaphoreGive(xMutex);
}

uint16_t getCount()
{
	xSemaphoreTake(xMutex, portMAX_DELAY);
	const uint16_t value = count;
	xSemaphoreGive(xMutex);
	return value;
}

void setCount(uint16_t value)
{
	xSemaphoreTake(xMutex, portMAX_DELAY);
	count = value;
	xSemaphoreGive(xMutex);
}

void coreTaskMeasurment(void * pvParameters)
{
	TickType_t x_last_wake_time = xTaskGetTickCount();
	const TickType_t x_frequency = getFrequency();
	const uint16_t countMax = getCount();
	
	//Info about running task
	String messageInfoTaskCreate = "Task running on core " + String(xPortGetCoreID());
	Serial.println(messageInfoTaskCreate.c_str());
	clientPublish(MESSAGE_INFO, messageInfoTaskCreate.c_str());

	//Create file
	File fileData = dataWriter.createDataFile();

	//Write Header
	String countString = "Number of samples : " + String(countMax);
	String frqString = "Frequency : " + String(x_frequency) + "tick";
	String headerString = "count;millis;accel_X;accel_Y;accel_Z;gyro_X;gyro_Y;gyro_Z;mag_X;mag_Y;mag_Z;teplota";
	dataWriter.appendDataToDataFile(fileData, countString.c_str());
	dataWriter.appendDataToDataFile(fileData, frqString.c_str());
	dataWriter.appendDataToDataFile(fileData, headerString.c_str());

	//Publish esp/info
	String messageFileName = "Measurment start (file is " + String(fileData.name()) + ")";
	clientPublish(MESSAGE_INFO, messageFileName.c_str());


	for (uint16_t count = 0; count < countMax; count++)
	{
		//break cyklus
		//if(!getIsMeasurment())
		//	break;

		mpu.read_all();
		
		//MQTT
		const String mpu_str = String(count) + ";" + \
							   String(x_last_wake_time) + ";" + \
							   String(mpu.accel_data[0]) + ";" + \
							   String(mpu.accel_data[1]) + ";" + \
							   String(mpu.accel_data[2]) + ";" + \
							   String(mpu.gyro_data[0]) + ";" + \
							   String(mpu.gyro_data[1]) + ";" + \
							   String(mpu.gyro_data[2]) + ";" + \
							   String(mpu.mag_data[0]) + ";" + \
							   String(mpu.mag_data[1]) + ";" + \
							   String(mpu.mag_data[2]) + ";" + \
							   String(mpu.temperature);
		
		if((count % DISPLAY_ROW) == 0)
		{
			displayMesssage(display, WiFi.localIP().toString().c_str(), ("Count: " + String(count)).c_str());
		}

		//clientPublish("esp/mpu9250", mpu_str.c_str());
		dataWriter.appendDataToDataFile(fileData, mpu_str.c_str());
		fileData.flush();

		vTaskDelayUntil(&x_last_wake_time, x_frequency);

	}

	//file close
	String messageInfoData = "Close file " + String(fileData.name());
	if(fileData.available())
	{
		Serial.println(messageInfoData.c_str());
		clientPublish(MESSAGE_INFO, messageInfoData.c_str());
		fileData.close();
	}
	
	displayMesssage(display, WiFi.localIP().toString().c_str(), ("Count: " + String(count)).c_str(), messageInfoData.c_str());
	
	//Publish esp/info
	clientPublish(MESSAGE_INFO, "Measurment stop");
	setIsMeasurment(false);

	//Delete task esp/info
	String messageDeleteTask = "Delete task on core " + String(xPortGetCoreID());
	Serial.println(messageDeleteTask.c_str());
	clientPublish(MESSAGE_INFO, messageDeleteTask.c_str());
	vTaskDelete(coreTaskMeasurmentHandle);
}

void callback_mqtt(char* topic, byte* payload, unsigned int length)
{
	Serial.println("-----------------------");
	Serial.println(String("Task running on core " + String(xPortGetCoreID())));
	Serial.println();
	Serial.printf("Message arrived in topic: \"%s\"\r\n", topic);

	//convert payload to char*
	char* buffer = (char*)payload;
	String message = String(buffer).substring(0, length);

	Serial.printf("topic:\"%s\"\n\r", topic);
	Serial.printf("Message(%d):\"%s\"\n\r", length, message.c_str());
	
	if (strcmp(topic,MESSAGE_SETTING) == 0)
	{
		//Dispaly command
		displayMesssage(display, WiFi.localIP().toString().c_str(), message.c_str());

		if (message == "start")
		{
			if(!getIsMeasurment())
			{
				setIsMeasurment(true);
				xTaskCreatePinnedToCore(coreTaskMeasurment,     /* Function to implement the task */
					"coreTaskMeasurment",                       /* Name of the task */
					10000,                                        /* Stack size in words */
					NULL,                                       /* Task input parameter */
					5,                                          /* Priority of the task */
					&coreTaskMeasurmentHandle,                  /* Task handle. */
					0);                                         /* Core where the task should run, man i loop is on core 1 */
			}
		}

		if (message == "stop")
		{
			setIsMeasurment(false);
		}

		if (message == "restart")
		{
			const char* messageRestart = "Restarting in 10 seconds";
			Serial.println(messageRestart);
			displayCommand(display, messageRestart);

			//message

			ESP.restart();
		}

		if (message == "info")
		{
			const int count = getCount();

			Serial.println(count);

			const uint32_t frq = getFrequency();
			String message1 = "Frequency is " + String(frq) + " tiks";
			String message2 = "Max count samp. is " + String(count);
			displayCommand(display, message1.c_str(), message2.c_str());
		}
	}

	if (strcmp(topic,MESSAGE_SETTING_FREQUENCY) == 0)
	{
		uint32_t frq;
		try
		{
			frq = message.toInt();
		}
		catch (...)
		{
			frq = 5;
		}
		
		setFrequency(frq);

		String messageInfo = "Frequency is " + String(frq) + "tiks";
		clientPublish(MESSAGE_INFO,messageInfo.c_str());
	}

	if(strcmp(topic,MESSAGE_SETTING_COUNT) == 0)
	{
		u_int value;
		try
		{
			value = message.toInt();
		}
		catch (...)
		{
			value = 5000;
		}
		
		setCount(value);

		String messageInfo = "Max count is " + String(value);
		clientPublish(MESSAGE_INFO,messageInfo.c_str());
	}

	Serial.println("-----------------------");
}

//https://pubsubclient.knolleary.net/api.html
bool setup_mqtt(bool reconect = false)
{
	if(mqtt.isDefault)
	{
		displayCommand(display, "Profile MQTT is not exist");
		return false;
	}

	if (!reconect)
	{
		//only first connect
		client.setServer(mqtt.server, mqtt.port);
		
		client.setCallback(callback_mqtt);
	}

	while (!client.connected())
	{
		Serial.println();
		Serial.printf("Connecting to MQTT (%s:%d) ...\r\n", mqtt.server, mqtt.port);

		if (!reconect)
			displayMqtt(display, WiFi.localIP().toString().c_str(), mqtt.server, mqtt.port);

		String clientName = "ESP32Client";
		if (client.connect(clientName.c_str()))
		{
			Serial.println("connected");
			if (!reconect)
			{
				displayMqtt(display, WiFi.localIP().toString().c_str(), mqtt.server, mqtt.port, clientName.c_str(), client.state());
				delay(1000);
			}

			//subscribe method
			clientSubscribe();
		}
		else
		{
			Serial.print("failed with state ");
			Serial.println(client.state());
			if (!reconect)
			{
				displayMqtt(display, WiFi.localIP().toString().c_str(), mqtt.server, mqtt.port, clientName.c_str(), client.state());
				delay(3000);
			}
			else
			{
				//Wait 1s
				delay(1000);
			}
		}
	}

	//publish
	if (!reconect)
	{
		clientPublish(MESSAGE_INFO, "MQTT init");
	}
	else
	{
		clientPublish(MESSAGE_INFO, "MQTT reconect");
	}
	
	return true;
}

bool setup_wifi()
{
	if (wifi.isDefault)
	{
		Serial.println("Profile WIFI is not exist");
		return false;
	}

	// We start by connecting to a WiFi network
	Serial.println();
	Serial.print("Connecting to ");
	Serial.println(wifi.ssid);

	displayWifi(display, wifi.ssid);

	WiFi.begin(wifi.ssid, wifi.password);
	byte i = 0;
	byte tryConut = 0;
	char progress[22];
	memset(progress, '\0', sizeof(progress));
	while (WiFi.status() != WL_CONNECTED)
	{
		delay(500);
		Serial.print(".");

		progress[i] = '.';
		displayWifi(display, wifi.ssid, NULL, false, progress);
		i++;

		if (i >= 20)
		{
			memset(progress, '\0', sizeof(progress));
			i = 0;
			tryConut++;
		}

		if (tryConut > TRY_COUNT)
		{
			Serial.print("Wifi not connected");
			displayNotWifi(display);
			return false;
		}
	}

	randomSeed(micros());

	Serial.println("");
	Serial.println("WiFi connected");
	Serial.println("IP address: ");
	Serial.println(WiFi.localIP());

	displayWifi(display, wifi.ssid, WiFi.localIP().toString().c_str(), true);

	return true;
}

bool setup_sdCard()
{
	if (dataWriter.cardMount())
	{
		Serial.printf("Free space: %lluMB\n", dataWriter.getCardFreeSpaceMB());
		if (dataWriter.getCardFreeSpaceMB() < 5)
		{
			Serial.println("SD Card is full");
			return false;
		}
	}
	return true;
}

void setup()
{
	//Serial monitor
	Serial.begin(115200);

	//Create Mutex
	xMutex = xSemaphoreCreateMutex();

	// Initialising the UI will init the display too.
	display.init();
	display.flipScreenVertically();
	display.setFont(ArialMT_Plain_10);

	//Mac address
	String mac = "Mac: " + getMacAddressESP32();

	displayCommand(display, "ready");

	if (!setup_sdCard())
	{
		
		displayCommand(display, "SD Card is not connect", mac.c_str());
	}
	else
	{
		//Read wifi profil from SD Card
		if (!cardProfile.cardMount())
		{
			displayCommand(display, "SD Card Mount Failed", mac.c_str());
		}
		else
		{
			if (cardProfile.wifi(wifi))
			{
				// We start by connecting to a WiFi network
				if (!setup_wifi())
				{
					displayCommand(display, "Wifi setup is not connect", mac.c_str());
					
				}
				else
				{
					if (cardProfile.mqtt(mqtt))
					{
						//Connection MQTT
						if (!setup_mqtt())
						{
							displayCommand(display, "Mqtt profile is not connect", mac.c_str());
							return;
						}
						
						setup_is_init = true;

						displaySetup(display, WiFi.localIP().toString().c_str());

						//MPU9250
						mpu.init(true);

						uint8_t wai = mpu.whoami();
						if (wai == 0x71)
						{
							String status = "Successful connection MPU9250";
							Serial.println(status.c_str());
							clientPublish(MESSAGE_INFO, status.c_str());
						}
						else
						{
							String status = "Failed connection MPU9250: " + String(wai, HEX);
							Serial.println(status.c_str());
							clientPublish(MESSAGE_INFO, status.c_str());
						}

						uint8_t wai_AK8963 = mpu.AK8963_whoami();
						if (wai_AK8963 == 0x48)
						{
							String status = "Successful connection AK8963";
							Serial.println(status.c_str());
							clientPublish(MESSAGE_INFO, status.c_str());
						}
						else
						{
							String status = "Failed connection AK8963: " + String(wai_AK8963, HEX);
							Serial.println(status.c_str());
							clientPublish(MESSAGE_INFO, status.c_str());
						}

						mpu.calib_acc();
						mpu.calib_mag();
					}
				}
			}
		}
	}
}

// the loop function runs over and over again until power down or reset
void loop()
{
	//Serial.print("Priority of the task on core 1 is ");
	//Serial.println(uxTaskPriorityGet(NULL));

	if(!setup_is_init)
		return;

	if (!client.connected())
	{
		//reconect MQTT
		displaySetup(display, WiFi.localIP().toString().c_str(), false);
		if (setup_mqtt(true))
		{
			displaySetup(display, WiFi.localIP().toString().c_str());
		}
	}

	client.loop();
	//delay(5);
}
