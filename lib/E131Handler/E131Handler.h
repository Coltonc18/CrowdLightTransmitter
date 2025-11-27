#pragma once
#include <SPI.h>
#include <Ethernet.h>
#include <EthernetUdp.h>
#include "Config.h"

class E131Handler {
public:
    void begin(byte* mac, IPAddress ip);
    bool checkHardware(); // Returns false if Eth cable/shield missing
    int parsePacket(uint8_t* dmxOutputBuffer); // Returns # of bytes read
    
private:
    EthernetUDP _udp;
    uint8_t _packetBuffer[PACKET_BUFFER_SIZE];
};