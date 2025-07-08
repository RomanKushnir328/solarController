#include "hardwareManagement.h"
#include "botManagement.h"
#include "internetManagement.h"
#include "eepromManagement.h"

extern FastBot bot;

extern bool alreadyAP;
extern WifiValues wifiValuesNow;

#ifdef LCD
static LiquidCrystal_I2C lcd(0x27, 20, 4);
#endif

static Adafruit_ADS1115 ads;

static Adafruit_BMP280 bmp280;
static AHT10 aht20(AHT10_ADDRESS_0X38, AHT20_SENSOR);

extern FB_Time timeNow;
Voltages voltagesNow;
Weather weatherNow;
bool presetSaveMode = false;
bool isСonnectedADS = false;
bool isСonnectedSensors = false;

static const uint32_t SLEEP_CHUNK = 268000; // 268 sec.
static uint32_t remains;
static bool isBatDisconected = false;
static bool saveMode = false;
extern bool maxPWM;
extern bool minPWM;

static void wakeup(void)
{
  if (remains <= SLEEP_CHUNK)
  { // Last iteration
    wifi_fpm_close();
  }
}

static void sleep(uint32_t ms)
{
  uint8_t optmode;

  wifi_station_disconnect();
  optmode = wifi_get_opmode();
  if (optmode != NULL_MODE)
    wifi_set_opmode_current(NULL_MODE);
  wifi_fpm_set_sleep_type(LIGHT_SLEEP_T);
  wifi_fpm_open();
  wifi_fpm_set_wakeup_cb(wakeup);
  remains = ms;
  while (remains > 0)
  {
    if (remains > SLEEP_CHUNK)
      ms = SLEEP_CHUNK;
    else
      ms = remains;
    wifi_fpm_do_sleep(ms * 1000);
    delay(ms);
    remains -= ms;
  }
  if (optmode != NULL_MODE)
    wifi_set_opmode_current(optmode);
}
// sleep func author https://github.com/MoonFox2006/LightSleep

static void readVoltages(void)
{
  if (isСonnectedADS)
  {
    voltagesNow.solBatVol = ads.computeVolts(ads.readADC_SingleEnded(SOL_BAT_VOL_PIN)) * DIVIDER_SOL_BAT;
    voltagesNow.sysBatVol = ads.computeVolts(ads.readADC_SingleEnded(SYS_BAT_VOL_PIN)) * DIVIDER_SYS_BAT;
    voltagesNow.sysBatCur = voltagesNow.solBatVol > voltagesNow.sysBatVol ? -ads.computeVolts(ads.readADC_SingleEnded(SOL_BAT_CURRENT_PIN)) / SOL_BAT_SHUNT_RESISTANCE : 0.0;
    voltagesNow.sunPower = voltagesNow.solBatVol > voltagesNow.sysBatVol ? voltagesNow.sysBatVol * voltagesNow.sysBatCur : 0.0;
  }
}

void pinsConnection(void)
{
  pinMode(PWM_PIN, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(OUT_SWITCH_PIN, OUTPUT);
  pinMode(LED_WORK_PIN, OUTPUT);
#ifdef LCD
  pinMode(CH_MENU_BUT, INPUT_PULLUP);
#endif
  digitalWrite(OUT_SWITCH_PIN, LOW);
  digitalWrite(LED_BUILTIN, HIGH);
  digitalWrite(LED_WORK_PIN, LOW);

  analogWriteResolution(PWM_RESOLUTION);

  Wire.begin(SDA, SCL);
}

void sensorsConnection(void)
{
#ifdef LCD
  lcd.init();
  lcd.noBacklight();
  lcd.autoscroll();
#endif

  isСonnectedSensors = aht20.begin();
  isСonnectedSensors = bmp280.begin();

  bmp280.setSampling(Adafruit_BMP280::MODE_NORMAL,
                     Adafruit_BMP280::SAMPLING_X2,
                     Adafruit_BMP280::SAMPLING_X16,
                     Adafruit_BMP280::FILTER_X16,
                     Adafruit_BMP280::STANDBY_MS_500);

  ads.setGain(GAIN_TWO);
  isСonnectedADS = ads.begin();
  if (isСonnectedADS)
  {
    analogWrite(PWM_PIN, MAX_PWM);
    delay(100);
    readVoltages();
    if (voltagesNow.sunPower)
    {
      voltagesNow.PWM = (MAX_SOL_POW / voltagesNow.sunPower) * MAX_PWM;
    }
  }
}

void readWeather(void)
{
  if (isСonnectedSensors)
  {
    weatherNow.sunPower = voltagesNow.sunPower;
#ifdef Celsius
    weatherNow.temperature = aht20.readTemperature();
#endif
#ifdef Fahrenheit
    weatherNow.temperature = aht20.readTemperature() * 9 / 5 + 32;
#endif
#ifdef Kelvin
    weatherNow.temperature = aht20.readTemperature() + 273, 15d;
#endif
    weatherNow.humidity = aht20.readHumidity();
#ifdef millimetreOfMercury
    weatherNow.pressure = bmp280.readPressure() * 0.00750062d;
#endif
#ifdef KPa
    weatherNow.pressure = bmp280.readPressure();
#endif
#ifdef bar
    weatherNow.pressure = bmp280.readPressure() * 0.00001d;
#endif
  }
}

#ifdef LCD
void controlDisplay(void)
{
  static DisplayMenu displayMenuNow = CONNECTION;
  static Timer butTimer = Timer(0, 0);
  static Timer backlightTimer = Timer(std::numeric_limits<int32_t>::min(), std::numeric_limits<int32_t>::min());

  if (!digitalRead(CH_MENU_BUT) && butTimer.everyMilli(250))
  {
    lcd.backlight();
    lcd.clear();
    backlightTimer.setTimerToZero();
    if (displayMenuNow == VOLTAGES)
    {
      displayMenuNow = CONNECTION;
    }
    else
    {
      displayMenuNow = static_cast<DisplayMenu>(displayMenuNow + 1);
    }
  }

  if (backlightTimer.everyMilli(300000))
  {
    weatherNow.sunPower ? lcd.backlight() : lcd.noBacklight();
  }

  switch (displayMenuNow)
  {
  case CONNECTION:
    if (WiFi.status() == WL_CONNECTED)
    {
      lcd.print("Connected");
      lcd.setCursor(0, 1);
      lcd.print(WiFi.localIP().toString());
    }
    else if (alreadyAP)
    {
      lcd.print(String(wifiValuesNow.apSsid));
      lcd.setCursor(0, 1);
      lcd.print(String(wifiValuesNow.apPassword));
    }
    else
    {
      displayMenuNow = WEATHER;
    }
    lcd.home();
    break;
  case WEATHER:
    if (weatherNow.sunPower > 0.0)
    {
      lcd.print(String(weatherNow.sunPower) + " ");
    }
    lcd.print(String(weatherNow.temperature));
    lcd.setCursor(0, 1);
    lcd.print(String(weatherNow.pressure) + " " + String(weatherNow.humidity));
    lcd.home();
    break;
  case VOLTAGES:
    if (weatherNow.sunPower > 0.0)
    {
      lcd.print(String(voltagesNow.sysBatVol) + " " + String(voltagesNow.solBatVol) + " " + String(voltagesNow.sysBatCur));
      lcd.setCursor(0, 1);
      lcd.print(String(voltagesNow.sunPower) + " " + String(voltagesNow.maxPowerPointVol) + " " + String(voltagesNow.PWM));
    }
    else
    {
      lcd.print(String(voltagesNow.sysBatVol));
      lcd.setCursor(0, 1);
      lcd.print(String(voltagesNow.solBatVol));
    }
    lcd.home();
    break;
  default:
    break;
  }
}
#endif

void maxPointPowerTracking(void)
{
  if (isСonnectedADS || voltagesNow.sunPower > 0.0)
  {
    static bool isLessVol;
    static bool isMoreVol;
    readVoltages();
    analogWrite(PWM_PIN, voltagesNow.PWM);
    if (maxPWM)
    {
      voltagesNow.PWM = MAX_PWM;
    }
    else if (minPWM)
    {
      voltagesNow.PWM = 0;
    }
    else
    {
      readVoltages();
      isLessVol = voltagesNow.solBatVol + HYSTERESIS < voltagesNow.maxPowerPointVol && voltagesNow.PWM > 0;
      isMoreVol = voltagesNow.solBatVol - HYSTERESIS > voltagesNow.maxPowerPointVol && voltagesNow.PWM < MAX_PWM;

      if (isLessVol)
      {
        if (voltagesNow.PWM > 0)
        {
          voltagesNow.PWM--;
        }
      }
      else if (isMoreVol)
      {
        if (voltagesNow.PWM < MAX_PWM)
        {
          voltagesNow.PWM++;
        }
      }
    }
  }

  analogWrite(PWM_PIN, voltagesNow.PWM);
}

void batControl(void)
{

  if (isСonnectedADS)
  {
    if (voltagesNow.sysBatVol < MIN_BAT_VOL)
    {
      digitalWrite(OUT_SWITCH_PIN, HIGH);
      isBatDisconected = true;
      saveMode = true;
    }
    else if (voltagesNow.sysBatVol > MIN_VOL_TO_CONNECT_BAT && isBatDisconected)
    {
      digitalWrite(OUT_SWITCH_PIN, LOW);
      isBatDisconected = false;
      if (!presetSaveMode)
      {
        saveMode = false;
      }
    }
    if (saveMode && (voltagesNow.sunPower == 0.0) && (timeNow.minute == 1))
    {
      analogWrite(PWM_PIN, 50);
      sleep(3500000); // sleep a little bit less than hour
      readVoltages();
    }
  }
}
