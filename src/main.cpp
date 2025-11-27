#include <Arduino.h>
#include "Config.h"
#include "E131Handler.h"
#include "RadioLink.h"
#include "DisplayMgr.h"

// --- Objects ---
DisplayMgr displayMgr; 
E131Handler eth;
RadioLink radio;

// --- Network Settings ---
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress ip(192, 168, 0, 100);

// --- Shared Data ---
volatile bool packetReceived = false;
volatile int  lastDataLen = 0;
uint8_t sharedDmxData[512];

TaskHandle_t NetworkTaskHandle;
TaskHandle_t DisplayTaskHandle;

// ==========================================
// CORE 0: Network Logic
// ==========================================
void networkLoop(void * parameter) {
    uint8_t localDmxBuffer[512]; 
    eth.begin(mac, ip);
    radio.begin();

    for(;;) {
        if (eth.checkHardware()) {
            int len = eth.parsePacket(localDmxBuffer);
            if (len > 0) {
                // Radio TX
                int bytesToSend = 3 * NUM_LEDS;
                if (bytesToSend > len) bytesToSend = len; 
                radio.sendDmxPacket(localDmxBuffer, bytesToSend);

                // Update Shared Memory
                memcpy(sharedDmxData, localDmxBuffer, len);
                lastDataLen = len;
                packetReceived = true;
                
                neopixelWrite(NEOPIXEL, localDmxBuffer[0], localDmxBuffer[1], localDmxBuffer[2]);
            }
            vTaskDelay(1);
        } else {
            vTaskDelay(100); 
        }        
    }
}

// ==========================================
// CORE 1: UI & Orchestration
// ==========================================
void displayLoop(void * parameter) {
    for (;;) {
        // 1. Refresh Rate Control (10 FPS)
        vTaskDelay(100);

        // 2. Grab atomic copy of data (optional but good practice)
        // For simple display, direct access is okay given the volatile flags
        bool hasData = packetReceived;
        packetReceived = false; // Reset "New Packet" flag for visual effect

        // 3. Render
        displayMgr.render(
            ip, 
            UNIVERSE, 
            hasData, 
            lastDataLen, 
            sharedDmxData
        );
    }
}

void setup() {
    Serial.begin(115200);
    
    // 1. Initialize Display Subsystem
    // (This now handles Wire.begin internally)
    displayMgr.begin();
    
    Serial.println("Starting Network Task...");
    // 2. Launch Core 0
    xTaskCreatePinnedToCore(
        networkLoop, "NetworkTask", 10000, NULL, 1, &NetworkTaskHandle, 0
    );

    Serial.println("Starting Display Task...");
    // 3. Launch Core 1
    xTaskCreatePinnedToCore(
        displayLoop, "DisplayTask", 10000, NULL, 1, &DisplayTaskHandle, 1
    );

    // 4. FUTURE: Check Button Inputs
    // if (digitalRead(BUTTON_PIN) == LOW) {
    //    displayMgr.toggleMenu();
    // }
}

void loop() {

}