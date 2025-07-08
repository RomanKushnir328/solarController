#pragma once

#define LCD

//#define DEBUG_EEPROM // send a msg if eeprom commit

#define EEPROM_CLEANING // erase all data from eeprom on load

#define PWM_PIN D7
#define OUT_SWITCH_PIN D1
#define LED_WORK_PIN D3
#define SDA D5
#define SCL D6
#ifdef LCD
#define CH_MENU_BUT D0
#endif

#define PWM_RESOLUTION 10
#define MAX_PWM (1 << (PWM_RESOLUTION)) - 1
#define DIVIDER_SOL_BAT 21.2151
#define DIVIDER_SYS_BAT 10.936
#define SOL_BAT_SHUNT_RESISTANCE 0.0145
#define HYSTERESIS 0.15
#define MIN_BAT_VOL 10.5
#define MIN_VOL_TO_CONNECT_BAT 11.5
#define MAX_SOL_POW 50.0
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
#define UTC_ZONE yourTimezone
