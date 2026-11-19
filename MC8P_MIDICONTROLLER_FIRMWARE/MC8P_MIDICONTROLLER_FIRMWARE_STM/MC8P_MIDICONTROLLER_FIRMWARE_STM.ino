// ********************* //
// MC8P MkII             //
// MIDI Controller       //
// firmware version 3.2  //
// hardware version: 2.0 //
// powered by STM32 BP   //
// ********************* //
// controller by         //
// .axs instruments      //
// ********************* //
// Description: 8-potentiometer MIDI controller with
//              programmable CC assignments and
//              multi-message per pot support.
// ********************* //

// ********************* //
// WEB EDITOR (USB SysEx)
//
// Every pot assignment, the scene settings and the display settings can be
// read and changed over USB MIDI SysEx by the MC8P web editor (Chrome / Edge,
// Web MIDI). Messages:
//   F0 7D 4D 38 <cmd> <data...> F7     (7D = non-commercial ID, "M8")
// Host -> MC8P:
//   01                       GET_INFO    -> 41 <proto> <pots> <max msgs> <max scenes> <fw ascii...>
//   02 <pot>                 GET_POT     -> 42 <pot> <count> {<ch> <cc> <curve> <min> <max>} x count
//   03 <pot> <count> {...}   SET_POT     same layout as the 42 reply; applies, replies 42
//   04                       GET_GLOBAL  -> 44 <edit pot> <scene cc> <scene mode> <custom len>
//                                           <custom order x12> <theme> <inverted>
//   05 <same as 44 data>     SET_GLOBAL  applies, replies 44
//   06                       GET_STATUS  -> 46 (see editorSendStatus())
//   07 53 41 56              SAVE ("SAV") writes everything to SPI flash, replies 46
//   08                       REVERT      reloads the last saved settings, replies 46
//   09 44 45 46              FACTORY RESET ("DEF"), replies 46
// ch: 0-15 = MIDI channel 1-16, 16 = OMNI. curve: 0 NOR, 1 INV, 2 LOG, 3 ILG.
// min/max: 0-127 with min <= max. scene mode: 0 STEP, 1 CUSTOM. custom order
// values are scenes 0-98 (shown +1). theme: 0 MkII, 1 MkI, 2 YX.
// Edits take effect straight away but, like the ASSIGN menu, are only kept
// after a SAVE (or a save from the panel). GET_STATUS reports whether
// anything is unsaved.
// ********************* //

// ********************* //
// LICENSE & COPYRIGHT
//
// This project is released under the MIT License.
// Copyright (c) 2024 .axs instruments
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
// ********************* //

// -- LIBRARIES --
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>
#include <MIDI.h>
#include <Adafruit_TinyUSB.h>
#include <Fonts/Picopixel.h>

// -- FLASH --
#include <Wire.h>
#include <SPI.h>

// -- FIRMWARE MODULES (see README.md in this folder) --
#include "hardware.h"         // pins, OLED, DIN + USB MIDI objects
#include "state.h"            // shared types and globals
#include "prototypes.h"       // every function, so modules can call each other
#include "display_assets.h"   // layout tables and bitmaps
#include "curves.h"           // pot -> CC curves
#include "spi_flash.h"        // W25Q64 driver
#include "storage.h"          // save / load / factory reset
#include "midi_out.h"         // DIN + USB CC output
#include "editor_sysex.h"     // web editor SysEx protocol
#include "assign.h"           // add / remove messages
#include "scenes.h"           // OP-XY scene navigation
#include "temp_override.h"    // ENTER-held override and catch-up
#include "pots.h"             // mux reads and pot -> MIDI
#include "pot_edit.h"         // menu editing with the edit pot
#include "buttons.h"          // button handling
#include "display_main.h"     // main screen themes
#include "display_assign.h"   // ASSIGN screen
#include "display_menus.h"    // menus and popups
#include "startup.h"          // boot sequence
#include "ui.h"               // screen dispatcher


// ***************************  //
// SETUP
// ***************************  //
void setup() {

  // Set ADC resolution to 10-bit (0-1023) for compatibility
  analogReadResolution(10);

  // Configure multiplexer control pins
  pinMode(MUX_S0, OUTPUT);
  pinMode(MUX_S1, OUTPUT);
  pinMode(MUX_S2, OUTPUT);
  pinMode(MUX_S3, OUTPUT);
  digitalWrite(MUX_S0, LOW);
  digitalWrite(MUX_S1, LOW);
  digitalWrite(MUX_S2, LOW);
  digitalWrite(MUX_S3, LOW);

  // Configure analog input pin for multiplexer
  pinMode(MUX_SIG_PIN, INPUT_ANALOG);

  for (int i = 0; i < N_POTS; i++) {
    setMuxChannel(i);
    delay(1);                 // Let the channel settle
    analogRead(MUX_SIG_PIN);  // Discard first reading
  }
  delay(10);  // Final settle delay

  // -- USB MIDI Setup --
  //TinyUSB_Device_Init(0);
  TinyUSBDevice.setManufacturerDescriptor(".axs instruments");
  TinyUSBDevice.setProductDescriptor("MC8P MkII");
  TinyUSBDevice.begin(0);
  MIDI_USB.begin(MIDI_CHANNEL_OMNI);
  MIDI_USB.turnThruOff();
  MIDI_USB.setHandleSystemExclusive(onUsbSysEx);  // web editor

  // BEGIN SERIAL MONITORING
  // SERIAL2 IS FOR MIDI OUTPUT
  Serial.begin(9600);
  Serial2.begin(31250);

  // -- INIT MIDI
  Serial2.setTx(MIDI_TX_PIN);  // SET MIDI OUTPUT PIN to PA15
  MIDI.begin();                // -- INIT TRS MIDI


  uint32_t start = millis();
  while (!TinyUSBDevice.mounted()) {
    TinyUSBDevice.task();
    delay(1);
    // Timeout after 5 seconds so device still works without USB
    if (millis() - start > 5000) break;
  }

  // POWER LED
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);

  // Initialize SPI Flash
  initFlash();

  // Test Flash connection by reading ID
  uint32_t flashID = readFlashID();
  if (flashID != 0xFFFFFF) {
    Serial.print("W25Q64 SPI Flash found! ID: 0x");
    Serial.println(flashID, HEX);
  } else {
    Serial.println("W25Q64 SPI Flash NOT found! Check wiring.");
  }


  // Initialize I2C for the OLED
  Wire.setSCL(PB6);
  Wire.setSDA(PB7);
  Wire.begin();

  // INITIALISE
  initController();
}


// *************************** //
// MAIN
// - BUTTONS BEING READ
// - POTENTIOMETERS BEING READ
// - MIDI OUTPUT
// - SCREEN UPDATES
// *************************** //
void loop() {

  // USB MIDI
  TinyUSBDevice.task();

  // USB MIDI IN - web editor SysEx. The MIDI library parses one byte per
  // read(), so drain everything that has arrived (capped so a flood can't
  // stall the pots).
  for (uint16_t n = 0; n < 512 && usb_midi.available(); n++) {
    MIDI_USB.read();
  }

  // BUTTON HANDLING
  readButtons();

  // Read all potentiometers via multiplexer
  readAllPotentiometers();

  // Read pots and send their MIDI messages
  processPotentiometers();

  // Handle parameter editing with potentiometer 8
  handleParameterEditWithPot();

  // Draw the current screen
  drawCurrentScreen();
}

