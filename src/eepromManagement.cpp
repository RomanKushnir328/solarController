#include "eepromManagement.h"
#include "botManagement.h"
#include "hardwareManagement.h"

extern FastBot bot;
extern FB_Time timeNow;
extern Weather weatherNow;
WifiValues wifiValuesNow;

static bool isUpdateEeprom = true;
static bool isYearDataUpdate = true;
static bool isUpdateMonthYearData = true;
Weather weatherMax;
Weather weatherMin;

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

void startEeprom(void)
{
  EEPROM.begin(4096);
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
    EEPROM.put(ADDR_WIFI_VALUES + 64, WIFI_PASS);
    EEPROM.put(ADDR_WIFI_VALUES + 128, AP_SSID);
    EEPROM.put(ADDR_WIFI_VALUES + 192, AP_PASS);
    weatherMax.temperature = std::numeric_limits<float>::lowest();
    weatherMax.sunPower = std::numeric_limits<float>::lowest();
    weatherMax.humidity = std::numeric_limits<float>::lowest();
    weatherMax.pressure = std::numeric_limits<float>::lowest();
    weatherMin.temperature = std::numeric_limits<float>::max();
    weatherMin.humidity = std::numeric_limits<float>::max();
    weatherMin.pressure = std::numeric_limits<float>::max();
    EEPROM.put(ADDR_MAX_INDEXES, weatherMax);
    EEPROM.put(ADDR_MAX_INDEXES + 16, weatherMin);
    EEPROM.put(ADDR_TIME_ZONE, UTC_ZONE);
    EEPROM.commit();
  }
  else
  {
    EEPROM.get(ADDR_MAX_INDEXES, weatherMax);
    EEPROM.get(ADDR_MAX_INDEXES + 16, weatherMin);
  }
  EEPROM.get(ADDR_WIFI_VALUES, wifiValuesNow);
}

void eepromManage(void)
{
  static Weather sumHourWeather;
  static uint16_t numberOfMeasurements = 0;
  static Timer averangeTimer = Timer(millis(), micros());
  static Timer maxTimer = Timer(millis(), micros());

  if (averangeTimer.everyMilli(1000))
  {
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
  }
  if (maxTimer.everyMilli(60000))
  {
    if (weatherNow.sunPower > weatherMax.sunPower)
    {
      weatherMax.sunPower = weatherNow.sunPower;
#ifdef DEBUG_EEPROM
      bot.sendMessage("New max sun power -> " + String(weatherNow.sunPower), ADMIN_CHAT_ID);
#endif
    }

    if (weatherNow.temperature > weatherMax.temperature)
    {
      weatherMax.temperature = weatherNow.temperature;
#ifdef DEBUG_EEPROM
      bot.sendMessage("New max temperature -> " + String(weatherNow.temperature), ADMIN_CHAT_ID);
#endif
    }
    else if (weatherNow.temperature < weatherMin.temperature)
    {
      weatherMin.temperature = weatherNow.temperature;
#ifdef DEBUG_EEPROM
      bot.sendMessage("New min temperature -> " + String(weatherNow.temperature), ADMIN_CHAT_ID);
#endif
    }

    if (weatherNow.humidity > weatherMax.humidity)
    {
      weatherMax.humidity = weatherNow.humidity;
#ifdef DEBUG_EEPROM
      bot.sendMessage("New max humidity -> " + String(weatherNow.humidity), ADMIN_CHAT_ID);
#endif
    }
    else if (weatherNow.humidity < weatherMin.humidity)
    {
      weatherMin.humidity = weatherNow.humidity;
#ifdef DEBUG_EEPROM
      bot.sendMessage("New min humidity -> " + String(weatherNow.humidity), ADMIN_CHAT_ID);
#endif
    }

    if (weatherNow.pressure > weatherMax.pressure)
    {
      weatherMax.pressure = weatherNow.pressure;
#ifdef DEBUG_EEPROM
      bot.sendMessage("New max pressure -> " + String(weatherNow.pressure), ADMIN_CHAT_ID);
#endif
    }
    else if (weatherNow.pressure < weatherMin.pressure)
    {
      weatherMin.pressure = weatherNow.pressure;
#ifdef DEBUG_EEPROM
      bot.sendMessage("New min pressure -> " + String(weatherNow.pressure), ADMIN_CHAT_ID);
#endif
    }
  }

  if (timeNow.minute == 0 && isUpdateEeprom)
  {
    isUpdateEeprom = false;

#ifdef DEBUG_EEPROM
    bot.sendMessage("Hour eeprom update", ADMIN_CHAT_ID);
#endif

    digitalWrite(LED_BUILTIN, LOW);

    Weather averangeHourWeather;
    if (numberOfMeasurements != 0)
    {
      averangeHourWeather.temperature = sumHourWeather.temperature / numberOfMeasurements;
      averangeHourWeather.sunPower = sumHourWeather.sunPower / numberOfMeasurements;
      averangeHourWeather.pressure = sumHourWeather.pressure / numberOfMeasurements;
      averangeHourWeather.humidity = sumHourWeather.humidity / numberOfMeasurements;
    }
    else
    {
      averangeHourWeather = weatherNow;
    }
    movingDataInMemory(ADDR_DAY_DATA, 24, averangeHourWeather);
    numberOfMeasurements = 0;

    if (timeNow.hour == HOUR_UPDATE_DATA && isUpdateMonthYearData)
    {
      isUpdateMonthYearData = false;

#ifdef DEBUG_EEPROM
      bot.sendMessage("Month data update", ADMIN_CHAT_ID);
#endif

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

#ifdef DEBUG_EEPROM
        bot.sendMessage("Year data update", ADMIN_CHAT_ID);
#endif

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
#ifdef DEBUG_EEPROM
      bot.sendMessage("Commit", ADMIN_CHAT_ID);
#endif
      if (timeNow.hour == HOUR_UPDATE_DATA)
      {
        isYearDataUpdate = true;
      }
      movingDataInMemory(ADDR_WEEK_DATA, 42, getAverangeWeather(ADDR_DAY_DATA + 16 * 20, 4));
      EEPROM.put(ADDR_MAX_INDEXES, weatherMax);
      EEPROM.put(ADDR_MAX_INDEXES, weatherMin);
      EEPROM.commit();
      delay(100);
    }

    digitalWrite(LED_BUILTIN, HIGH);
  }
  else if (timeNow.minute != 0)
  {
    isUpdateEeprom = true;
  }
}

LastTimeWeather getLastTimeData(LastTimePeriod type)
{
  std::vector<Weather> data;
  Weather temp;

  switch (type)
  {
  case LAST_DAY:
    for (uint16_t i = ADDR_DAY_DATA; i < ADDR_DAY_DATA + (24 / LAST_DAY) * 4; i += 4)
    {
      EEPROM.get(i, temp);
      data.push_back(temp);
    }
    break;
  case LAST_WEEK:
    for (uint16_t i = ADDR_WEEK_DATA; i < ADDR_WEEK_DATA + (168 / LAST_WEEK) * 4; i += 4)
    {
      EEPROM.get(i, temp);
      data.push_back(temp);
    }
    break;
  case LAST_MONTH:
    for (uint16_t i = ADDR_MONTH_DATA; i < ADDR_MONTH_DATA + (720 / LAST_MONTH) * 4; i += 4)
    {
      EEPROM.get(i, temp);
      data.push_back(temp);
    }
    break;
  case LAST_YEAR:
    for (uint16_t i = ADDR_YEAR_DATA; i < ADDR_YEAR_DATA + (8760 / LAST_YEAR) * 4; i += 4)
    {
      EEPROM.get(i, temp);
      data.push_back(temp);
    }
    break;
  default:
    break;
  }

  return LastTimeWeather(data, type, timeNow);
}

Weather getAverangeWeather(void)
{
  Weather result;
  EEPROM.get(ADDR_AVERANGE_INDEXES, result);
  return result;
}

void setWifiValues(WifiValues newWifiValues)
{
  EEPROM.put(ADDR_WIFI_VALUES, newWifiValues);
  EEPROM.commit();
}

WifiValues getWifiValues(void)
{
  EEPROM.get(ADDR_WIFI_VALUES, wifiValuesNow);
  return wifiValuesNow;
}

void setTimeZone(int16_t timeZone)
{
  EEPROM.put(ADDR_TIME_ZONE, timeZone);
  EEPROM.commit();
}

int16_t getTimeZone()
{
  int16_t timeZone;
  EEPROM.get(ADDR_TIME_ZONE, timeZone);
  return timeZone;
}