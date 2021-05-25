#include "CustomDisplay.h"

void displayWifi(SSD1306 display, const char* ssid, const char* ipAddress, bool connected, const char* progress)
{
	display.clear();
	
	//header
	display.drawString(0, 1, "Connecting to ");
	display.drawString(0, 15, ssid);
	
	//Progress
	display.drawString(0, 22, progress);

	//foote
	if(connected && ipAddress != nullptr)
	{
		display.drawString(0, 33, "WiFi connected ");
		display.drawString(0, 45, "IP address: ");
		display.drawString(55, 45, ipAddress);
	}

	display.display();
}

void displayNotWifi(SSD1306 display)
{
	display.clear();
	//header
	display.drawString(0, 1, "Wifi not connected");
 
	display.display();
}

void displayMqtt(SSD1306 display, const char* ipAddress, const char* server, int port, const char* clientName, int state)
{
	display.clear();

	//header
	display.drawString(0, 1, "IP addr: ");
	display.drawString(40, 1, ipAddress);
	
	//body
	display.drawString(0, 15, "Connecting to MQTT ...");
	String text = server + String(":") + String(port);
	display.drawString(0, 29, text);
   
	//state
	if(state != MQTT_CONNECT_STATE_DEFAULT)
	{
		if(state == 0)
		{
			display.drawString(0, 44, String("Connected " + String(clientName)));
		}
		else
		{
			String text = "Connecting failed (" + String(state) + ")";
			display.drawString(0, 44, text);
		}
	}

	display.display();
}

void displayHeader(SSD1306 display, const char* ipAddress, bool mqtt)
{
	display.drawString(0, 1, "IP addr: ");
	display.drawString(40, 1, ipAddress);
	if (mqtt)
		display.drawString(110, 1, "MQ");
}

void displayMeasurment(SSD1306 display, const char* ipAddress, float ftemperature, bool sending)
{
	display.clear();

	//header
	displayHeader(display, ipAddress, true);

	//body
	display.drawString(0, 20, "Temperature [°C]: ");
	display.drawString(85, 20, (String)ftemperature);

	//sending
	if(sending)
	{
		display.drawString(0, 40, "Sending ...");
	}
	
	display.display();
}

void displaySetup(SSD1306 display, const char* ipAddress, bool mqtt)
{
	display.clear();

	//header
	displayHeader(display, ipAddress, mqtt);
	
	display.display();
}

void displayMesssage(SSD1306 display, const char* ipAddress, const char* messageRow1, const char* messageRow2)
{
	display.clear();

	//header
	displayHeader(display, ipAddress, true);
	
	//body
	display.drawString(0, 20, messageRow1);
	
	if(messageRow2 != NULL)
	{
		display.drawString(0, 34, messageRow2);
	}
	
	display.display();
}

void displayCommand(SSD1306 display, const char* message1, const char* message2)
{
	display.clear();

	//body
	Serial.println(message1);
	display.drawString(0, 20, "Command:");
	display.drawString(0, 34, message1);
	if (message2 != NULL)
	{
		display.drawString(0, 48, message2);
	}
	
	display.display();
}

