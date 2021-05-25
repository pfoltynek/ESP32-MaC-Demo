#ifndef _CUSTOMDISPLAY_h
#define _CUSTOMDISPLAY_h

#include <WiFi.h>
#include <SSD1306.h>

#define MQTT_CONNECT_STATE_DEFAULT -100

void displayWifi(SSD1306 display, const char* ssid, const char* ipAddress = NULL, bool connected = false, const char* progress = "");

void displayNotWifi(SSD1306 display);

void displayMqtt(SSD1306 display, const char* ipAddress, const char* server, int port, const char* clientName = "", int state = MQTT_CONNECT_STATE_DEFAULT);

void displayMeasurment(SSD1306 display, const char* ipAddress, float ftemperature, bool sending = true);

void displayMesssage(SSD1306 display, const char* ipAddress, const char* messageRow1, const char* messageRow2 = NULL);

void displaySetup(SSD1306 display, const char* ipAddress, bool mqtt = true);

void displayCommand(SSD1306 display, const char* message1, const char* message2 = NULL);

#endif /* _CUSTOMDISPLAY_h */