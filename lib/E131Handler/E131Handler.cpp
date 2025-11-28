#include "E131Handler.h"
#include "Logger.h"

void E131Handler::begin(byte* mac, IPAddress ip) {
    LOG_INFO_TAG("E131", "Initializing Ethernet...");
    SPI.begin(ETH_SCK, ETH_MISO, ETH_MOSI, ETH_CS);
    Ethernet.init(ETH_CS);
    
    if (Ethernet.hardwareStatus() == EthernetNoHardware) {
        LOG_ERROR_TAG("E131", "W5500 hardware not detected!");
    } else {
        LOG_DEBUG_TAG("E131", "W5500 hardware detected");
    }
    
    Ethernet.begin(mac, ip);
    _udp.begin(E131_PORT);
    
    LOG_INFO_TAG("E131", "Listening on port %d, IP: %d.%d.%d.%d", 
                 E131_PORT, ip[0], ip[1], ip[2], ip[3]);
}

void E131Handler::setUniverse(uint16_t universe) {
    if (_universe != universe) {
        _universe = universe;
        LOG_INFO_TAG("E131", "Universe changed to %d", universe);
    }
}

bool E131Handler::checkHardware() {
    static bool lastHardwareOk = true;
    static bool lastLinkOk = true;
    
    bool hardwareOk = (Ethernet.hardwareStatus() != EthernetNoHardware);
    bool linkOk = (Ethernet.linkStatus() != LinkOFF);
    
    // Log status changes
    if (hardwareOk != lastHardwareOk) {
        if (!hardwareOk) {
            LOG_ERROR_TAG("E131", "Hardware failure detected");
        }
        lastHardwareOk = hardwareOk;
    }
    
    if (linkOk != lastLinkOk) {
        if (linkOk) {
            LOG_INFO_TAG("E131", "Link UP - cable connected");
        } else {
            LOG_WARN_TAG("E131", "Link DOWN - cable disconnected");
        }
        lastLinkOk = linkOk;
    }
    
    return hardwareOk && linkOk;
}

int E131Handler::parsePacket(uint8_t* dmxOutputBuffer) {
    int packetSize = _udp.parsePacket();
    
    if (packetSize > 0) {
        _udp.read(_packetBuffer, min(packetSize, E131_MAX_PACKET_SIZE));

        if (packetSize < E131_HEADER_SIZE) {
            LOG_WARN_TAG("E131", "Packet too small: %d bytes", packetSize);
            return 0;
        }

        // Check Universe
        uint16_t rxUniverse = (_packetBuffer[E131_UNIVERSE_OFFSET] << 8) |
            _packetBuffer[E131_UNIVERSE_OFFSET+1];
        if (rxUniverse != _universe) {
            LOG_DEBUG_TAG("E131", "Universe mismatch: got %d, expected %d", rxUniverse, _universe);
            return 0;
        }

        // Check DMX start code
        if (_packetBuffer[E131_LENGTH_OFFSET+2] != DMX_STARTCODE) {
            LOG_WARN_TAG("E131", "Invalid DMX start code: 0x%02X", _packetBuffer[E131_LENGTH_OFFSET+2]);
            return 0;
        }

        uint16_t dmxLen = ((_packetBuffer[E131_LENGTH_OFFSET] << 8) |
            _packetBuffer[E131_LENGTH_OFFSET+1]) - 1;
        uint8_t* dmxDataPtr = &_packetBuffer[E131_HEADER_SIZE];

        if(dmxLen > DMX_MAX_CHANNELS) dmxLen = DMX_MAX_CHANNELS;
        memcpy(dmxOutputBuffer, dmxDataPtr, dmxLen);
        
        LOG_VERBOSE_TAG("E131", "Packet received: %d channels", dmxLen);
        return dmxLen;
    }
    return 0;
}
