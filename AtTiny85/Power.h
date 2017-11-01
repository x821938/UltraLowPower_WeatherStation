#ifndef _POWERSAVE_h
#define _POWERSAVE_h

#include <Arduino.h>

#define ESP_RESET_PIN 1			// Pin number connected to ESP reset pin. This is for waking it up.
#define SENSOR_POWER_PIN 4
#define SLEEP_PERIOD 8			// seconds to sleep (1 or 8)



void gotoDeepSleep( uint16_t seconds );
void resetWatchdog();

void wakeESP();
void sensorPower( bool powerOn );

#endif

