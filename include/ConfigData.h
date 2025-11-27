#ifndef CONFIG_DATA_H
#define CONFIG_DATA_H

#include <Arduino.h>

struct DeviceConfig {
    uint16_t universe;              // DMX Universe (e.g., 1)
    uint16_t numLeds;               // Number of pixels/LEDs
    uint32_t ipAddress;             // IP Address stored as a 32-bit integer
    bool useDhcp;                   // DHCP Mode
};

#endif