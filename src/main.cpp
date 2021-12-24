#include <Arduino.h>
#include <WiFi.h>
#include <ArduinoOTA.h>
#include "ESPDateTime.h"
#include "DateTimeTZ.h"
#include "NeoPixelBus.h"
#include "Cfg.h"
#include "CfgSecure.h"
#include "Alarm.h"

void connectToWifi();
void setupDateTime();
bool isDateInRange(uint8_t hour, uint8_t minute, Alarm alarm);
bool updateLight();
bool setColor(RgbColor color, uint8_t ratio = 255);

RgbColor black(0);
RgbColor red(255, 0, 0);
RgbColor green(0, 255, 0);
RgbColor orange(255, 128, 0);

NeoPixelBus<NeoGrbFeature, Neo800KbpsMethod> led(Cfg::ledCount, Cfg::pinLed);
RgbColor currentColor = black;
uint64_t lastTimeSync = 0;

Alarm alarms[] = 
{
  {true, 6, 0, 12, 30, green, 150},
  {true, 12, 30, 12, 50, orange, 255},
  {true, 12, 50, 15, 0, red, 255},
  {true, 15, 0, 20, 30, green, 255},
  {true, 20, 30, 20, 50, orange, 200},
  {true, 20, 50, 6, 0, red, 100},
};

void setup()
{
  Serial.begin(115200);
  Serial.println("Starting...");

  led.Begin();
  led.Show();

  WiFi.mode(WIFI_STA);
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
    
  if (DateTime.isTimeValid())
    updateLight();
  else
    setColor(black);  
}

bool updateLight()
{
  DateTimeParts dt = DateTime.getParts();
  for (uint8_t i = 0; i < sizeof(alarms)/sizeof(alarms[0]); i++)
  {
    Alarm alarm = alarms[i];
    if (isDateInRange(dt.getHours(), dt.getMinutes(), alarm))
    {
      setColor(alarm.color, alarm.colorRatio);
    }
  }
}

bool setColor(RgbColor color, uint8_t ratio)
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
  DateTime.setServer("pool.ntp.org");
  DateTime.setTimeZone(TZ_Europe_Kiev);
  DateTime.begin();

  if (!DateTime.isTimeValid())
  {
    Serial.println("Failed to get time from server.");
    lastTimeSync = 0;
    return;
  }
  lastTimeSync = millis();
  Serial.printf("Date Now is %s\n", DateTime.toISOString().c_str());
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
