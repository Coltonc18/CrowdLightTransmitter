#include "ConfigManager.h"
#include "Logger.h"
#include <WiFi.h> // Include for IPAddress conversion

#define STORAGE_NAMESPACE "crowdlight"
#define CONFIG_KEY "device_config"

ConfigManager::ConfigManager() {}

void ConfigManager::begin() {
    // 1. Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        // Partition was truncated and needs to be erased
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // 2. Open NVS namespace
    ret = nvs_open(STORAGE_NAMESPACE, NVS_READWRITE, &_nvsHandle);
    if (ret != ESP_OK) {
        LOG_ERROR_TAG("CONFIG", "Error opening NVS handle: %s", esp_err_to_name(ret));
    } else {
        LOG_DEBUG_TAG("CONFIG", "NVS namespace opened successfully");
    }
}

void ConfigManager::loadConfig(DeviceConfig& config) {
    size_t required_size = sizeof(DeviceConfig);
    esp_err_t ret = nvs_get_blob(_nvsHandle, CONFIG_KEY, &config, &required_size);

    if (ret == ESP_ERR_NVS_NOT_FOUND) {
        LOG_WARN_TAG("CONFIG", "Config not found, loading defaults");
        // --- Set Default Values (Only happens on first boot) ---
        config.universe = DEFAULT_UNIVERSE; 
        config.useDhcp = DEFAULT_DHCP_STATUS;
        IPAddress defaultIP(DEFAULT_IP);
        config.ipAddress = defaultIP;
        // ----------------------------------------------------
        saveConfig(config); // Save the defaults immediately
    } else if (ret != ESP_OK) {
        LOG_ERROR_TAG("CONFIG", "Error reading config blob: %s", esp_err_to_name(ret));
    } else {
        LOG_INFO_TAG("CONFIG", "Config loaded - Universe: %d, LEDs: %d", config.universe, config.numLeds);
    }
}

void ConfigManager::saveConfig(const DeviceConfig& config) {
    esp_err_t ret = nvs_set_blob(_nvsHandle, CONFIG_KEY, &config, sizeof(DeviceConfig));
    if (ret != ESP_OK) {
        LOG_ERROR_TAG("CONFIG", "Error saving config blob: %s", esp_err_to_name(ret));
        return;
    }
    ret = nvs_commit(_nvsHandle);
    if (ret != ESP_OK) {
        LOG_ERROR_TAG("CONFIG", "Error committing NVS: %s", esp_err_to_name(ret));
    } else {
        LOG_INFO_TAG("CONFIG", "Config saved - Universe: %d, LEDs: %d", config.universe, config.numLeds);
    }
}
