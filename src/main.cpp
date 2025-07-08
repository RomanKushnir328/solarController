#include "dataTypes.h"
#include "hardwareManagement.h"
#include "internetManagement.h"
#include "eepromManagement.h"
#include "botManagement.h"

extern FastBot bot;

void setup(void)
{
  pinsConnection();
  digitalWrite(LED_WORK_PIN, LOW);
  sensorsConnection();
  startEeprom();
  WiFiConnection();
  botConnection();
  digitalWrite(LED_BUILTIN, LOW);
  delay(100);
  digitalWrite(LED_BUILTIN, HIGH);
  digitalWrite(LED_WORK_PIN, HIGH);
}

void loop(void)
{
  managementWiFi();
  botManage();
  eepromManage();
  digitalWrite(LED_WORK_PIN, HIGH);
  static Timer weatherTimer = Timer(0, 0);
  if (weatherTimer.everyMilli(1000))
  {
    readWeather();
    batControl();
  }
  maxPointPowerTracking();
#ifdef LCD
  static Timer displayTimer = Timer(0, 0);
  if (displayTimer.everyMilli(100))
  {
    controlDisplay();
  }
#endif
  digitalWrite(LED_WORK_PIN, LOW);
}
