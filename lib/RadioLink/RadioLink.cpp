#include "RadioLink.h"

void RadioLink::begin() {
    pinMode(HC12_SET, OUTPUT);
    
    // Enter Setup Mode
    digitalWrite(HC12_SET, LOW);
    delay(500);

    // Initialize Serial2
    // RX=18, TX=17
    _serial = &Serial2;
    _serial->begin(HC12_BAUD, SERIAL_8N1, HC12_RX, HC12_TX);

    // Check HC-12 Health
    _serial->print("AT+RX"); 
    delay(100);
    // Flush response for cleanliness
    while(_serial->available()) _serial->read();

    // Exit Setup Mode
    digitalWrite(HC12_SET, HIGH);
    delay(100);
}

void RadioLink::sendDmxPacket(uint8_t* dmxData, uint16_t length) {
    // Protocol: [0xAA] [Length] [Data...] [Checksum]
    
    _serial->write(0xAA);    // Start Byte
    _serial->write(length);  // Total channels
    
    uint8_t checksum = 0xAA;
    
    // Send data for each LED (already multiplied by CHAN_PER_LED when called)
    for (int i = 0; i < length; i++) {
        _serial->write(dmxData[i]);
        checksum ^= dmxData[i];
    }
    
    _serial->write(checksum);
}