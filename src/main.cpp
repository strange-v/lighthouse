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

RgbColor black(0);
RgbColor red(255, 0, 0);
RgbColor green(0, 255, 0);
RgbColor orange(255, 128, 0);

NeoPixelBus<NeoGrbFeature, Neo800KbpsMethod> led(Cfg::ledCount, Cfg::pinLed);
RgbColor currentColor = black;

Alarm alarms[] = 
{
  {true, 6, 0, 20, 30, green, 255},
  {true, 20, 30, 20, 55, orange, 255},
  {true, 20, 55, 6, 0, red, 100},
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

  DateTimeParts dt = DateTime.getParts();
  for (uint8_t i = 0; i < sizeof(alarms)/sizeof(alarms[0]); i++)
    {
      Alarm alarm = alarms[i];
      if (isDateInRange(dt.getHours(), dt.getMinutes(), alarm))
      {
        if (currentColor != alarm.color)
        {
          currentColor = alarm.color.Dim(alarm.colorRatio);
          led.SetPixelColor(0, currentColor);
          led.Show();
        }
      }
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
  DateTime.setServer("time.pool.aliyun.com");
  DateTime.setTimeZone(TZ_Europe_Kiev);
  DateTime.begin();

  if (!DateTime.isTimeValid())
  {
    Serial.println("Failed to get time from server.");
    return;
  }

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
