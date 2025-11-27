#pragma once
#include <SPI.h>
#include <Ethernet.h>
#include <EthernetUdp.h>
#include "Config.h"

class E131Handler {
public:
    void begin(byte* mac, IPAddress ip);
    void setUniverse(uint16_t universe);
    bool checkHardware(); 
    int parsePacket(uint8_t* dmxOutputBuffer); 
    
private:
    EthernetUDP _udp;
    uint8_t _packetBuffer[E131_MAX_PACKET_SIZE];
    uint16_t _universe = DEFAULT_UNIVERSE;
};