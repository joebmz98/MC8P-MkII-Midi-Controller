// =============================================================================
// hardware.h — pin map and the peripheral objects (OLED, DIN + USB MIDI)
//
// STM32F411 Black Pill. Everything that names a pin lives here, so a board
// revision only needs this file changed.
//
//   OLED (SSD1306 128x64, I2C1)  : SCL PB6, SDA PB7 (set in setup())
//   Pots (16-channel mux)        : SIG PA1, S0-S3 PA8 / PA9 / PA10 / PA15
//   DIN MIDI out (USART2 TX)     : PA2, 31,250 baud
//   W25Q64 SPI flash (SPI1)      : CS PA4, SCK PA5, MISO PA6, MOSI PA7
//   Buttons (to 3V3, pull-downs) : ASSIGN PB12, ENTER PB13, PREV PB14, NEXT PB15
// =============================================================================
#pragma once
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>
#include <MIDI.h>
#include <Adafruit_TinyUSB.h>
#include <Fonts/Picopixel.h>
#include <Wire.h>
#include <SPI.h>

// -- OLED SETUP --
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define SCREEN_ADDRESS 0x3C
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// -- MULTIPLEXER PINS
#define MUX_SIG_PIN PA1  // Analog input from MUX output
#define MUX_S0 PA8       // Select pin 0
#define MUX_S1 PA9       // Select pin 1
#define MUX_S2 PA10      // Select pin 2
#define MUX_S3 PA15      // Select pin 3

// -- MIDI PIN
#define MIDI_TX_PIN PA2  // MIDI output

// -- MIDI INSTANCE --
MIDI_CREATE_INSTANCE(HardwareSerial, Serial2, MIDI);  //

// USB MIDI via TinyUSB
Adafruit_USBD_MIDI usb_midi;
MIDI_CREATE_INSTANCE(Adafruit_USBD_MIDI, usb_midi, MIDI_USB);

// -- W25Q64 SPI FLASH PINS --
#define FLASH_CS_PIN PA4    // Chip Select
#define FLASH_SCK_PIN PA5   // SPI Clock
#define FLASH_MISO_PIN PA6  // Master In Out (D0)
#define FLASH_MOSI_PIN PA7  // Master Out In (D1)

// -- BUTTON CONFIG
const uint8_t NUM_BUTTONS = 4;
const int BUTTON_PINS[NUM_BUTTONS] = { PB12, PB13, PB14, PB15 };
const char* BUTTON_NAMES[NUM_BUTTONS] = { "ASSIGN", "ENTER", "PREV", "NEXT" };
