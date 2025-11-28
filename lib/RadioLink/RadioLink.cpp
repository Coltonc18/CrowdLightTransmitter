#include "RadioLink.h"
#include "Logger.h"

void RadioLink::begin() {
    LOG_INFO_TAG("RADIO", "Initializing HC-12 radio...");
    pinMode(HC12_SET, OUTPUT);
    
    // Enter Setup Mode
    digitalWrite(HC12_SET, LOW);
    delay(500);
    LOG_DEBUG_TAG("RADIO", "Entered AT command mode");

    // Initialize Serial2
    // RX=18, TX=17
    _serial = &Serial2;
    _serial->begin(HC12_BAUD, SERIAL_8N1, HC12_RX, HC12_TX);

    // Check HC-12 Health
    _serial->print("AT+RX"); 
    delay(100);
    
    // Check for response
    if (_serial->available()) {
        String response = "";
        while(_serial->available()) {
            response += (char)_serial->read();
        }
        LOG_DEBUG_TAG("RADIO", "HC-12 response: %s", response.c_str());
    } else {
        LOG_WARN_TAG("RADIO", "No response from HC-12 module");
    }

    // Exit Setup Mode
    digitalWrite(HC12_SET, HIGH);
    delay(100);
    LOG_INFO_TAG("RADIO", "HC-12 initialized, exited AT mode");
}

void RadioLink::sendDmxPacket(uint8_t* dmxData, uint16_t length) {
    // Protocol: [0xAA] [Length] [Data...] [Checksum]
    
    if (length > 255) {
        LOG_ERROR_TAG("RADIO", "Packet too large: %d bytes", length);
        return;
    }
    
    _serial->write(0xAA);    // Start Byte
    _serial->write(length);  // Total channels
    
    uint8_t checksum = 0xAA;
    
    // Send data for each LED (already multiplied by CHAN_PER_LED when called)
    for (int i = 0; i < length; i++) {
        _serial->write(dmxData[i]);
        checksum ^= dmxData[i];
    }
    
    _serial->write(checksum);
    
    LOG_VERBOSE_TAG("RADIO", "Sent %d bytes via HC-12", length + 3); // +3 for header, length, checksum
}
