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
  readWeather();
  maxPointPowerTracking();
  batControl();
  digitalWrite(LED_WORK_PIN, LOW);
}
