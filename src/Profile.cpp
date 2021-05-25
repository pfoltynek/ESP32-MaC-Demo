#include "Profile.h"

Profile::Profile()
{
	
}

bool Profile::cardMount()
{
	if (!SD.begin()) 
	{
		Serial.println("Card Mount Failed");
		return false;
	}

	uint8_t cardType = SD.cardType();

	if (cardType == CARD_NONE) 
	{
		Serial.println("No SD card attached");
		return false;
	}

	return true;
}

bool Profile::wifi(wifiProfile &wifiProfile)
{
	const char* path = "/profile/wifi.conf";
	if(!SD.exists(path))
	{
		Serial.printf("File %s not exist\n", path);
		return false;
	}

	File file = SD.open(path);
	if (!file) 
	{
		Serial.println("Failed to open file for reading");
		return false;
	}

	// Allocate the memory pool on the stack.
	StaticJsonBuffer<300> jsonBuffer;

	// Parse the root object
	JsonObject &root = jsonBuffer.parseObject(file);

	if (!root.success())
	{
		Serial.println(F("Failed to read file, using default configuration"));
	}

	strlcpy(wifiProfile.ssid, root["ssid"] | "None", sizeof(wifiProfile.ssid));
	strlcpy(wifiProfile.password, root["password"] | "None", sizeof(wifiProfile.password));

	file.close();

	wifiProfile.isDefault = false;

	return true;
}

bool Profile::mqtt(mqttProfile &mqttProfile)
{
const char* path = "/profile/mqtt.conf";
	if(!SD.exists(path))
	{
		Serial.printf("File %s not exist\n", path);
		return false;
	}

	File file = SD.open(path);
	if (!file) 
	{
		Serial.println("Failed to open file for reading");
		return false;
	}

	// Allocate the memory pool on the stack.
	StaticJsonBuffer<300> jsonBuffer;

	// Parse the root object
	JsonObject &root = jsonBuffer.parseObject(file);

	if (!root.success())
	{
		Serial.println(F("Failed to read file, using default configuration"));
	}

	strlcpy(mqttProfile.server, root["server"] | "None", sizeof(mqttProfile.server));
	mqttProfile.port = root["port"] | 0;

	file.close();

	mqttProfile.isDefault = false;

	return true;
}