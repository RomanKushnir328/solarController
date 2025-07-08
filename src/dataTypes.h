#pragma once
#include <Arduino.h>
#include <limits.h>
#include <vector>
#include <FastBot.h>
#include "publicPreferences.h"

byte getMonthLength(int16_t year, byte month);

class Timer
{
private:
  uint32_t lastMilli;
  uint32_t lastMicro;

public:
  Timer(uint32_t lastMilli = 0, uint32_t lastMicro = 0);

  void setTimerToZero();
  bool everyMilli(uint32_t time);
  bool everyMicro(uint32_t time);
};

struct Weather
{
  float sunPower = 0.0;
  float temperature = 0.0;
  float humidity = 0.0;
  float pressure = 0.0;
  String toString(void);
};

struct Voltages
{
  float solBatVol = 0.0;
  float sysBatVol = 0.0;
  float sysBatCur = 0.0;
  float maxPowerPointVol = 18.0;
  float sunPower = 0.0;
  int16_t PWM = 0;
  String toString(void);
};

struct WifiValues
{
  char wifiSsid[64];
  char wifiPassword[64];
  char apSsid[64];
  char apPassword[64];
  String toString(void);
};

enum LastTimePeriod
{
  LAST_DAY = 1,
  LAST_WEEK = 4,
  LAST_MONTH = 24,
  LAST_YEAR = 120 // update frequency in hours
};

struct LastTimeWeather
{
private:
  std::vector<Weather> data;
  std::vector<FB_Time> labels;

  uint32_t getUnix(FB_Time time, int16_t utcZone);
public:
  LastTimeWeather() {}
  LastTimeWeather(std::vector<Weather> &data, LastTimePeriod period, FB_Time time, int16_t utcZone);

  String toString();

  String dataToString(byte param);

  String labelsToString();
};

#ifdef LCD
enum DisplayMenu
{
  CONNECTION,
  WEATHER,
  VOLTAGES,
};
#endif
