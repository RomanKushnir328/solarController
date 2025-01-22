#pragma once
#include <Arduino.h>
#include <limits.h>
#include "publicPreferences.h"

struct Weather
{
  float sunPower;
  float temperature;
  float humidity;
  float pressure;
  String toString(void)
  {
    return "Sun power => " + String(sunPower) + "\n" +
           "Temperature => " + String(temperature) + "\n" +
           "Humidity => " + String(humidity) + "\n" +
           "Pressure => " + String(pressure);
  }
};

struct Voltages
{
  float solBatVol;
  float sysBatVol;
  float sysBatCur;
  float maxPowerPointVol = 18.0;
  float sunPower;
  int16_t PWM;
  String toString(void)
  {
    return "Solar battery voltage => " + String(solBatVol) + "\n" +
           "System battery voltage => " + String(sysBatVol) + "\n" +
           "System battery current => " + String(sysBatCur) + "\n" +
           "Max power point voltage => " + String(maxPowerPointVol) + "\n" +
           "Sun power => " + String(sunPower) + "\n" +
           "PWM => " + String(PWM);
  }
};

struct wifiValues{
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