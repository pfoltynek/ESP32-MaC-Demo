#ifndef _ESP32SENSOR_h
#define _ESP32SENSOR_h

#include "arduino.h"
#include "esp_system.h"

//Import C Function
#ifdef __cplusplus
extern "C" {
#endif

uint8_t temprature_sens_read();

inline String getMacAddressESP32() 
{
	uint8_t baseMac[6];
	// Get MAC address for WiFi station
	esp_read_mac(baseMac, ESP_MAC_WIFI_STA);
	char baseMacChr[18] = {0};
	sprintf(baseMacChr, "%02X:%02X:%02X:%02X:%02X:%02X", baseMac[0], baseMac[1], baseMac[2], baseMac[3], baseMac[4], baseMac[5]);
	return String(baseMacChr);
}

#ifdef __cplusplus
}
#endif

class Esp32Sensor
{
    public:
        static float temprature_sensor();
        static uint8_t temprature_sensor_raw();
};

#endif /* _ESP32SENSOR_h */