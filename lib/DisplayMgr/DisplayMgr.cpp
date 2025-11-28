#include "DisplayMgr.h"
#include "Logger.h"

DisplayMgr::DisplayMgr() : _oled(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1), _currentState(SCREEN_BOOT) {}

void DisplayMgr::begin() {
    LOG_INFO_TAG("DISPLAY", "Initializing OLED display...");
    Wire.begin(DISPLAY_SDA, DISPLAY_SCL);
    if(!_oled.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
        LOG_ERROR_TAG("DISPLAY", "OLED initialization failed at address 0x%02X", OLED_ADDR);
    } else {
        LOG_INFO_TAG("DISPLAY", "OLED initialized successfully");
    }
    _oled.clearDisplay();
    
    _currentState = SCREEN_STATUS_IP;
}

void DisplayMgr::render(DeviceConfig& config, IPAddress currentIP, E131Status netStatus) {
    _oled.clearDisplay();
    _drawHeader();

    // Auto-cycle slideshow if in status mode
    if (_currentState >= SCREEN_STATUS_IP && _currentState <= SCREEN_STATUS_SENSORS) {
        _slideshowLogic(STATUS_SCREEN_LENGTH_MS);
    }

    switch(_currentState) {
        case SCREEN_STATUS_IP:
            _drawStatusIP(currentIP, config.useDhcp);
            break;
        case SCREEN_STATUS_E131:
            _drawStatusE131(config.universe, config.numLeds, netStatus);
            break;
        case SCREEN_STATUS_SENSORS:
            _drawStatusSensors();
            break;
        case SCREEN_MENU_MAIN:
            _drawMainMenu();
            break;
        case SCREEN_EDIT_UNIVERSE:
            _drawEditScreen("SET UNIVERSE", config.universe);
            break;
        case SCREEN_EDIT_NUM_LEDS:
            _drawEditScreen("SET NUM LEDS", config.numLeds);
            break;
        default: break;
    }
    _oled.display();
}

void DisplayMgr::_drawHeader() {
    _oled.setCursor(0, 0);
    _oled.print(F("CrowdLight TX"));
    _oled.drawLine(0, 8, 128, 8, SSD1306_WHITE);
}

void DisplayMgr::_drawStatusIP(IPAddress ip, bool dhcp) {
    _oled.setCursor(0, 15);
    _oled.print(F("Mode: ")); _oled.println(dhcp ? F("DHCP") : F("STATIC"));
    _oled.println();
    _oled.print(F("IP: ")); _oled.println(ip);
}

void DisplayMgr::_drawStatusE131(uint16_t universe, uint16_t numLeds, E131Status status) {
    _oled.setCursor(0, 15);
    _oled.print(F("Univ: ")); _oled.println(universe);
    _oled.print(F("LEDs: ")); _oled.println(numLeds);
    _oled.println();
    _oled.print(F("Stat: "));
    
    switch(status) {
        case STATUS_DISCONNECTED: _oled.print(F("NO CABLE")); break;
        case STATUS_CONNECTED:    _oled.print(F("LINK UP")); break;
        case STATUS_ACTIVE:       _oled.print(F("RECEIVING")); break;
        case STATUS_IDLE:         _oled.print(F("IDLE")); break;
    }
}

void DisplayMgr::_drawStatusSensors() {
    _oled.setCursor(0, 15);
    _oled.println(F("Sensors:"));
    _oled.setCursor(0, 30);
    _oled.print(F("Input Voltage: --.- V")); 
    _oled.setCursor(0, 45);
    _oled.print(F("Temperature: --.- F"));
}

void DisplayMgr::_drawMainMenu() {
    const char* items[] = {"Exit", "Set Universe", "Set Num LEDs"};
    _oled.setCursor(0, 15);
    for(int i=0; i<3; i++) {
        if(i == _menuIndex) _oled.print(F("> "));
        else _oled.print(F("  "));
        _oled.println(items[i]);
    }
}

void DisplayMgr::_drawEditScreen(const char* title, int value) {
    _oled.setCursor(0, 15);
    _oled.println(title);
    _oled.setCursor(10, 35);
    _oled.setTextSize(2);
    _oled.print(value);
    _oled.setTextSize(1);
    _oled.setCursor(110, 35);
    _oled.print(F("<>"));
}

void DisplayMgr::_slideshowLogic(uint16_t intervalMs) {
    if (millis() - _lastSlideshowTime > intervalMs) {
        _lastSlideshowTime = millis();
        int next = (int)_currentState + 1;
        if (next > SCREEN_STATUS_SENSORS) next = SCREEN_STATUS_IP;
        _currentState = (ScreenState)next;
    }
}

void DisplayMgr::handleButtonPress(int button, DeviceConfig& config, void (*saveCallback)(const DeviceConfig&)) {
    LOG_DEBUG_TAG("DISPLAY", "Button pressed: %d", button);
    
    // 1. Any button interrupts slideshow
    if (_currentState <= SCREEN_STATUS_SENSORS) {
        _currentState = SCREEN_MENU_MAIN;
        _menuIndex = 0;
        LOG_DEBUG_TAG("DISPLAY", "Entered menu mode");
        return;
    }

    // 2. Main Menu
    if (_currentState == SCREEN_MENU_MAIN) {
        if (button == BTN_UP)   _menuIndex = max(0, _menuIndex - 1);
        if (button == BTN_DOWN) _menuIndex = min(2, _menuIndex + 1); 
        if (button == BTN_SEL) {
            switch(_menuIndex) {
                case 0: 
                    _currentState = SCREEN_STATUS_IP;
                    LOG_DEBUG_TAG("DISPLAY", "Exited menu");
                    break; 
                case 1: 
                    _currentState = SCREEN_EDIT_UNIVERSE;
                    LOG_DEBUG_TAG("DISPLAY", "Editing universe");
                    break;
                case 2: 
                    _currentState = SCREEN_EDIT_NUM_LEDS;
                    LOG_DEBUG_TAG("DISPLAY", "Editing num LEDs");
                    break;
            }
        }
    }

    // 3. Edit Screens
    else if (_currentState == SCREEN_EDIT_UNIVERSE || _currentState == SCREEN_EDIT_NUM_LEDS) {
        uint16_t* target = (_currentState == SCREEN_EDIT_UNIVERSE) ? &config.universe : &config.numLeds;
        
        switch (button) {
            case BTN_UP:
                if (_currentState == SCREEN_EDIT_UNIVERSE) {
                    if (*target < MAX_UNIVERSE) (*target)++;
                } else if (_currentState == SCREEN_EDIT_NUM_LEDS) {
                    if (*target < MAX_NUM_LEDS) (*target)++;
                } else {
                    (*target)++;
                }
                break;
            case BTN_DOWN:
                if (_currentState == SCREEN_EDIT_UNIVERSE) {
                    if (*target > MIN_UNIVERSE) (*target)--;
                } else if (_currentState == SCREEN_EDIT_NUM_LEDS) {
                    if (*target > MIN_NUM_LEDS) (*target)--;
                } else {
                    if (*target > 0) (*target)--;
                }
                break;
            case BTN_LEFT:
                break;
            case BTN_RIGHT:
                break;
            case BTN_SEL:
                LOG_INFO_TAG("DISPLAY", "Saving configuration changes");
                saveCallback(config);
                _currentState = SCREEN_MENU_MAIN;
                break;
            default:
                break;
        }
    }
}
