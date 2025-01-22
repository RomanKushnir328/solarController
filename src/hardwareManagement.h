#pragma once
#include <Arduino.h>
#include "dataTypes.h"
#include <Wire.h>

#ifdef LCD
#include <LiquidCrystal_I2C.h>
#endif

#ifndef DEBUG_WEATHER_SENSORS
#include <Adafruit_BMP280.h>
#include <AHT10.h>
#endif

#ifndef DEBUG_ADC
#include <Adafruit_ADS1X15.h>
#endif

#define SYS_BAT_VOL_PIN 0
#define SOL_BAT_VOL_PIN 1
#define SOL_BAT_CURRENT_PIN 2 

#define MAX_PWM (1 << (PWM_RESOLUTION)) - 1

void pinsConnection(void);
void sensorsConnection(void);
void readWeather(void);
void maxPointPowerTracking(void);
void batControl(void);
