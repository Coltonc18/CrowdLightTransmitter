#pragma once
#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "Config.h"
#include "ConfigData.h"

enum ScreenState {
    SCREEN_BOOT,
    // Slideshow States
    SCREEN_STATUS_IP,      
    SCREEN_STATUS_E131,    
    SCREEN_STATUS_SENSORS, 
    // Menu States
    SCREEN_MENU_MAIN,
    // Edit States
    SCREEN_EDIT_UNIVERSE,
    SCREEN_EDIT_NUM_LEDS,
    SCREEN_EDIT_IP // Placeholder for future implementation
};

enum E131Status {
    STATUS_DISCONNECTED, 
    STATUS_CONNECTED,    
    STATUS_ACTIVE,       
    STATUS_IDLE          
};

class DisplayMgr {
public:
    DisplayMgr();
    void begin();

    void render(DeviceConfig& config, IPAddress currentIP, E131Status netStatus);
    void handleButtonPress(int button, DeviceConfig& config, void (*saveCallback)(const DeviceConfig&));

private:
    Adafruit_SSD1306 _oled;
    ScreenState _currentState;
    unsigned long _lastSlideshowTime = 0;
    
    // Menu Navigation
    int _menuIndex = 0;

    // Helpers
    void _drawHeader();
    void _drawStatusIP(IPAddress ip, bool dhcp);
    void _drawStatusE131(uint16_t universe, uint16_t numLeds, E131Status status);
    void _drawStatusSensors();
    void _drawMainMenu();
    void _drawEditScreen(const char* title, int value);
    
    void _slideshowLogic(uint16_t intervalMs);
};