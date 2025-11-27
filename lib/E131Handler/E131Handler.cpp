#include "E131Handler.h"

void E131Handler::begin(byte* mac, IPAddress ip) {
    SPI.begin(ETH_SCK, ETH_MISO, ETH_MOSI, ETH_CS);
    Ethernet.init(ETH_CS);
    Ethernet.begin(mac, ip);
    _udp.begin(E131_PORT);
}

void E131Handler::setUniverse(uint16_t universe) {
    _universe = universe;
}

bool E131Handler::checkHardware() {
    if (Ethernet.hardwareStatus() == EthernetNoHardware) return false;
    if (Ethernet.linkStatus() == LinkOFF) return false;
    return true;
}

int E131Handler::parsePacket(uint8_t* dmxOutputBuffer) {
    int packetSize = _udp.parsePacket();
    
    if (packetSize > 0) {
        _udp.read(_packetBuffer, min(packetSize, E131_MAX_PACKET_SIZE));

        if (packetSize < E131_HEADER_SIZE) return 0; 

        // Check Universe using the variable, not constant
        uint16_t rxUniverse = (_packetBuffer[E131_UNIVERSE_OFFSET] << 8) |
            _packetBuffer[E131_UNIVERSE_OFFSET+1];
        if (rxUniverse != _universe) return 0;

        if (_packetBuffer[E131_LENGTH_OFFSET+2] != DMX_STARTCODE) return 0;

        uint16_t dmxLen = ((_packetBuffer[E131_LENGTH_OFFSET] << 8) |
            _packetBuffer[E131_LENGTH_OFFSET+1]) - 1;
        uint8_t* dmxDataPtr = &_packetBuffer[E131_HEADER_SIZE];

        if(dmxLen > DMX_MAX_CHANNELS) dmxLen = DMX_MAX_CHANNELS;
        memcpy(dmxOutputBuffer, dmxDataPtr, dmxLen);
        
        return dmxLen;
    }
    return 0;
}