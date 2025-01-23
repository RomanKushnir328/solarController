#include "botManagement.h"
#include "eepromManagement.h"
#include "hardwareManagement.h"

extern Voltages voltagesNow;
extern Weather weatherNow;
extern bool presetSaveMode;
extern bool isСonnectedADS;
extern bool isСonnectedSensors;

FastBot bot(BOT_TOKEN);
FB_Time timeNow;
static uint64_t timeLastMsg;
int16_t timeZone = UTC_ZONE;

bool maxPWM = false;
bool minPWM = false;
static uint32_t timeSetPWMToMax;
static uint32_t timeSetPWMToMin;

static void newMsg(FB_msg &msg);
static bool mainBotFunctions(const String userMsg, const String userID);

void botConnection(void)
{
  timeNow = bot.getTime(timeZone);
  bot.attach(newMsg);
  bot.sendMessage("Bot started, local IP => " + WiFi.localIP().toString() + ", connection strenght => " + String(WiFi.RSSI()), ADMIN_CHAT_ID);
  bot.skipUpdates();

#ifndef DEBUG_WEATHER_SENSORS
  if (!isСonnectedSensors)
  {
    bot.sendMessage("Can`t connect sensors", ADMIN_CHAT_ID);
  }
#endif

#ifndef DEBUG_ADS
  if (!isСonnectedADS)
  {
    bot.sendMessage("Can`t connect ADS", ADMIN_CHAT_ID);
  }
#endif
}

void botManage(void)
{
  bot.tick();
  if (millis() - timeLastMsg < 10000)
  {
    bot.setPeriod(1000);
  }
  else
  {
    bot.setPeriod(3500);
  }
  if (millis() - timeSetPWMToMax > 10000)
  {
    maxPWM = false;
  }
  if (millis() - timeSetPWMToMin > 10000)
  {
    minPWM = false;
  }
}

static void newMsg(FB_msg &msg)
{
  static bool isUpdateTimeZone = false;

  digitalWrite(LED_BUILTIN, LOW);

  timeLastMsg = millis();

  String botMsg;

  if (msg.query)
  {
    bot.answer("Data erasure... Wait until the microcontroller will restart", FB_NOTIF);
    EEPROM.put(ADDR_IS_EEPROM_CLEANING, 255);
    EEPROM.commit();
    delay(100);
    ESP.restart();
  }

  if (!isUpdateTimeZone)
  {
    if (!mainBotFunctions(msg.text, msg.userID))
    {
      if (msg.userID == ADMIN_CHAT_ID)
      {
        if (msg.text == "/menuLastTimeWeather")
        {
          bot.showMenu(menuLastTimeWeatherForAdmin, msg.userID);
        }
        else if (msg.text == "/menuControlHardware")
        {
          bot.showMenu(menuControlHardware, msg.userID);
        }
        else if (msg.text == "/menuControlWiFi")
        {
          bot.showMenu(menuControlWiFi, msg.userID);
        }
        else if (msg.text == "/restart")
        {
          ESP.restart();
        }
        else if (msg.text == "/resetMemory")
        {
          bot.inlineMenu("This action is irreversible, are you sure ?.", "Yes", ADMIN_CHAT_ID);
        }
        else if (msg.text == "/getVoltages")
        {
          botMsg += voltagesNow.toString();
        }
        else if (msg.text == "/outOn")
        {
          digitalWrite(OUT_SWITCH_PIN, LOW);
          botMsg += "output is disabled";
        }
        else if (msg.text == "/outOff")
        {
          digitalWrite(OUT_SWITCH_PIN, HIGH);
          botMsg += "output is enabled";
        }
        else if (msg.text == "/saveModeOff")
        {
          presetSaveMode = false;
          botMsg += "Save mode is turned off";
        }
        else if (msg.text == "/saveModeOn")
        {
          presetSaveMode = true;
          botMsg += "Save mode is turned on";
        }
        else if (msg.text == "/maxPWM")
        {
          if (!minPWM)
          {
            timeSetPWMToMax = millis();
            maxPWM = true;
            botMsg += "PWM set to maximum";
          }
          else
          {
            botMsg += "PWM is already set to minimum";
          }
        }
        else if (msg.text == "/minPWM")
        {
          if (!maxPWM)
          {
            timeSetPWMToMin = millis();
            minPWM = true;
            botMsg += "PWM set to minimum";
          }
          else
          {
            botMsg += "PWM is already set to maximum";
          }
        }
        else if (msg.text == "/getLocalIP")
        {
          botMsg += WiFi.localIP().toString() + ", connection quality =>" + WiFi.RSSI();
        }
        else if (msg.text == "/setWiFi")
        {
          botMsg += "unfinished";
        }
        else if (msg.text == "/setAP")
        {
          botMsg += "unfinished";
        }
        else if (msg.text == "/setTimeZone")
        {
          isUpdateTimeZone = true;
          botMsg += "Write your current time zone UTC plus or minus minutes";
        }
        else if (msg.OTA)
        {
          bot.update();
        }
        else
        {
          botMsg += "unknown command";
        }
      }
      else
      {
        if (msg.text == "/menuLastTimeWeather")
        {
          botMsg += "menuLastTimeWeather";
        }
        else if (msg.OTA)
        {
          botMsg += "You do not have access to this feature";
        }
        else
        {
          botMsg += "You do not have access to this feature or unknown command";
        }
        bot.sendMessage(msg.username + " " + msg.chatID, ADMIN_CHAT_ID);
      }
    }
  }
  else
  {
    timeZone = msg.data.toInt();
    if (timeZone > 720 || timeZone < -720)
    {
      botMsg += "Send your time zone in minutes!. From -720 to 720";
    }
    else
    {
      botMsg += "Time zone set to " + String(timeZone) + "minutes plus UTC";
      isUpdateTimeZone = false;
    }
  }
  bot.sendMessage(botMsg, msg.userID);
  digitalWrite(LED_BUILTIN, HIGH);
}

static bool mainBotFunctions(const String userMsg, const String userID)
{
  bool isResult = true;
  String botMsg = "";
  if (userMsg == "/getWeather")
  {
    botMsg += "Sun power => " + String(weatherNow.sunPower) + ", temperature => " + String(weatherNow.temperature) + ", humidity => " + String(weatherNow.humidity) + ", pressure => " + String(weatherNow.pressure);
  }
  else if (userMsg == "/getSunPower")
  {
    botMsg += "Sun power => " + String(weatherNow.sunPower);
  }
  else if (userMsg == "/getTemperature")
  {
    botMsg += "Temperature => " + String(weatherNow.temperature);
  }
  else if (userMsg == "/getHumidity")
  {
    botMsg += "Humidity => " + String(weatherNow.humidity);
  }
  else if (userMsg == "/getPressure")
  {
    botMsg += "Pressure => " + String(weatherNow.pressure);
  }
  else if (userMsg == "/menuWeatherNow" || userMsg == "/start")
  {
    bot.showMenu(menuWeatherNow, userID);
  }
  else if (userMsg == "/getWeatherLastDay")
  {
    botMsg += getDayData();
  }
  else if (userMsg == "/getWeatherLastWeek")
  {
    botMsg += getWeekData();
  }
  else if (userMsg == "/getWeatherLastMonth")
  {
    botMsg += getMonthData();
  }
  else if (userMsg == "/getWeatherLastYear")
  {
    botMsg += getYearData();
  }
  else if (userMsg == "/getAverangeWeather")
  {
    botMsg += getAverangeWeather();
  }
  else if (userMsg == "/getMaximumIndexes")
  {
    botMsg += getMaxIndexes();
  }
  else
  {
    isResult = false;
  }

  if (botMsg.length() > 0)
  {
    bot.sendMessage(botMsg, userID);
  }

  return isResult;
}