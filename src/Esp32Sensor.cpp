#include "Esp32Sensor.h"

float Esp32Sensor::temprature_sensor()
{
	// Convert raw temperature in F to Celsius degrees
	return ((temprature_sens_read() - 32) / 1.8);
}

uint8_t Esp32Sensor::temprature_sensor_raw()
{
	// raw temperature in F
	return temprature_sens_read();
}

