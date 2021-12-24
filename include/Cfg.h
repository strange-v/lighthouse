#ifndef CFG_h
#define CFG_h
#include <stdint.h>

namespace Cfg
{
    const uint8_t pinLed = 14;
    const uint8_t ledCount = 1;
    const uint64_t syncTimeInterval = 1 * 60 * 60 * 1000;
}
#endif