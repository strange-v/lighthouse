#include <Arduino.h>
#include <WiFi.h>
#include <ArduinoOTA.h>
#include <Time.h>
#include "NeoPixelBus.h"
#include "Cfg.h"
#include "CfgSecure.h"
#include "Alarm.h"

void connectToWifi();
void setupDateTime();
bool isTimeSynchronized();
bool isDateInRange(uint8_t hour, uint8_t minute, Alarm alarm);
void updateLight();
void setColor(RgbColor color, uint8_t ratio = 255);

RgbColor black(0);
RgbColor red(255, 0, 0);
RgbColor green(0, 255, 0);
RgbColor blue(0, 0, 255);
RgbColor orange(255, 128, 0);

NeoPixelBus<NeoGrbFeature, Neo800KbpsMethod> led(Cfg::ledCount, Cfg::pinLed);
RgbColor currentColor = black;
uint64_t lastTimeSync = 0;

Alarm alarms[] = 
{
  {true, 6, 45, 12, 30, green, 100},
  {true, 12, 30, 12, 50, blue, 255},
  {true, 12, 50, 14, 40, red, 255},
  {true, 14, 40, 20, 30, green, 255},
  {true, 20, 30, 20, 50, blue, 200},
  {true, 20, 50, 6, 30, red, 100},
};

void setup()
{
  Serial.begin(115200);
  Serial.println("Starting...");

  led.Begin();
  led.Show();

  connectToWifi();
  setupDateTime();

  ArduinoOTA.setPassword(CfgSecure::otaPassword);
  ArduinoOTA.begin();
}

void loop()
{
  ArduinoOTA.handle();

  uint64_t ms = millis();
  if (ms - lastTimeSync >= Cfg::syncTimeInterval || ms < lastTimeSync)
  {
    if (WiFi.status() == WL_CONNECTED)
      setupDateTime();
  }
    
  if (isTimeSynchronized())
    updateLight();
  else
    setColor(black);  
}

void updateLight()
{
  time_t now;
  tm local;

  time(&now);
  localtime_r(&now, &local);

  for (uint8_t i = 0; i < sizeof(alarms)/sizeof(alarms[0]); i++)
  {
    Alarm alarm = alarms[i];
    if (isDateInRange(local.tm_hour, local.tm_min, alarm))
    {
      setColor(alarm.color, alarm.colorRatio);
    }
  }
}

void setColor(RgbColor color, uint8_t ratio)
{
  if (currentColor != color)
  {
    currentColor = color.Dim(ratio);
    led.SetPixelColor(0, currentColor);
    led.Show();
  }
}

void connectToWifi()
{
  WiFi.mode(WIFI_MODE_NULL);
  WiFi.setHostname("lighthouse");
  WiFi.mode(WIFI_STA);
  WiFi.begin(CfgSecure::wifiSsid, CfgSecure::wifiPassword);
  
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.println(WiFi.localIP());
}

void setupDateTime()
{
  configTime(0, 0, "ua.pool.ntp.org");
  isTimeSynchronized();

  setenv("TZ", "WET0WEST,M3.5.0/1,M10.5.0", 5);
  tzset();
}

bool isTimeSynchronized()
{
    time_t now;
    time(&now);

    tm local;
    localtime_r(&now, &local);

    return local.tm_year > (2016 - 1900);
}

bool isDateInRange(uint8_t hour, uint8_t minute, Alarm alarm)
{
  uint16_t midnight = 1440;
  uint16_t current = hour * 60 + minute;

  if (alarm.getFrom() < alarm.getTo())
  {
    return current >= alarm.getFrom() && current < alarm.getTo();
  }
  else
  {
    return (current >= alarm.getFrom() && current < midnight)
      || (current < alarm.getTo());
  }
}
