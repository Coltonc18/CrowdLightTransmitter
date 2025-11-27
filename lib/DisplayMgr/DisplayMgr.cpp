#include "DisplayMgr.h"

DisplayMgr::DisplayMgr()
    : _oled(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1), _currentState(SCREEN_BOOT) {}

void DisplayMgr::begin() {
    // We handle the Wire initialization here to keep main.cpp clean
    Wire.begin(DISPLAY_SDA, DISPLAY_SCL);

    if(!_oled.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
        Serial.println(F("Display Alloc Failed"));
        // In a real app, maybe blink an LED code here
    }
    
    _oled.clearDisplay();
    _oled.setTextColor(SSD1306_WHITE);
    _oled.setTextSize(1);
    
    // Show Boot Screen immediately
    _oled.setCursor(20, 25);
    _oled.println(F("SYSTEM BOOT..."));
    _oled.display();
    delay(1000); // Visual pause
    
    _currentState = SCREEN_STATUS;
}

void DisplayMgr::toggleMenu() {
    // Simple toggle for now. 
    // Later you can make this cycle: Status -> Menu -> Settings -> Status
    if (_currentState == SCREEN_STATUS) {
        _currentState = SCREEN_MENU;
    } else {
        _currentState = SCREEN_STATUS;
    }
}

void DisplayMgr::render(IPAddress ip, uint16_t universe, bool hasData, int len, uint8_t *dmxData) {
    _oled.clearDisplay();
    _drawHeader();

    switch(_currentState) {
        case SCREEN_STATUS:
            _drawStatusPage(ip, universe, hasData, len, dmxData);
            break;
            
        case SCREEN_MENU:
            _drawMenuPage();
            break;
            
        default:
            break;
    }

    _oled.display();
}

void DisplayMgr::_drawHeader() {
    _oled.setCursor(0, 0);
    _oled.print(F("ESP32-S3 DMX Node"));
    _oled.drawLine(0, 8, 128, 8, SSD1306_WHITE);
}

void DisplayMgr::_drawStatusPage(IPAddress ip, uint16_t universe, bool hasData, int len, uint8_t *dmxData) {
    _oled.setCursor(0, 12);
    _oled.print(F("IP: ")); _oled.println(ip);
    _oled.print(F("Univ: ")); _oled.println(universe);

    _oled.setCursor(0, 35);
    if (hasData) {
        _oled.print(F("STATUS: RECEIVING"));
        _oled.setCursor(0, 45);
        _oled.printf("LED 0: (%d, %d, %d)\n", dmxData[0], dmxData[1], dmxData[2]);
    } else {
        _oled.print(F("STATUS: NO SIGNAL"));
        _oled.setCursor(0, 45);
        _oled.println(F("Check E1.31 Source"));
    }
}

void DisplayMgr::_drawMenuPage() {
    _oled.setCursor(0, 15);
    _oled.println(F("--- MAIN MENU ---"));
    _oled.println(F("> Set Universe"));
    _oled.println(F("  Set IP Address"));
    _oled.println(F("  Test Radio"));
    _oled.println(F("  Exit"));
    
    // In the future, you will pass an 'index' here to 
    // highlight the selected row
}