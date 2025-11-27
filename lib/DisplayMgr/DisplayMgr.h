#pragma once
#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "Config.h"

// The different "Pages" your screen can show
enum ScreenState {
    SCREEN_BOOT,
    SCREEN_STATUS,
    SCREEN_MENU,
    SCREEN_SETTINGS
};

class DisplayMgr {
public:
    DisplayMgr(); // Constructor
    
    // Hardware Init
    void begin();

    // The main update function called from loop()
    // We pass in all the dynamic data needed for the status screen
    void render(IPAddress ip, uint16_t universe, bool hasData, int len, uint8_t *dmxData);

    // Future: Call this when button is pressed to change screens
    void toggleMenu(); 

private:
    Adafruit_SSD1306 _oled;
    ScreenState _currentState;

    // Helper functions to draw specific pages
    void _drawStatusPage(IPAddress ip, uint16_t universe, bool hasData, int len, uint8_t *dmxData);
    void _drawMenuPage();
    void _drawHeader();
};