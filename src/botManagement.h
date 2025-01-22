#pragma once
#include <Arduino.h>
#include <FastBot.h>
#include "dataTypes.h"
#include "eepromManagement.h"
#include "hardwareManagement.h"

//- telegram bot menu:
//                user
const char menuWeatherNow[] = "/getWeather \t /getSunPower \t /getTemperature \n /getHumidity \t /getPressure \t /menuLastTimeWeather";
const char menuLastTimeWeather[] = "/menuWeatherNow \t /getWeatherLastDay \n /getWeatherLastWeek \t /getWeatherLastMonth \n /getWeatherLastYear \t /getAverangeWeather \n /getMaximumIndexes";

//                admin
const char menuLastTimeWeatherForAdmin[] = "/menuWeatherNow \t /getWeatherLastDay \n /getWeatherLastWeek \t /getWeatherLastMonth \n /getWeatherLastYear \t /getAverangeWeather \n /getMaximumIndexes \t /menuControlHardware";
const char menuControlHardware[] = "/menuLastTimeWeather \t /getVoltages \n /outOff \t /outOn \n /resetMemory \t /restart \n /saveModeOff \t /saveModeOn \n /maxPWM \t /minPWM \n /menuControlWiFi";
const char menuControlWiFi[] = "/menuControlHardware \t /getLocalIP \t /setWifi \n /setAP \t /setTimeZone";

void botConnection(void);
void botManage(void);