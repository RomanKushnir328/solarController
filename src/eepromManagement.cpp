#include "eepromManagement.h"
#include "botManagement.h"
#include "hardwareManagement.h"

extern FB_Time timeNow;
extern Weather weatherNow;
wifiValues wifiValuesNow;

#ifndef DEBUG_EEPROM
static bool isUpdateEeprom = true;
static bool isYearDataUpdate = true;
static bool isUpdateMonthYearData = true;
static Weather getAverangeWeather(int16_t cellBegin, byte number)
{
  Weather temp;
  Weather averange;
  for (int16_t i = cellBegin; i < cellBegin + number * 16; i += 16)
  {
    EEPROM.get(i, temp);
    averange.temperature += temp.temperature;
    averange.sunPower += temp.sunPower;
    averange.humidity += temp.humidity;
    averange.pressure += temp.pressure;
  }
  averange.temperature = averange.temperature / number;
  averange.humidity = averange.humidity / number;
  averange.pressure = averange.pressure / number;
  return averange;
}

static void movingDataInMemory(int16_t cellBegin, byte number, Weather newLastInstance)
{
  Weather temp;
  for (int16_t i = cellBegin + 16; i < cellBegin + (number + 1) * 16; i += 16)
  {
    EEPROM.get(i, temp);
    EEPROM.put(i - 16, temp);
  }
  EEPROM.put(cellBegin + number * 16, newLastInstance);
}

#endif

void startEeprom(void)
{
  EEPROM.begin(4096);
#ifndef DEBUG_EEPROM
  #ifdef EEPROM_CLEANING
    EEPROM.put(ADDR_IS_EEPROM_CLEANING, 255);
    EEPROM.commit();
    delay(10);
  #endif
  if (EEPROM.read(ADDR_IS_EEPROM_CLEANING) == 255)
  {
    for (int16_t i = 0; i < 4096; i++)
    {
      EEPROM.put(i, 0);
    }
    EEPROM.put(ADDR_WIFI_VALUES, WIFI_SSID);
    EEPROM.put(ADDR_WIFI_VALUES+64, WIFI_PASS);
    EEPROM.put(ADDR_WIFI_VALUES+128, AP_SSID);
    EEPROM.put(ADDR_WIFI_VALUES+192, AP_PASS);
    Weather weatherMax;
    weatherMax.temperature = std::numeric_limits<float>::lowest();
    weatherMax.sunPower = std::numeric_limits<float>::lowest();
    weatherMax.pressure = std::numeric_limits<float>::lowest();
    EEPROM.put(ADDR_MAX_INDEXES, weatherMax);
    Weather weatherMin;
    weatherMin.temperature = std::numeric_limits<float>::max();
    weatherMin.humidity = std::numeric_limits<float>::max();
    weatherMin.pressure = std::numeric_limits<float>::max();
    EEPROM.put(ADDR_MAX_INDEXES, weatherMax);
    EEPROM.put(ADDR_MAX_INDEXES + 16, weatherMin);
    EEPROM.commit();
  }
#endif
  EEPROM.get(ADDR_WIFI_VALUES, wifiValuesNow);
}

void eepromManage(void)
{
#ifndef DEBUG_EEPROM
  static Weather sumHourWeather;
  static uint16_t numberOfMeasurements = 0;
  if (numberOfMeasurements == 0)
  {
    sumHourWeather.sunPower = 0.0;
    sumHourWeather.temperature = 0.0;
    sumHourWeather.pressure = 0.0;
    sumHourWeather.humidity = 0.0;
  }
  sumHourWeather.temperature += weatherNow.temperature;
  sumHourWeather.sunPower += weatherNow.sunPower;
  sumHourWeather.pressure += weatherNow.pressure;
  sumHourWeather.humidity += weatherNow.humidity;
  numberOfMeasurements++;

  Weather weatherMax;
  EEPROM.get(ADDR_MAX_INDEXES, weatherMax);
  Weather weatherMin;
  EEPROM.get(ADDR_MAX_INDEXES + 16, weatherMin);

  if (weatherNow.sunPower > weatherMax.sunPower)
  {
    weatherMax.sunPower = weatherNow.sunPower;
    EEPROM.put(ADDR_MAX_INDEXES, weatherMax);
  }
  if (weatherNow.temperature > weatherMax.temperature)
  {
    weatherMax.temperature = weatherNow.temperature;
    EEPROM.put(ADDR_MAX_INDEXES, weatherMax);
  }
  if (weatherNow.pressure > weatherMax.pressure)
  {
    weatherMax.pressure = weatherNow.pressure;
    EEPROM.put(ADDR_MAX_INDEXES, weatherMax);
  }

  if (weatherNow.temperature < weatherMin.temperature)
  {
    weatherMin.temperature = weatherNow.temperature;
    EEPROM.put(ADDR_MAX_INDEXES + 16, weatherMin);
  }
  if (weatherNow.humidity < weatherMin.humidity)
  {
    weatherMin.humidity = weatherNow.humidity;
    EEPROM.put(ADDR_MAX_INDEXES + 16, weatherMin);
  }
  if (weatherNow.pressure < weatherMin.pressure)
  {
    weatherMin.pressure = weatherNow.pressure;
    EEPROM.put(ADDR_MAX_INDEXES + 16, weatherMin);
  }

  if (timeNow.minute == 0 && isUpdateEeprom)
  {
    isUpdateEeprom = false;

    digitalWrite(LED_BUILTIN, LOW);

    Weather averangeHourWeather;
    averangeHourWeather.temperature = sumHourWeather.temperature / numberOfMeasurements;
    averangeHourWeather.sunPower = sumHourWeather.sunPower / numberOfMeasurements;
    averangeHourWeather.pressure = sumHourWeather.pressure / numberOfMeasurements;
    averangeHourWeather.humidity = sumHourWeather.humidity / numberOfMeasurements;
    movingDataInMemory(ADDR_DAY_DATA, 24, averangeHourWeather);
    numberOfMeasurements = 0;

    if (timeNow.hour == HOUR_UPDATE_DATA && isUpdateMonthYearData)
    {
      isUpdateMonthYearData = false;

      movingDataInMemory(ADDR_MONTH_DATA, 30, getAverangeWeather(ADDR_WEEK_DATA + 16 * 35, 6));

      int16_t lastTimeUpdateYearData;
      EEPROM.get(ADDR_LAST_TIME_UPDATE_YEAR_DATA, lastTimeUpdateYearData);
      int16_t dayOfTheYear = 0;
      bool isLeapYear = (timeNow.year % 4 == 0 && (timeNow.year % 100 != 0 || timeNow.year % 400 == 0));
      for (byte i = 1; i < timeNow.month; i++)
      {
        if (i == 2)
        {
          if (isLeapYear)
          {
            dayOfTheYear += getMonthLength(timeNow.year, i - 1) + 1;
          }
          else
          {
            dayOfTheYear += getMonthLength(timeNow.year, i - 1);
          }
        }
        else
        {
          dayOfTheYear += getMonthLength(timeNow.year, i - 1);
        }
      }
      dayOfTheYear += timeNow.day;
      byte differenceBetween = (dayOfTheYear < lastTimeUpdateYearData ? isLeapYear ? dayOfTheYear + 366 : dayOfTheYear + 365 : dayOfTheYear) - lastTimeUpdateYearData;

      if (differenceBetween >= 5 && isYearDataUpdate)
      {
        isYearDataUpdate = false;
        EEPROM.put(ADDR_LAST_TIME_UPDATE_YEAR_DATA, dayOfTheYear);
        Weather averangeWeatherForFiveDays = getAverangeWeather(ADDR_WEEK_DATA + 16 * 11, 30);
        movingDataInMemory(ADDR_YEAR_DATA, 73, averangeWeatherForFiveDays);

        Weather averangeWeatherOfAllTime;
        averangeWeatherOfAllTime.temperature = 0.0;
        averangeWeatherOfAllTime.humidity = 0.0;
        averangeWeatherOfAllTime.pressure = 0.0;

        Weather lastAverangeWeatherOfAllTime;
        EEPROM.get(ADDR_AVERANGE_INDEXES, lastAverangeWeatherOfAllTime);

        Weather sumAverangeWeather;
        sumAverangeWeather.temperature = 0.0;
        sumAverangeWeather.humidity = 0.0;
        sumAverangeWeather.pressure = 0.0;

        uint16_t numberOfMeasurementsAverangeWeather;
        EEPROM.get(ADDR_AVERANGE_INDEXES + 16, numberOfMeasurementsAverangeWeather);

        sumAverangeWeather.temperature = lastAverangeWeatherOfAllTime.temperature * numberOfMeasurementsAverangeWeather + averangeWeatherForFiveDays.temperature;
        sumAverangeWeather.humidity = lastAverangeWeatherOfAllTime.humidity * numberOfMeasurementsAverangeWeather + averangeWeatherForFiveDays.humidity;
        sumAverangeWeather.pressure = lastAverangeWeatherOfAllTime.pressure * numberOfMeasurementsAverangeWeather + averangeWeatherForFiveDays.pressure;

        averangeWeatherOfAllTime.temperature = sumAverangeWeather.temperature / (numberOfMeasurementsAverangeWeather + 1);
        averangeWeatherOfAllTime.humidity = sumAverangeWeather.humidity / (numberOfMeasurementsAverangeWeather + 1);
        averangeWeatherOfAllTime.pressure = sumAverangeWeather.pressure / (numberOfMeasurementsAverangeWeather + 1);

        EEPROM.put(ADDR_AVERANGE_INDEXES, averangeWeatherOfAllTime);
        EEPROM.put(ADDR_AVERANGE_INDEXES + 16, ++numberOfMeasurementsAverangeWeather);
      }
    }
    else if (timeNow.hour != HOUR_UPDATE_DATA)
    {
      isUpdateMonthYearData = true;
    }

    if (timeNow.hour % 4 == 0)
    {
      if (timeNow.hour == HOUR_UPDATE_DATA)
      {
        isYearDataUpdate = true;
      }
      movingDataInMemory(ADDR_WEEK_DATA, 42, getAverangeWeather(ADDR_DAY_DATA + 16 * 20, 4));
      EEPROM.commit();
      delay(100);
    }

    digitalWrite(LED_BUILTIN, HIGH);
  }
  else if (timeNow.minute != 0)
  {
    isUpdateEeprom = true;
  }
#endif
}

byte getMonthLength(int16_t year, byte month)
{
  bool isLeapYear = (year % 4 == 0 && (year % 100 != 0 || year % 400 == 0));
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
    return isLeapYear ? 29 : 28;
  default:
    return 255;
  }
}

String getDayData(void)
{
  String result;
  Weather tempWeather;
  byte hourMultipleOf_4 = timeNow.hour - (timeNow.hour % 4);
  result += "Time  sunPower temperature humidity pressure \n";
  for (byte i = 0; i < 24; i++)
  {
    EEPROM.get(ADDR_DAY_DATA + i * 16, tempWeather);
    byte tempHour = (hourMultipleOf_4 + i) % 24 + 1;
    result += String(tempHour) + ":00" + (tempHour > 9 ? "      " : "        ") + String(tempWeather.sunPower) + (tempWeather.sunPower > 9.99 ? "           " : "             ") + String(tempWeather.temperature) + (tempWeather.temperature > 9.99 || tempWeather.temperature < -9.99 ? "           " : "             ") + String(tempWeather.humidity) + "      " + String(tempWeather.pressure) + "\n";
  }
  return result;
}

String getWeekData(void)
{
  String result;
  Weather tempWeather;
  FB_Time tempTime;
  byte hourMultipleOf_4 = timeNow.hour - (timeNow.hour % 4);
  tempTime.month = timeNow.month;
  tempTime.day = timeNow.day;
  if (timeNow.day > 7)
  {
    tempTime.day -= 7;
  }
  else
  {
    tempTime.day = getMonthLength(timeNow.year, timeNow.month) + (int(timeNow.day) - 7);
    tempTime.month--;
  }
  tempTime.hour = hourMultipleOf_4;

  result += "Time sunPower temperature humidity pressure \n";
  for (byte i = 0; i < 42; i++)
  {
    EEPROM.get(ADDR_WEEK_DATA + i * 16, tempWeather);
    result += String(tempTime.day) + "." + String(tempTime.month) + " " + String(tempTime.hour) + ":00 \n" +
              String(tempWeather.sunPower) + (tempWeather.sunPower > 9.99 ? "         " : "           ") +
              String(tempWeather.temperature) + (tempWeather.temperature > 9.99 || tempWeather.temperature < -9.99 ? "             " : "               ") +
              String(tempWeather.humidity) + "          " + String(tempWeather.pressure) + "\n";
    tempTime.hour += 4;
    if (tempTime.hour == 24)
    {
      tempTime.hour = 0;
      tempTime.day++;
    }
    if (tempTime.day > getMonthLength(timeNow.year, timeNow.month))
    {
      tempTime.day = 1;
      tempTime.month++;
    }
  }
  return result;
}

String getMonthData(void)
{
  String result;
  Weather tempWeather;
  FB_Time tempTime;
  tempTime.month = timeNow.month;
  tempTime.day = timeNow.day;
  if (timeNow.day > 30)
  {
    tempTime.day -= 30;
  }
  else
  {
    tempTime.day = getMonthLength(timeNow.year, timeNow.month) + (int(timeNow.day) - 29);
    tempTime.month--;
  }
  result += "time sunPower temperature humidity pressure \n";
  for (byte i = 0; i < 30; i++)
  {
    EEPROM.get(ADDR_MONTH_DATA + i * 16, tempWeather);
    result += String(tempTime.day) + "." + String(tempTime.month) +
              (tempTime.day > 9 ? " " : "   ") + (tempTime.month > 9 ? " " : "   ") +
              String(tempWeather.sunPower) + (tempWeather.sunPower > 9.99 ? "         " : "           ") +
              String(tempWeather.temperature) + (tempWeather.temperature > 9.99 || tempWeather.temperature < -9.99 ? "             " : "               ") +
              String(tempWeather.humidity) + "          " + String(tempWeather.pressure) + "\n";
    tempTime.day++;
    if (tempTime.day > getMonthLength(timeNow.year, timeNow.month))
    {
      tempTime.day = 1;
      tempTime.month++;
    }
  }
  return result;
}

String getYearData(void)
{
  String result;
  Weather tempWeather;
  uint16_t lastTimeUpdateYearData;
  EEPROM.get(ADDR_LAST_TIME_UPDATE_YEAR_DATA, lastTimeUpdateYearData);
  result += "Time temperature sunPower humidity pressure \n";
  for (uint16_t i = 0; i < 73; i++)
  {
    EEPROM.get(ADDR_YEAR_DATA + i * 16, tempWeather);
    String(tempWeather.sunPower) + (tempWeather.sunPower > 9.99 ? "         " : "           ") +
        String(tempWeather.temperature) + (tempWeather.temperature > 9.99 || tempWeather.temperature < -9.99 ? "             " : "               ") +
        String(tempWeather.humidity) + "          " + String(tempWeather.pressure) + "\n";
  }
  return result;
}

String getAverangeWeather(void)
{
  String result;
  Weather temp;
  EEPROM.get(ADDR_AVERANGE_INDEXES, temp);
  result += "Temperature => " + String(temp.temperature) + ", humidity => " + String(temp.humidity) + ", pressure => " + String(temp.pressure);
  return result;
}

String getMaxIndexes(void)
{
  String result;
  Weather temp;
  EEPROM.get(ADDR_MAX_INDEXES, temp);
  result += "Highest indexes: \n";
  result += "temperature => " + String(temp.temperature) + ", sunPower => " + String(temp.sunPower) + ", pressure => " + String(temp.pressure) + "\n";
  EEPROM.get(ADDR_MAX_INDEXES + 16, temp);
  result += "Lowest indexes: \n";
  result += "temperature => " + String(temp.temperature) + ", humidity => " + String(temp.humidity) + ", pressure => " + String(temp.pressure);
  return result;
}

wifiValues getWifiValues(void){
 return EEPROM.get(ADDR_WIFI_VALUES, wifiValuesNow);
}