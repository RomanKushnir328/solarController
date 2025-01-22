#include "internetManagement.h"
#include "hardwareManagement.h"
#include "internetManagement.h"

ESP8266WebServer server(80);

extern wifiValues wifiValuesNow;
extern Weather weatherNow;
extern Voltages voltagesNow;
#ifdef LCD
extern LiquidCrystal_I2C lcd;
#endif
static bool isWiFi = false;
static bool isStartWiFi = true;
static bool alreadyAP = false;

static String SendHTML(void);

void WiFiConnection(void)
{
  managementWiFi();
  server.on("/", []()
            { server.send(200, "text/html", SendHTML()); });
  server.on("/outOn", []()
            {digitalWrite(OUT_SWITCH_PIN, LOW); server.send(200, "text/html", SendHTML()); });
  server.on("/outOff", []()
            {digitalWrite(OUT_SWITCH_PIN, HIGH); server.send(200, "text/html", SendHTML()); });
  server.on("/reset", []()
            { ESP.restart(); });
  server.onNotFound([]()
                    { server.send(404, "text/plain", "Not found"); });
  server.begin();
  #ifdef LCD
  lcd.backlight();
  if(WiFi.status() == WL_CONNECTED){
    lcd.print("Connected!");
    delay(1000);
    lcd.clear();
    lcd.home();
    lcd.print("Local IP ->");
    lcd.setCursor(0, 1);
    lcd.print(WiFi.localIP().toString());
  }else{
    lcd.print("AP created");
    delay(1000);
    lcd.clear();
    lcd.print("SSID ->");
    lcd.setCursor(0, 1);
    lcd.print(wifiValuesNow.apSsid);
    delay(2500);
    lcd.clear();
    lcd.print("Password ->");
    lcd.setCursor(0, 1);
    lcd.print(wifiValuesNow.apPassword);
  }
  delay(2500);
  lcd.clear();
  #endif
}

void managementWiFi(void)
{
  server.handleClient();
  if (isStartWiFi)
  {
    alreadyAP = false;
    isStartWiFi = false;
    WiFi.mode(WIFI_STA);
    WiFi.begin(wifiValuesNow.wifiSsid, wifiValuesNow.wifiPassword);
    uint16_t lastTime = millis();
    while (WiFi.status() != WL_CONNECTED)
    {
      if (millis() - lastTime > 10000){
        break;
      }
      delay(100);
      yield();
    }
  }

  yield();

  if (WiFi.status() != WL_CONNECTED)
  {
    isWiFi = false;
    int16_t networks = WiFi.scanNetworks();
    for (byte i = 0; i < networks; i++)
    {
      if (WiFi.SSID(i) == WIFI_SSID)
      {
        isWiFi = true;
      }
    }

    if (!isWiFi && !alreadyAP)
    {
      alreadyAP = true;
      WiFi.mode(WIFI_AP);
      WiFi.softAP(wifiValuesNow.apSsid, wifiValuesNow.apPassword);
    }
    else
    {
      isStartWiFi = true;
    }
  }
}

static String SendHTML(void)
{
  String ptr = "<!DOCTYPE html>\n";
  ptr += "<html>\n";
  ptr += "<head>\n";
  ptr += "<meta\n";
  ptr += "name=\"viewport\"\n";
  ptr += "content=\"width=device-width, initial-scale=1.0, user-scalable=no\"\n";
  ptr += "/>\n";
  ptr += "<title>SolarStationControl</title>\n";
  ptr += "<style>\n";
  ptr += "html {\n";
  ptr += "font-family: Helvetica;\n";
  ptr += "color: aliceblue;\n";
  ptr += "text-overflow: aliceblue;\n";
  ptr += "display: inline-block;\n";
  ptr += "margin: 0px auto;\n";
  ptr += "text-align: center;\n";
  ptr += "background-color: #010216;\n";
  ptr += "}\n";
  ptr += "h1 {\n";
  ptr += "font-size: xx-large;\n";
  ptr += "}\n";
  ptr += "h2 {\n";
  ptr += "font-size: x-large;\n";
  ptr += "}\n";
  ptr += "button {\n";
  ptr += "position: center;\n";
  ptr += "background-color: #5dffb3;\n";
  ptr += "font-size: x-large;\n";
  ptr += "border-radius: 10px;\n";
  ptr += "}\n";
  ptr += "</style>\n";
  ptr += "</head>\n";
  ptr += "<body>\n";
  ptr += "<h1>SolarStationControl</h1>\n";
  ptr += "<h2>Weather now</h2>\n";
  ptr += "<p>Temperature => " + String(weatherNow.temperature) + "</p>\n";
  ptr += "<p>Humidity => " + String(weatherNow.humidity) + "</p>\n";
  ptr += "<p>Preassure => " + String(weatherNow.pressure) + "</p>\n";
  ptr += "<h2>Voltages</h2>\n";
  ptr += "<p>Solarbattery voltage => " + String(voltagesNow.solBatVol) + "</p>\n";
  ptr += "<p>Solarbattery current => " + String(voltagesNow.sysBatCur) + "</p>\n";
  ptr += "<p>Battery voltage => " + String(voltagesNow.sysBatVol) + "</p>\n";
  ptr += "<p>Power input => " + String(weatherNow.sunPower) + "</p>\n";
  ptr += "<h2>Control</h2>\n";
  !digitalRead(OUT_SWITCH_PIN) ? ptr += "<button onclick=\"window.location.href='/outOff'\">outOff</button>\n" : ptr += "<button onclick=\"window.location.href='/outOn'\">outOn</button>\n";
  ptr += "<button onclick=\"window.location.href='/reset'\">reset</button>\n";
  ptr += "<h2>Settings</h2>\n";
  ptr += "<h3>WiFi connection</h3>\n";
  ptr += "<form>\n";
  ptr += "<label for=\"formSSID\">Input SSID</label>\n";
  ptr += "<input type=\"text\" id=\"formSSID\" placeholder=\"MyWiFi\">\n";
  ptr += "</form>\n";
  ptr += "<form>\n";
  ptr += "<label for=\"formPassword\">Input password</label>\n";
  ptr += "<input type=\"password\" id=\"formPassword\" placeholder=\"12345678\">\n";
  ptr += "</form>\n";
  ptr += "<h3>AP</h3>\n";
  ptr += "<form>\n";
  ptr += "<label for=\"formSSID_AP\">Input SSID</label>\n";
  ptr += "<input type=\"button\" id=\"formSSID_AP\" placeholder=\"MyAP\">\n";
  ptr += "</form>\n";
  ptr += "</body>\n";
  ptr += "</html>\n";
  return ptr;
}