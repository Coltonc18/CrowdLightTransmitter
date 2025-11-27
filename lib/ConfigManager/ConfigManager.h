#pragma once
#include <Arduino.h>
#include <nvs_flash.h>      // Include the ESP-IDF NVS library
#include <nvs.h>            // Include the ESP-IDF NVS library
#include "ConfigData.h"
#include "Config.h"

class ConfigManager {
public:
    ConfigManager();
    void begin();

    // Load data from NVS into the shared config structure
    void loadConfig(DeviceConfig& config);
    
    // Save the current config structure to NVS
    void saveConfig(const DeviceConfig& config);

private:
    // NVS handle for the partition we use
    nvs_handle_t _nvsHandle;
};