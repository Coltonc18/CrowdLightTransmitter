#include "E131Handler.h"

void E131Handler::begin(byte* mac, IPAddress ip) {
    // Init SPI with specific pins
    SPI.begin(ETH_SCK, ETH_MISO, ETH_MOSI, ETH_CS);
    
    // Init Ethernet
    Ethernet.init(ETH_CS);
    Ethernet.begin(mac, ip);
    
    _udp.begin(E131_PORT);
}

bool E131Handler::checkHardware() {
    if (Ethernet.hardwareStatus() == EthernetNoHardware) return false;
    if (Ethernet.linkStatus() == LinkOFF) return false;
    return true;
}

int E131Handler::parsePacket(uint8_t* dmxOutputBuffer) {
    int packetSize = _udp.parsePacket();
    
    if (packetSize > 0) {
        _udp.read(_packetBuffer, PACKET_BUFFER_SIZE);

        // 1. Check Packet Size
        if (packetSize < 126) return 0; // Too small

        // 2. Check Universe (Bytes 113, 114)
        uint16_t rxUniverse = (_packetBuffer[113] << 8) | _packetBuffer[114];
        if (rxUniverse != UNIVERSE) return 0;

        // 3. Check Start Code (Byte 125)
        if (_packetBuffer[125] != DMX_STARTCODE) return 0;

        // 4. Extract Data
        uint16_t dmxLen = ((_packetBuffer[123] << 8) | _packetBuffer[124]) - 1;
        uint8_t* dmxDataPtr = &_packetBuffer[126];

        // Safety cap
        if(dmxLen > 512) dmxLen = 512;

        // Copy valid data to the output buffer
        memcpy(dmxOutputBuffer, dmxDataPtr, dmxLen);
        
        return dmxLen;
    }
    return 0;
}