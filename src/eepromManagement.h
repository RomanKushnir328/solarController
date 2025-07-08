#pragma once
#include <Arduino.h>
#include <EEPROM.h>
#include <FastBot.h>
#include "dataTypes.h"

//EEPROM bitmap
//WiFi 
#define ADDR_WIFI_VALUES 0

//1 year ago
//size(float)*4*73(every 5 day)
#define ADDR_YEAR_DATA 321

//1 month ago
//size(float)*4*30(every day)
#define ADDR_MONTH_DATA 1489

//1 week ago
//size(float)*4*42(every 4 hours)
#define ADDR_WEEK_DATA 1969

//day
//size(float)*4*24(every hour)
#define ADDR_DAY_DATA 2641

//maximumIndexes
//max indexes size(float)*4, min indexes size(float)*4
#define ADDR_MAX_INDEXES 2025

//averangeWeather
//size(float)*4 + size(uint16_t)-number of measurements
#define ADDR_AVERANGE_INDEXES 3057

//size(int16_t)-lastTimeUpdateYearData
#define ADDR_LAST_TIME_UPDATE_YEAR_DATA 3081

//size(int16_t)-timeZone
#define ADDR_TIME_ZONE 3083


void startEeprom(void);
void eepromManage(void);
LastTimeWeather getLastTimeData(LastTimePeriod);
Weather getAverangeWeather(void);
void setWifiValues(WifiValues);
WifiValues getWifiValues(void);
void setTimeZone(int16_t timeZone);
int16_t getTimeZone();