#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// E1.31 Settings
#define E131_PORT 5568
#define UNIVERSE 129
#define NUM_LEDS 10
#define PACKET_BUFFER_SIZE 638
#define HEADER_SIZE 126
#define DMX_STARTCODE 0

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

// Onboard LED
#define NEOPIXEL 48

#endif