#pragma once
#include <Arduino.h>
#include "Config.h"

class RadioLink {
public:
    void begin();
    void sendDmxPacket(uint8_t* dmxData, uint16_t length);
private:
    HardwareSerial* _serial;
};