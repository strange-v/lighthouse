#ifndef ALARM_h
#define ALARM_h
#include <stdint.h>
#include "NeoPixelBus.h"

struct Alarm
{
	bool enabled;
    uint8_t fromHour;
    uint8_t fromMinute;
    uint8_t toHour;
    uint8_t toMinute;
	RgbColor color;
    uint8_t colorRatio;

    uint16_t getFrom() {
        return fromHour * 60 + fromMinute;
    }

    uint16_t getTo() {
        return toHour * 60 + toMinute;
    }
};
#endif