#ifndef _PROFILE_h
#define _PROFILE_h

#include "arduino.h"
#include <ArduinoJson.h>
#include <FS.h>
#include <SD.h>

struct wifiProfile
{
	char ssid[40];
	char password[40];
	bool isDefault = true;
};

struct mqttProfile
{
	char server[40];
	int port;
	bool isDefault = true;
};

class Profile
{
	protected:
		
	public:
		Profile();
		bool cardMount();
		bool wifi(wifiProfile &wifiProfile);
		bool mqtt(mqttProfile &mqttProfile);
};

#endif /* _PROFILE_h */