#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// ============================================================================
// LOGGING CONFIGURATION
// ============================================================================

// Set compile-time log level (affects which logs are compiled into binary)
// Options: LOG_LEVEL_NONE, LOG_LEVEL_ERROR, LOG_LEVEL_WARN, 
//          LOG_LEVEL_INFO, LOG_LEVEL_DEBUG, LOG_LEVEL_VERBOSE
#define LOG_LEVEL LOG_LEVEL_DEBUG

// Enable/disable features
#define LOG_ENABLE_COLORS true       // ANSI color codes in terminal
#define LOG_ENABLE_TIMESTAMPS true   // Include millisecond timestamps
#define LOG_BUFFER_SIZE 256          // Buffer for formatting log messages
#define LOG_MAX_TAG_LENGTH 8         // Max characters for module tags
#define LOG_ERROR_BUFFER_SIZE 50     // Circular buffer for recent errors

// Uncomment to run logger self-tests on boot
// #define DEBUG_TESTS

// E1.31 Settings
#define DEFAULT_IP 192,168,0,100
#define DEFAULT_MAC { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED }
#define E131_PORT 5568
#define DEFAULT_DHCP_STATUS false
#define MIN_UNIVERSE 1
#define DEFAULT_UNIVERSE 129
#define MAX_UNIVERSE 63999
#define MIN_NUM_LEDS 0
#define DEFAULT_NUM_LEDS 10
#define MAX_NUM_LEDS 50
#define HEADER_SIZE 126
#define DMX_STARTCODE 0
#define DMX_MAX_CHANNELS 512
#define E131_UNIVERSE_OFFSET 113
#define E131_LENGTH_OFFSET 123
#define E131_HEADER_SIZE 126
#define E131_MAX_PACKET_SIZE E131_HEADER_SIZE + DMX_MAX_CHANNELS

// Ethernet SPI Pins (ESP32-S3)
#define ETH_MISO 13
#define ETH_MOSI 11
#define ETH_SCK  12
#define ETH_CS   10

// HC-12 Radio Pins
#define HC12_RX 18
#define HC12_TX 17
#define HC12_SET 16
#define HC12_BAUD 9600

// Display
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_ADDR 0x3C
#define DISPLAY_SDA 8
#define DISPLAY_SCL 9
#define STATUS_SCREEN_LENGTH_MS 3000

// LED Settings
#define NEOPIXEL 48
#define RGB_STRIP 0 // TODO: Assign actual pin of rgb strip
#define CHAN_PER_LED 3

// Buttons
#define BTN_UP    20
#define BTN_DOWN  21
#define BTN_LEFT  22
#define BTN_RIGHT 23
#define BTN_SEL   24

#endif
