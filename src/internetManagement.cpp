#include "internetManagement.h"
#include "hardwareManagement.h"
#include "eepromManagement.h"

ESP8266WebServer server(80);

extern FastBot bot;
extern WifiValues wifiValuesNow;
extern Weather weatherNow;
extern Voltages voltagesNow;
extern bool isСonnectedADS;
extern bool isСonnectedSensors;
#ifdef LCD
#endif
static bool isWiFi = false;
static bool isStartWiFi = true;
bool alreadyAP = false;

LastTimePeriod graphPeriod = LAST_DAY;
byte graphType = 0;

static void handlePost(void);
static void handleGet(void);

void WiFiConnection(void)
{
  managementWiFi();
  server.on("/", []()
            { server.send(200, "text/html", sendHTML()); });
  server.on("/outOn", []()
            {digitalWrite(OUT_SWITCH_PIN, LOW); server.send(200, "text/html", sendHTML()); });
  server.on("/outOff", []()
            {digitalWrite(OUT_SWITCH_PIN, HIGH); server.send(200, "text/html", sendHTML()); });
  server.on("/reset", []()
            { ESP.restart(); });
  server.on("/post", HTTP_POST, handlePost);
  server.on("/graph", handleGet);
  server.onNotFound([]()
                    { server.send(404, "text/plain", "Not found"); });
  server.begin();
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
      if (millis() - lastTime > 10000)
      {
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
      if (WiFi.SSID(i) == wifiValuesNow.wifiSsid)
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

String sendHTML(void)
{
  String ptr = "<!DOCTYPE html>\n";
  ptr += "<html>\n";
  ptr += "<head>\n";
  ptr += "<meta name = \"viewport\" width = device-width, initial-scale = 1.0, user-scalable = yes/>\n";
  ptr += "<title> SolarStationControl</title>\n";
  ptr += "<style> html\n";
  ptr += "{\n";
  ptr += "text-align: center;\n";
  ptr += "font-family: Helvetica;\n";
  ptr += "color:\n";
  ptr += "aliceblue;\n";
  ptr += "background-color: #010216;\n";
  ptr += "}\n";
  ptr += "h1\n";
  ptr += "{\n";
  ptr += "font-size: xx-large;\n";
  ptr += "}\n";
  ptr += "h2\n";
  ptr += "{\n";
  ptr += "font-size: x-large;\n";
  ptr += "}\n";
  ptr += "button\n";
  ptr += "{\n";
  ptr += "position:\n";
  ptr += "center;\n";
  ptr += "background-color: #04859D;\n";
  ptr += "font-size: x-large;\n";
  ptr += "border-radius: 20px;\n";
  ptr += "}\n";
  ptr += ".toggleButton\n";
  ptr += "{\n";
  ptr += "background-color: #FF7C00;\n";
  ptr += "font-size: medium;\n";
  ptr += "}\n";
  ptr += ".formButton\n";
  ptr += "{\n";
  ptr += "background-color: #186F05;\n";
  ptr += "font-size: medium;\n";
  ptr += "border-radius: 15px;\n";
  ptr += "}\n";
  ptr += ".container\n";
  ptr += "{\n";
  ptr += "display:\n";
  ptr += "grid;\n";
  ptr += "grid-template-columns: 1fr 1fr;\n";
  ptr += "gap:\n";
  ptr += "20px;\n";
  ptr += "}\n";
  ptr += ".box\n";
  ptr += "{\n";
  ptr += "width:\n";
  ptr += "95 % ;\n";
  ptr += "padding:\n";
  ptr += "10px;\n";
  ptr += "text-align: center;\n";
  ptr += "}\n";
  ptr += "@media(max-width: 800px)\n";
  ptr += "{\n";
  ptr += ".container\n";
  ptr += "{\n";
  ptr += "grid-template-columns: 1fr;\n";
  ptr += "}\n";
  ptr += "}\n";
  ptr += "#graph {\n";
  ptr += "background-color: beige;\n";
  ptr += "width:\n";
  ptr += "100 % ;\n";
  ptr += "max-height: 600px;\n";
  ptr += "}\n";
  ptr += "</style>\n";
  ptr += "</head>\n";
  ptr += "<body>\n ";
  ptr += "<h1>SolarStationControl</h1>\n";
  ptr += "<div class=\"container\">\n";
  ptr += "<div class=\"box\">\n";
#ifndef DEBUG_SENSORS
  ptr += "<h2>Weather now</h2>\n";
  if (isСonnectedSensors)
  {
    ptr += "<p>Temperature => " + String(weatherNow.temperature) + "</p>\n";
    ptr += "<p>Humidity => " + String(weatherNow.humidity) + "</p>\n";
    ptr += "<p>Preassure => " + String(weatherNow.pressure) + "</p>\n";
  }
  else
  {
    ptr += "<p>Sensors aren`t connected</p>\n";
  }
#endif
#ifndef DEBUG_ADS
  ptr += "<h2>Voltages</h2>\n";
  if (isСonnectedADS)
  {
    ptr += "<p>Solarbattery voltage => " + String(voltagesNow.solBatVol) + "</p>\n";
    ptr += "<p>Solarbattery current => " + String(voltagesNow.sysBatCur) + "</p>\n";
    ptr += "<p>Battery voltage => " + String(voltagesNow.sysBatVol) + "</p>\n";
    ptr += "<p>Power input => " + String(weatherNow.sunPower) + "</p>\n";
  }
  else
  {
    ptr += "<p>ADS aren`t connected</p>\n";
  }
#endif
  ptr += "<h2>Control</h2>\n";
  !digitalRead(OUT_SWITCH_PIN) ? ptr += "<button onclick=\"window.location.href='/outOff'\">outOff</button>\n" : ptr += "<button onclick=\"window.location.href='/outOn'\">outOn</button>\n";
  ptr += "<button onclick=\"window.location.href = \'/reset\'\">reset</button>\n";
  ptr += "<h2>Settings</h2>\n";
  ptr += "<form method=\"POST\" action=\"/post\">\n";
  ptr += "<h3>WiFi connection</h3>\n";
  ptr += "<p>Wi-Fi ssid <input type=\"text\" name=\"wifiSsid\" value=\"" + String(wifiValuesNow.wifiSsid) + "\"></p>\n";
  ptr += "<p>Wi-Fi pass:\n";
  ptr += "<input type=\"password\" id=\"wifiPass\" name=\"wifiPass\" value=\"" + String(wifiValuesNow.wifiPassword) + "\" required>\n";
  ptr += "<button class=\"toggleButton\" type=\"button\" id=\"toggleWifiPass\"\n";
  ptr += "onclick=\"toggleWifiPassword()\">Show</button>\n";
  ptr += "</p>\n";
  ptr += "<h3>AP</h3>\n";
  ptr += "<p>AP ssid <input type=\"text\" name=\"apSsid\" value=\"" + String(wifiValuesNow.apSsid) + "\"></p>\n";
  ptr += "<p>AP pass:\n";
  ptr += "<input type=\"password\" id=\"apPass\" name=\"apPass\" value=\"" + String(wifiValuesNow.apPassword) + "\" required>\n";
  ptr += "<button class=\"toggleButton\" type=\"button\" id=\"toggleApPass\"\n";
  ptr += "onclick=\"toggleApPassword()\">Show</button>\n";
  ptr += "</p>\n";
  ptr += "<input class=\"formButton\" type=\"submit\" value=\"send and reconect\">\n";
  ptr += "</form>\n";
  ptr += "</div>\n";
  ptr += "<div class=\"box\" id=\"graphContainer\">\n";
  ptr += "<h2>Weather graph</h2>\n";
  ptr += "<form action=\"/graph\" method=\"GET\">\n";
  ptr += "<label for=\"choiceGraphPeriod\">Chose graph period: </label>\n";
  ptr += "<select id=\"choiceGraphPeriod\" name=\"choiceGraphPeriod\" onchange=\"this.form.submit()\">\n";
  if (graphPeriod == LAST_DAY)
  {
    ptr += "<option value=\"lastDay\" selected>lastDay</option>\n";
  }
  else
  {
    ptr += "<option value=\"lastDay\">lastDay</option>\n";
  }

  if (graphPeriod == LAST_WEEK)
  {
    ptr += "<option value=\"lastWeek\" selected>lastWeek</option>\n";
  }
  else
  {
    ptr += "<option value=\"lastWeek\">lastWeek</option>\n";
  }

  if (graphPeriod == LAST_MONTH)
  {
    ptr += "<option value=\"lastMonth\" selected>lastMonth</option>\n";
  }
  else
  {
    ptr += "<option value=\"lastMonth\">lastMonth</option>\n";
  }

  if (graphPeriod == LAST_YEAR)
  {
    ptr += "<option value=\"lastYear\" selected>lastYear</option>\n";
  }
  else
  {
    ptr += "<option value=\"lastYear\">lastYear</option>\n";
  }
  ptr += "</select>\n";
  ptr += "</form>\n";
  ptr += "<form action=\"/graph\" method=\"GET\">\n";
  ptr += "<label for=\"choiceGraphType\">Chose graph type: </label>\n";
  ptr += "<select id=\"choiceGraphType\" name=\"choiceGraphType\" onchange=\"this.form.submit()\">\n";
  if (graphType == 0)
  {
    ptr += "<option value=\"solPower\" selected >Solar power</option>\n";
  }
  else
  {
    ptr += "<option value=\"solPower\">Solar power</option>\n";
  }

  if (graphType == 1)
  {
    ptr += "<option value=\"temperature\" selected >Temperature</option>\n";
  }
  else
  {
    ptr += "<option value=\"temperature\">Temperature</option>\n";
  }

  if (graphType == 2)
  {
    ptr += "<option value=\"pressure\" selected >Preassure</option>\n";
  }
  else
  {
    ptr += "<option value=\"pressure\">Preassure</option>\n";
  }

  if (graphType == 3)
  {
    ptr += "<option value=\"humidity\" selected >Humidity</option>\n";
  }
  else
  {
    ptr += "<option value=\"humidity\">Humidity</option>\n";
  }
  ptr += "</select>\n";
  ptr += "</form>\n";
  ptr += "<canvas id=\"graph\"></canvas>\n";
  ptr += "<script>\n";
  ptr += "function toggleWifiPassword() {\n";
  ptr += "var wifiPassInput = document.getElementById(\"wifiPass\");\n";
  ptr += "var wifiToggleBtn = document.getElementById(\"toggleWifiPass\");\n";
  ptr += "if (wifiPassInput.type === \"password\") {\n";
  ptr += "wifiPassInput.type = \"text\";\n";
  ptr += "wifiToggleBtn.textContent = \"Hide\";\n";
  ptr += "} else {\n";
  ptr += "wifiPassInput.type = \"password\";\n";
  ptr += "wifiToggleBtn.textContent = \"Show\";\n";
  ptr += "}\n";
  ptr += "}\n";
  ptr += "function toggleApPassword() {\n";
  ptr += "var apPassInput = document.getElementById(\"apPass\");\n";
  ptr += "var apToggleBtn = document.getElementById(\"toggleApPass\");\n";
  ptr += "if (apPassInput.type === \"password\") {\n";
  ptr += "apPassInput.type = \"text\";\n";
  ptr += "apToggleBtn.textContent = \"Hide\";\n";
  ptr += "} else {\n";
  ptr += "apPassInput.type = \"password\";\n";
  ptr += "apToggleBtn.textContent = \"Show\";\n";
  ptr += "}\n";
  ptr += "}\n";
  ptr += "function printGraph(width, height) {\n";
  ptr += "const data = [\n";

  ptr += "];\n";
  ptr += "const retreatForLabels = 40;\n";
  ptr += "const horizontalColomsNum = 10;\n";
  ptr += "const colomsNumToChangeLabel = 4;\n";
  ptr += "let dataMax = Math.max(...data);\n";
  ptr += "let dataMin = Math.min(...data);\n";
  ptr += "let sum = data.reduce((a, b) => a + b, 0);\n";
  ptr += "let dataAverange = sum/data.length;\n";
  ptr += "let difference = dataMax-dataMin;\n";
  ptr += "const canvas = document.getElementById(\"graph\");\n";
  ptr += "const ctx = canvas.getContext(\"2d\");\n";
  ptr += "canvas.width = width;\n";
  ptr += "canvas.height = height;\n";
  ptr += "width -= retreatForLabels;\n";
  ptr += "height -= retreatForLabels;\n";
  ptr += "const stepX = width/data.length;\n";
  ptr += "const stepY = height/difference;\n";
  ptr += "ctx.clearRect(0, 0, canvas.width, canvas.height);\n";
  ptr += "ctx.beginPath();\n";
  ptr += "ctx.font = \"10px roboto\";\n";
  ptr += "for (let i = 0; i < data.length; i++) {\n";
  ptr += "pointX = i * stepX + 40;\n";
  ptr += "pointY = height-height * ((data[i]-dataMin)/difference);\n";
  ptr += "ctx.strokeStyle = \"blue\";\n";
  ptr += "ctx.lineWidth = 3;\n";
  ptr += "ctx.lineTo(pointX, pointY);\n";
  ptr += "ctx.stroke();\n";
  ptr += "ctx.strokeStyle = \"black\";\n";
  ptr += "ctx.lineWidth = 1;\n";
  ptr += "ctx.moveTo(pointX, 0);\n";
  ptr += "if (i % colomsNumToChangeLabel === 0) {\n";
  ptr += "ctx.lineTo(pointX, canvas.height);\n";
  ptr += "ctx.fillStyle = \"black\";\n";
  ptr += "ctx.fillText(labels[i], pointX + 5, canvas.height-5);\n";
  ptr += "} else {\n";
  ptr += "ctx.lineTo(pointX, canvas.height-retreatForLabels);\n";
  ptr += "}\n";
  ptr += "ctx.stroke();\n";
  ptr += "ctx.beginPath();\n";
  ptr += "ctx.moveTo(pointX, pointY);\n";
  ptr += "}\n";
  ptr += "for (let j = 0; j < horizontalColomsNum + 1; j++) {\n";
  ptr += "labelY = j * (height/horizontalColomsNum);\n";
  ptr += "ctx.moveTo(0, labelY);\n";
  ptr += "ctx.fillText((dataMax-(difference/horizontalColomsNum) * j).toFixed(1), 5, labelY-5);\n";
  ptr += "ctx.lineTo(canvas.width, labelY);\n";
  ptr += "}\n";
  ptr += "ctx.stroke();\n";
  ptr += "}\n";
  ptr += "function updateGraphSize() {\n";
  ptr += "const graphContainer = document.getElementById(\"graphContainer\");\n";
  ptr += "const rect = graphContainer.getBoundingClientRect();\n";
  ptr += "printGraph(rect.width, rect.height);\n";
  ptr += "}\n";
  ptr += "window.onload = updateGraphSize;\n";
  ptr += "window.addEventListener(\"resize\", updateGraphSize);\n";
  ptr += "</script>\n";
  ptr += "</div>\n";
  ptr += "</div>\n";
  ptr += "</body>\n";
  ptr += "</html>\n";
  return ptr;
}

static void handlePost(void)
{
  digitalWrite(LED_BUILTIN, LOW);
  if (server.hasArg("plain"))
  {
    bool updated = false;
    if (server.hasArg("wifiSsid"))
    {
      strncpy(wifiValuesNow.wifiSsid, server.arg("wifiSsid").c_str(), sizeof(wifiValuesNow.wifiSsid) - 1);
      wifiValuesNow.wifiSsid[sizeof(wifiValuesNow.wifiSsid) - 1] = '\0';
      updated = true;
    }
    if (server.hasArg("wifiPass"))
    {
      strncpy(wifiValuesNow.wifiPassword, server.arg("wifiPass").c_str(), sizeof(wifiValuesNow.wifiPassword) - 1);
      wifiValuesNow.wifiPassword[sizeof(wifiValuesNow.wifiPassword) - 1] = '\0';
      updated = true;
    }
    if (server.hasArg("apSsid"))
    {
      strncpy(wifiValuesNow.apSsid, server.arg("apSsid").c_str(), sizeof(wifiValuesNow.apSsid) - 1);
      wifiValuesNow.wifiSsid[sizeof(wifiValuesNow.wifiSsid) - 1] = '\0';
      updated = true;
    }
    if (server.hasArg("apPass"))
    {
      strncpy(wifiValuesNow.apPassword, server.arg("apPass").c_str(), sizeof(wifiValuesNow.apPassword) - 1);
      wifiValuesNow.wifiPassword[sizeof(wifiValuesNow.wifiPassword) - 1] = '\0';
      updated = true;
    }

    if (updated)
    {
      setWifiValues(wifiValuesNow);
      server.send(200, "html", sendHTML());
    }
    else
    {
      server.send(400, "application/json", "{\"status\":\"error\",\"message\":\"No valid fields provided\"}");
    }
  }
  else
  {
    digitalWrite(LED_BUILTIN, HIGH);
    server.send(400, "application/json", "{\"status\":\"error\",\"message\":\"No data received\"}");
  }

  digitalWrite(LED_BUILTIN, HIGH);
}

static void handleGet(void)
{
  if (server.hasArg("choiceGraphPeriod"))
  {
    String graphPeriodParam = server.arg("choiceGraphPeriod");
    if (graphPeriodParam == "lastDay")
    {
      graphPeriod = LAST_DAY;
    }
    else if (graphPeriodParam == "lastWeek")
    {
      graphPeriod = LAST_WEEK;
    }
    else if (graphPeriodParam == "lastMonth")
    {
      graphPeriod = LAST_MONTH;
    }
    else if (graphPeriodParam == "lastYear")
    {
      graphPeriod = LAST_YEAR;
    }
  }
  if (server.hasArg("choiceGraphType"))
  {
    String graphTypeParam = server.arg("choiceGraphType");
    if (graphTypeParam == "solPower")
    {
      graphType = 0;
    }
    else if (graphTypeParam == "temperature")
    {
      graphType = 1;
    }
    else if (graphTypeParam == "pressure")
    {
      graphType = 2;
    }
    else if (graphTypeParam == "humidity")
    {
      graphType = 3;
    }
  }
  server.send(200, "text/html", sendHTML());
}