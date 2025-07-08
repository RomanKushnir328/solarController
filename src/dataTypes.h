#pragma once
#include <Arduino.h>
#include <limits.h>
#include <vector>
#include <FastBot.h>
#include "publicPreferences.h"

#define EVERY_MS(x)                  \
  static uint32_t tmr;               \
  bool flag = millis() - tmr >= (x); \
  if (flag)                          \
    tmr += (x);                      \
  if (flag)

byte getMonthLength(int16_t year, byte month);

class Timer
{
private:
  uint32_t lastMilli;
  uint32_t lastMicro;

public:
  Timer(uint32_t lastMilli, uint32_t lastMicro) : lastMilli(lastMilli), lastMicro(lastMicro) {}

  void setTimerToZero()
  {
    lastMilli = millis();
    lastMicro = micros();
  }

  bool everyMilli(uint32_t time)
  {
    if (millis() - lastMilli > time)
    {
      lastMilli = millis();
      lastMicro = micros();
      return true;
    }
    return false;
  }

  bool everyMicro(uint32_t time)
  {
    if (micros() - lastMicro > time)
    {
      lastMilli = millis();
      lastMicro = micros();
      return true;
    }
    return false;
  }
};

struct Weather
{
  float sunPower = 0.0;
  float temperature = 0.0;
  float humidity = 0.0;
  float pressure = 0.0;
  String toString(void)
  {
    if (sunPower > 0.0)
    {
      return "Sun power => " + String(sunPower) + "\n" +
             "Temperature => " + String(temperature) + "\n" +
             "Humidity => " + String(humidity) + "\n" +
             "Pressure => " + String(pressure);
    }
    else
    {
      return "Temperature => " + String(temperature) + "\n" +
             "Humidity => " + String(humidity) + "\n" +
             "Pressure => " + String(pressure);
    }
  }
};

struct Voltages
{
  float solBatVol = 0.0;
  float sysBatVol = 0.0;
  float sysBatCur = 0.0;
  float maxPowerPointVol = 18.0;
  float sunPower = 0.0;
  int16_t PWM = 0;
  String toString(void)
  {
    if (sunPower > 0.0)
    {
      return "Solar battery voltage => " + String(solBatVol) + "\n" +
             "System battery voltage => " + String(sysBatVol) + "\n" +
             "System battery current => " + String(sysBatCur) + "\n" +
             "Max power point voltage => " + String(maxPowerPointVol) + "\n" +
             "Sun power => " + String(sunPower) + "\n" +
             "PWM => " + String(PWM);
    }
    else
    {
      return "Solar battery voltage => " + String(solBatVol) + "\n" +
             "System battery voltage => " + String(sysBatVol) + "\n";
    }
  }
};

struct WifiValues
{
  char wifiSsid[64];
  char wifiPassword[64];
  char apSsid[64];
  char apPassword[64];
  String toString(void)
  {
    return String("Wi-Fi ssid => ") + wifiSsid + "\n" +
           String("Wi-Fi password => ") + wifiPassword + "\n" +
           String("AP ssid  => ") + apSsid + "\n" +
           String("AP password => ") + apPassword;
  }
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

  FB_Time subtractHours(FB_Time time, uint16_t hours)
  {
    FB_Time result;
    result.second = 0;
    result.minute = 0;

    uint16_t subtractDays;

    if (time.hour <= hours)
    {
      hours -= time.hour;
      result.hour = 24 - hours % 24;
      subtractDays = (hours - time.hour) / 24 + 1;
      if (time.day <= subtractDays)
      {
        result.year = time.year;
        result.day = 0;
        for (result.month = time.month; result.day == 0 || result.day >= 31; result.month--)
        {
          if (result.month == 0)
          {
            result.year--;
            result.month = 12;
          }
          subtractDays -= getMonthLength(result.year, result.month - 1);
          result.day = time.day - subtractDays;
        }
      }
      else
      {
        result.day = time.day - subtractDays;
        result.month = time.month;
        result.year = time.year;
      }
    }
    else
    {
      result.hour = time.hour - hours;
      result.day = time.day;
      result.month = time.month;
      result.year = time.year;
    }

    return result;
  }

public:
  LastTimeWeather() {}
  LastTimeWeather(std::vector<Weather> &data, LastTimePeriod period, FB_Time time)
  {
    for (uint16_t i = 0; i < data.size(); i++)
    {
      Weather tempWeather = data[i];
      if (tempWeather.sunPower <= 500.0f && tempWeather.humidity > 0.0f && tempWeather.humidity <= 100.0f)
      {
        this->data.push_back(tempWeather);
        switch (period)
        {
        case LAST_DAY:
          labels.push_back(subtractHours(time, 24 - period * i));
          break;
        case LAST_WEEK:
          labels.push_back(subtractHours(time, 168 - period * i));
          break;
        case LAST_MONTH:
          labels.push_back(subtractHours(time, 720 - period * i));
          break;
        case LAST_YEAR:
          labels.push_back(subtractHours(time, 8760 - period * i));
          break;
        }
      }
    }
  }

  String toString()
  {
    String result;
    if (!data.empty() || !labels.empty())
    {
      for (uint16_t i = 0; i < data.size(); i++)
      {
        FB_Time tempTime = labels[i];
        result += tempTime.dateString() + "\n";
        result += tempTime.timeString() + " ";
        Weather tempWeather = data[i];
        result += String(tempWeather.sunPower) + " " + String(tempWeather.temperature) + " " + String(tempWeather.pressure) + " " + String(tempWeather.humidity) + "\n";
      }
    }
    else
    {
      result += "No result";
    }
    return result;
  }

  String dataToString(byte param)
  {
    String result;
    for (Weather temp : data)
    {
      switch (param)
      {
      case 0:
        result += String(temp.sunPower);
        break;
      case 1:
        result += String(temp.temperature);
        break;
      case 2:
        result += String(temp.pressure);
        break;
      case 3:
        result += String(temp.humidity);
        break;
      }
      result += ", ";
    }
    result.remove(result.length() - 2, 2);
    return result;
  }

  String labelsToString()
  {
    String result;
    for (FB_Time temp : labels)
    {
      result += "\"" + temp.dateString() + " " + temp.timeString() + "\"";
      result += ", ";
    }
    result.remove(result.length() - 2, 2);
    return result;
  }
};

#ifdef LCD
enum DisplayMenu
{
  CONNECTION,
  WEATHER,
  VOLTAGES,
};
#endif
