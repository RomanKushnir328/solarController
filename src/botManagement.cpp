#include "botManagement.h"
#include "eepromManagement.h"
#include "hardwareManagement.h"
#include "internetManagement.h"

extern Voltages voltagesNow;
extern Weather weatherNow;
extern bool presetSaveMode;
extern bool isСonnectedADS;
extern bool isСonnectedSensors;
extern Weather weatherMax;
extern Weather weatherMin;

FastBot bot(BOT_TOKEN);
FB_Time timeNow;
static uint64_t timeLastMsg;

bool maxPWM = false;
bool minPWM = false;
static uint32_t timeSetPWMToMax;
static uint32_t timeSetPWMToMin;

static void newMsg(FB_msg &msg);
static bool mainBotFunctions(const String userMsg, const String userID);

void botConnection(void)
{
  bot.attach(newMsg);
  bot.sendMessage("Bot started, local IP => " + WiFi.localIP().toString() + ", connection strenght => " + String(WiFi.RSSI()), ADMIN_CHAT_ID);
  bot.skipUpdates();

  if (!isСonnectedSensors)
  {
    bot.sendMessage("Can`t connect sensors", ADMIN_CHAT_ID);
  }

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
  timeNow = bot.getTime(getTimeZone());
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

static String getMenuControl(void)
{
  String result = String(menuControl);

  if (!digitalRead(OUT_SWITCH_PIN))
  {
    result += "\n /outOff";
  }
  else
  {
    result += "\n /outOn";
  }

  if (presetSaveMode)
  {
    result += "\t /saveModeOff";
  }
  else
  {
    result += "\t /saveModeOn";
  }
  return result;
}

static void newMsg(FB_msg &msg)
{
  static bool isUpdateTimeZone = false;

  digitalWrite(LED_BUILTIN, LOW);

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
        if (msg.text == "/menuControl")
        {
          bot.showMenu(getMenuControl(), msg.userID);
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
          bot.showMenu(getMenuControl(), msg.userID);
        }
        else if (msg.text == "/outOff")
        {
          digitalWrite(OUT_SWITCH_PIN, HIGH);
          botMsg += "output is enabled";
          bot.showMenu(getMenuControl(), msg.userID);
        }
        else if (msg.text == "/saveModeOff")
        {
          presetSaveMode = false;
          botMsg += "Save mode is turned off";
          bot.showMenu(getMenuControl(), msg.userID);
        }
        else if (msg.text == "/saveModeOn")
        {
          presetSaveMode = true;
          botMsg += "Save mode is turned on";
          bot.showMenu(getMenuControl(), msg.userID);
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
        if (msg.OTA)
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
    int16_t timeZone;
    msg.text.trim();
    timeZone = msg.text.toInt();
    if (timeZone > 720 || timeZone < -720)
    {
      botMsg += "Send your time zone in minutes!. From -720 to 720";
    }
    else
    {
      setTimeZone(timeZone);
      botMsg += "Time zone set to " + String(timeZone) + " minutes plus UTC";
      isUpdateTimeZone = false;
    }
  }
  bot.sendMessage(botMsg, msg.userID);
  timeLastMsg = millis();
  digitalWrite(LED_BUILTIN, HIGH);
}

static bool mainBotFunctions(const String userMsg, const String userID)
{
  if (userMsg == "/getWeather")
  {
      bot.sendMessage(weatherNow.toString(), userID);
  }
  else if (userMsg == "/mainMenu" || userMsg == "/start")
  {
    if (userID == ADMIN_CHAT_ID)
    {
      bot.showMenu(mainMenuForAdmin, userID);
    }
    else
    {
      bot.showMenu(mainMenu, userID);
    }
  }
  else if (userMsg == "/getWeatherLastDay")
  {
    bot.sendMessage("Date solPower temp pressure humidity\n" + getLastTimeData(LAST_DAY).toString(), userID);
  }
  else if (userMsg == "/getWeatherLastWeek")
  {
    bot.sendMessage("Date solPower temp pressure humidity\n" + getLastTimeData(LAST_WEEK).toString(), userID);
  }
  else if (userMsg == "/getWeatherLastMonth")
  {
    bot.sendMessage("Date solPower temp pressure humidity\n" + getLastTimeData(LAST_MONTH).toString(), userID);
  }
  else if (userMsg == "/getWeatherLastYear")
  {
    bot.sendMessage("Date solPower temp pressure humidity\n" + getLastTimeData(LAST_YEAR).toString(), userID);
  }
  else if (userMsg == "/getAverangeWeather")
  {
    bot.sendMessage(getAverangeWeather().toString(), userID);
  }
  else if (userMsg == "/getMaximumIndexes")
  {
    bot.sendMessage("Max indexes:\n" + weatherMax.toString(), userID);
    bot.sendMessage("Min indexes:\n" + weatherMin.toString(), userID);
  }
  else
  {
    return false;
  }

  return true;
}