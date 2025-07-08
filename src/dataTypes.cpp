#include <Arduino.h>
#include "dataTypes.h"

#define EVERY_MS(x)                  \
  static uint32_t tmr;               \
  bool flag = millis() - tmr >= (x); \
  if (flag)                          \
    tmr += (x);                      \
  if (flag)

#define IS_LEAP_YEAR(x) x % 4 == 0 && (x % 100 != 0 || x % 400 == 0)

byte getMonthLength(int16_t year, byte month)
{
  switch (month)
  {
  case 1:
  case 3:
  case 5:
  case 7:
  case 8:
  case 10:
  case 12:
    return 31;
  case 4:
  case 6:
  case 9:
  case 11:
    return 30;
  case 2:
    return IS_LEAP_YEAR(year) ? 29 : 28;
  default:
    return 255;
  }
}

Timer::Timer(uint32_t lastMilli, uint32_t lastMicro)
    : lastMilli(lastMilli), lastMicro(lastMicro) {}
void Timer::setTimerToZero()
{
  lastMilli = millis();
  lastMicro = micros();
}

bool Timer::everyMilli(uint32_t time)
{
  if (millis() - lastMilli > time)
  {
    lastMilli = millis();
    lastMicro = micros();
    return true;
  }
  return false;
}

bool Timer::everyMicro(uint32_t time)
{
  if (micros() - lastMicro > time)
  {
    lastMilli = millis();
    lastMicro = micros();
    return true;
  }
  return false;
}

String Weather::toString(void)
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

String Voltages::toString(void)
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

String WifiValues::toString(void)
{
  return String("Wi-Fi ssid => ") + wifiSsid + "\n" +
         String("Wi-Fi password => ") + wifiPassword + "\n" +
         String("AP ssid  => ") + apSsid + "\n" +
         String("AP password => ") + apPassword;
}

uint32_t LastTimeWeather::getUnix(FB_Time time, int16_t timeZone)
{
  uint32_t result = 0;

  for (uint16_t year = 1970; year < time.year; year++)
  {
    result += IS_LEAP_YEAR(year) ? 1000 * 60 * 24 * 366 : 1000 * 60 * 24 * 365;
  }
  for (byte month = 0; month < time.month; month++)
  {
    result += getMonthLength(time.year, month) * 24 * 60 * 60 * 1000;
  }
  result += time.day * 24 * 60 * 60 * 1000;
  result += time.hour * 60 * 60 * 1000;
  result += time.minute * 60 * 1000;
  result += time.second * 1000;
  result -= timeZone*60*60*1000;

  return result;
}

LastTimeWeather::LastTimeWeather(std::vector<Weather> &data, LastTimePeriod period, FB_Time time, int16_t timeZone)
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
        labels.push_back(FB_Time(getUnix(time, timeZone)-(24 - period * i)*60*60*1000, timeZone));
        break;
      case LAST_WEEK:
        labels.push_back(FB_Time(getUnix(time, timeZone)-(168 - period * i)*60*60*1000, timeZone));
        break;
      case LAST_MONTH:
        labels.push_back(FB_Time(getUnix(time, timeZone)-(720 - period * i)*60*60*1000, timeZone));
        break;
      case LAST_YEAR:
        labels.push_back(FB_Time(getUnix(time, timeZone)-(8760 - period * i)*60*60*1000, timeZone));
        break;
      }
    }
  }
}

String LastTimeWeather::toString(void)
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

String LastTimeWeather::dataToString(byte param)
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

String LastTimeWeather::labelsToString(void)
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
