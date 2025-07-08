#pragma once
#include <Arduino.h>
#include <FastBot.h>
#include "dataTypes.h"
#include "eepromManagement.h"
#include "hardwareManagement.h"

//- telegram bot menu:
//             user
const char mainMenu[] = "/getWeather \t/getWeatherLastDay \n/getWeatherLastWeek \t/getWeatherLastMonth \n/getWeatherLastYear \t/getAverangeWeather \n/getMaximumIndexes";

//             admin
const char mainMenuForAdmin[] = "/getWeather \t/getWeatherLastDay \n/getWeatherLastWeek \t/getWeatherLastMonth \n/getWeatherLastYear \t/getAverangeWeather \n/getMaximumIndexes \t/menuControl";
const char menuControl[] = "/mainMenu \t /getVoltages \n /resetMemory \t /restart \n /maxPWM \t /minPWM \n /setTimeZone \t /getLocalIP";

void botConnection(void);
void botManage(void);