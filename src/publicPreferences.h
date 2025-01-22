#pragma once

#define EVERY_MS(x)                  \
  static uint32_t tmr;               \
  bool flag = millis() - tmr >= (x); \
  if (flag)                          \
    tmr += (x);                      \
  if (flag)

#define PWM_PIN D7
#define OUT_SWITCH_PIN D1
#define LED_WORK_PIN D3
#define SDA D5
#define SCL D6

#define PWM_RESOLUTION 12
#define DIVIDER_SOL_BAT 21.2151
#define DIVIDER_SYS_BAT 10.937
#define SOL_BAT_SHUNT_RESISTANCE 0.0145
#define HYSTERESIS 0.15
#define MIN_BAT_VOL 10.5
#define MIN_VOL_TO_CONNECT_BAT 11.5
#define SAVEMODE_STARTS 0
#define SAVEMODE_ENDS 6

#define ADDR_IS_EEPROM_CLEANING 4090

#define Celsius // unit of measurement of temperature
// #define Fahrenheit
// #define Kelvin

#define millimetreOfMercury // unit of measurement of pressure
// #define KPa
// #define bar

#define HOUR_UPDATE_DATA 0 // multiple 4!!!

#define WIFI_SSID "your Wi-Fi ssid"
#define WIFI_PASS "your Wi-Fi pass"
#define AP_SSID "AP"
#define AP_PASS "AP_PASS" // without if empty

#define BOT_TOKEN "your bot token"
#define ADMIN_CHAT_ID "your chat"
#define UTC_ZONE 2

#define LCD
//#define DEBUG_WEATHER_SENSORS
//#define DEBUG_ADC
//#define DEBUG_EEPROM

#ifndef DEBUG_EEPROM
//    #define EEPROM_CLEANING
#endif