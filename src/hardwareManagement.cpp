#include "hardwareManagement.h"
#include "botManagement.h"

#ifndef DEBUG_ADC
static Adafruit_ADS1115 ads;
#endif

#ifndef DEBUG_WEATHER_SENSORS
static Adafruit_BMP280 bmp280;
static AHT10 aht20(AHT10_ADDRESS_0X38, AHT20_SENSOR);
#endif

#ifdef LCD
LiquidCrystal_I2C lcd(0x27, 20, 4);
#endif

extern FB_Time timeNow;
Voltages voltagesNow;
Weather weatherNow;
bool presetSaveMode = false;
bool isСonnectedADS = false;
bool isСonnectedSensors = false;

#ifndef DEBUG_ADC
static const uint32_t SLEEP_CHUNK = 268000; // 268 sec.
static uint32_t remains;
static bool isBatDisconected = false;
static bool saveMode = false;
extern bool maxPWM;
extern bool minPWM;
#endif

#ifndef DEBUG_ADC
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
  voltagesNow.solBatVol = ads.computeVolts(ads.readADC_SingleEnded(SOL_BAT_VOL_PIN)) * DIVIDER_SOL_BAT;
  voltagesNow.sysBatVol = ads.computeVolts(ads.readADC_SingleEnded(SYS_BAT_VOL_PIN)) * DIVIDER_SYS_BAT;
  voltagesNow.sysBatCur = voltagesNow.solBatVol > voltagesNow.sysBatVol ? -ads.computeVolts(ads.readADC_SingleEnded(SOL_BAT_CURRENT_PIN)) / SOL_BAT_SHUNT_RESISTANCE : 0.0;
  voltagesNow.sunPower = voltagesNow.solBatVol > voltagesNow.sysBatVol ? voltagesNow.sysBatVol * voltagesNow.sysBatCur : 0.0;
#ifdef LCD
  EVERY_MS(200)
  {
    lcd.print(String(voltagesNow.solBatVol) + String(voltagesNow.sysBatVol) + String(voltagesNow.sysBatCur) + String(voltagesNow.sunPower));
  }
#endif
}
#endif

void pinsConnection(void)
{
  pinMode(PWM_PIN, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(OUT_SWITCH_PIN, OUTPUT);
  pinMode(LED_WORK_PIN, OUTPUT);
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
#endif

#ifndef DEBUG_WEATHER_SENSORS

  isСonnectedADS = aht20.begin();
  isСonnectedSensors = bmp280.begin();

  bmp280.setSampling(Adafruit_BMP280::MODE_NORMAL,
                     Adafruit_BMP280::SAMPLING_X2,
                     Adafruit_BMP280::SAMPLING_X16,
                     Adafruit_BMP280::FILTER_X16,
                     Adafruit_BMP280::STANDBY_MS_500);
#endif

#ifndef DEBUG_ADC
  ads.setGain(GAIN_TWO);
  isСonnectedADS = ads.begin(0x4A);
#endif
}

void readWeather(void)
{
#ifndef DEBUG_WEATHER_SENSORS
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
#endif
}

void maxPointPowerTracking(void)
{
#ifndef DEBUG_ADC
  if (isСonnectedADS)
  {
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
      static bool isLessVol = voltagesNow.solBatVol + HYSTERESIS < voltagesNow.maxPowerPointVol && voltagesNow.PWM > 0;
      static bool isMoreVol = voltagesNow.solBatVol - HYSTERESIS > voltagesNow.maxPowerPointVol && voltagesNow.PWM < MAX_PWM;
      while (isLessVol || isMoreVol)
      {
        if (isLessVol)
        {
          voltagesNow.PWM--;
        }
        else if (isMoreVol)
        {
          voltagesNow.PWM++;
        }
        analogWrite(PWM_PIN, voltagesNow.PWM);

        botManage();

        readVoltages();

        isLessVol = voltagesNow.solBatVol + HYSTERESIS < voltagesNow.maxPowerPointVol && voltagesNow.PWM > 0;
        isMoreVol = voltagesNow.solBatVol - HYSTERESIS > voltagesNow.maxPowerPointVol && voltagesNow.PWM < MAX_PWM;
      }
    }
  }
#else
  analogWrite(PWM_PIN, voltagesNow.PWM);
#endif
}

void batControl(void)
{
#ifndef DEBUG_ADC
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
#endif
}
