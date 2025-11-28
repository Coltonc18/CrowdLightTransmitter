#include <Arduino.h>
#include <nvs_flash.h>
#include "Config.h"
#include "ConfigData.h"
#include "Logger.h"
#include "ConfigManager.h"
#include "E131Handler.h"
#include "RadioLink.h"
#include "DisplayMgr.h"

// Objects
ConfigManager configMgr;
DisplayMgr displayMgr; 
E131Handler eth;
RadioLink radio;

// Shared Data
DeviceConfig deviceConfig;
volatile bool packetReceived = false;
unsigned long lastPacketTime = 0;
uint8_t sharedDmxData[512];

// Tasks
TaskHandle_t NetworkTaskHandle;
TaskHandle_t DisplayTaskHandle;
TaskHandle_t InputTaskHandle;

// Callback
void saveConfigCallback(const DeviceConfig& cfg) {
    configMgr.saveConfig(cfg);
    // Update live settings
    eth.setUniverse(cfg.universe);
}

// --- CORE 0: Network ---
void networkLoop(void * parameter) {
    uint8_t localDmxBuffer[DMX_MAX_CHANNELS]; 
    
    // Note: We use static MAC, but IP is loaded from config
    byte mac[] = DEFAULT_MAC;
    IPAddress currentIP(deviceConfig.ipAddress);

    eth.begin(mac, currentIP);
    eth.setUniverse(deviceConfig.universe);
    radio.begin();

    for(;;) {
        if (eth.checkHardware()) {
            int len = eth.parsePacket(localDmxBuffer);
            if (len > 0) {
                // Determine LEDs to send based on Config
                int bytesToSend = CHAN_PER_LED * deviceConfig.numLeds;
                if (bytesToSend > len) bytesToSend = len; 
                
                radio.sendDmxPacket(localDmxBuffer, bytesToSend);

                memcpy(sharedDmxData, localDmxBuffer, len);
                lastPacketTime = millis();
                packetReceived = true;
                
                neopixelWrite(NEOPIXEL, localDmxBuffer[0], localDmxBuffer[1], localDmxBuffer[2]);
            }
            vTaskDelay(1);
        } else {
            vTaskDelay(100); 
        }        
    }
}

// --- CORE 1: Buttons ---
void buttonInputLoop(void * parameter) {
    int pins[] = {BTN_UP, BTN_DOWN, BTN_LEFT, BTN_RIGHT, BTN_SEL};
    int lastState[] = {HIGH, HIGH, HIGH, HIGH, HIGH};

    for(;;) {
        for(int i=0; i<5; i++) {
            int read = digitalRead(pins[i]);
            if(read == LOW && lastState[i] == HIGH) {
                // Button Pressed
                displayMgr.handleButtonPress(pins[i], deviceConfig, saveConfigCallback);
            }
            lastState[i] = read;
        }
        vTaskDelay(50); // Debounce
    }
}

// --- CORE 1: Display ---
void displayLoop(void * parameter) {
    for (;;) {
        vTaskDelay(100); // 10 FPS

        // Determine Status
        E131Status status = STATUS_DISCONNECTED;
        if (Ethernet.linkStatus() == LinkON) {
            if (millis() - lastPacketTime < 2500) {
                status = STATUS_ACTIVE;
            } else if (lastPacketTime > 0) { 
                status = STATUS_IDLE; 
            } else {
                status = STATUS_CONNECTED;
            }
        } else {
            status = STATUS_DISCONNECTED;
        }

        IPAddress currentIP(deviceConfig.ipAddress);
        displayMgr.render(deviceConfig, currentIP, status);
    }
}

void setup() {
    Serial.begin(115200);
    delay(100); // Allow serial to initialize
    
    // 1. Initialize Logger
    Logger::begin();
    LOG_INFO_TAG("SYSTEM", "=== CrowdLight Transmitter Starting ===");
    
#ifdef DEBUG_TESTS
    Logger::runTests();
#endif

    // 2. Config
    LOG_INFO_TAG("SYSTEM", "Initializing configuration...");
    configMgr.begin();
    configMgr.loadConfig(deviceConfig);

    // 3. Buttons
    pinMode(BTN_UP, INPUT_PULLUP);
    pinMode(BTN_DOWN, INPUT_PULLUP);
    pinMode(BTN_LEFT, INPUT_PULLUP);
    pinMode(BTN_RIGHT, INPUT_PULLUP);
    pinMode(BTN_SEL, INPUT_PULLUP);

    // 4. Display
    LOG_INFO_TAG("SYSTEM", "Initializing display...");
    displayMgr.begin();

    // 5. Tasks
    LOG_INFO_TAG("SYSTEM", "Creating FreeRTOS tasks...");
    xTaskCreatePinnedToCore(networkLoop, "NetTask", 10000, NULL, 1, &NetworkTaskHandle, 0);
    LOG_INFO_TAG("SYSTEM", "Network task created on Core 0");
    xTaskCreatePinnedToCore(displayLoop, "DispTask", 10000, NULL, 1, &DisplayTaskHandle, 1);
    LOG_INFO_TAG("SYSTEM", "Display task created on Core 1");
    xTaskCreatePinnedToCore(buttonInputLoop, "InTask", 4096, NULL, 1, &InputTaskHandle, 1);
    LOG_INFO_TAG("SYSTEM", "Input task created on Core 1");
    LOG_INFO_TAG("SYSTEM", "=== System Ready ===");
}

void loop() {}
