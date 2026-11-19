// =============================================================================
// storage.h — saving / loading settings to SPI flash, and factory reset
//
// Two records, each in its own 4 KB sector:
//   FLASH_MIDI_START_ADDR   (0)    : MidiSettings   - every pot's messages
//   FLASH_GLOBAL_START_ADDR (4096) : GlobalSettings - scene, edit pot, display
// Each starts with FLASH_SIGNATURE and a version; a mismatch loads defaults.
// Bump SETTINGS_VERSION / GLOBAL_SETTINGS_VERSION when a struct changes.
// =============================================================================
#pragma once
#include "hardware.h"
#include "state.h"
#include "prototypes.h"
#include "spi_flash.h"

// -- W25Q64 SPI FLASH CONFIGURATION --
#define FLASH_SIGNATURE 0x55AA        // Signature to verify saved data
#define FLASH_MIDI_START_ADDR 0       // Start address for MIDI settings
#define FLASH_GLOBAL_START_ADDR 4096  // Start address for global settings (4KB alignment)

// EEPROM STRUCTURES
// STRUCTURE TO SAVE MIDI CONFIGURATION TO EEPROM
struct MidiSettings {
  uint16_t signature;
  uint8_t version;
  byte messageCount[N_POTS];
  struct SavedMidiMessage {
    byte channel;
    byte cc;
    CurveType curve;
    byte minValue;
    byte maxValue;
    uint16_t value;
  } potMessages[N_POTS][MAX_MESSAGES_PER_POT];
};

// STRUCTURE FOR GLOBAL SETTINGS
struct GlobalSettings {
  uint16_t signature;
  uint8_t version;
  bool displayInverted;
  int editPot;
  int sceneCC;
  SceneNavigationMode sceneMode;
  int customSceneLength;
  int customSceneOrder[MAX_CUSTOM_SCENES];
  uint8_t displayTheme;
};

// -- EEPROM VERSION
#define SETTINGS_VERSION 4
#define GLOBAL_SETTINGS_VERSION 4

// *********************** //
// SAVE MIDI SETTINGS TO SPI FLASH
// *********************** //
void saveMidiSettingsToEEPROM() {
  MidiSettings settings;
  settings.signature = FLASH_SIGNATURE;
  settings.version = SETTINGS_VERSION;

  for (int i = 0; i < N_POTS; i++) {
    settings.messageCount[i] = messageCount[i];
    for (int j = 0; j < MAX_MESSAGES_PER_POT; j++) {
      settings.potMessages[i][j].channel = potMessages[i][j].channel;
      settings.potMessages[i][j].cc = potMessages[i][j].cc;
      settings.potMessages[i][j].curve = potMessages[i][j].curve;
      settings.potMessages[i][j].minValue = potMessages[i][j].minValue;
      settings.potMessages[i][j].maxValue = potMessages[i][j].maxValue;
      settings.potMessages[i][j].value = potMessages[i][j].value;
    }
  }

  // Write to SPI Flash
  writeStructToFlash(FLASH_MIDI_START_ADDR, settings);
  savedMidiSum = midiConfigSum();

  Serial.println("MIDI settings saved to SPI Flash");
}

// *********************** //
// LOAD MIDI SETTINGS FROM SPI FLASH
// *********************** //
void loadMidiSettingsFromEEPROM() {
  MidiSettings settings;
  readStructFromFlash(FLASH_MIDI_START_ADDR, settings);

  if (settings.signature == FLASH_SIGNATURE && settings.version == SETTINGS_VERSION) {
    // Valid settings found - load them
    for (int i = 0; i < N_POTS; i++) {
      if (settings.messageCount[i] >= 1 && settings.messageCount[i] <= MAX_MESSAGES_PER_POT) {
        messageCount[i] = settings.messageCount[i];
      } else {
        messageCount[i] = 1;
        Serial.print("Pot ");
        Serial.print(i);
        Serial.println(" had invalid messageCount, reset to 1");
      }

      for (int j = 0; j < MAX_MESSAGES_PER_POT; j++) {
        if (settings.potMessages[i][j].channel >= 0 && settings.potMessages[i][j].channel <= 16) {
          potMessages[i][j].channel = settings.potMessages[i][j].channel;
        } else {
          potMessages[i][j].channel = (j == 0) ? i : 0;
        }

        if (settings.potMessages[i][j].cc >= 0 && settings.potMessages[i][j].cc <= 127) {
          potMessages[i][j].cc = settings.potMessages[i][j].cc;
        } else {
          potMessages[i][j].cc = (j == 0) ? 7 : 1;
        }

        if (settings.potMessages[i][j].curve >= 0 && settings.potMessages[i][j].curve <= 3) {
          potMessages[i][j].curve = settings.potMessages[i][j].curve;
        } else {
          potMessages[i][j].curve = CURVE_NOR;
        }

        if (settings.potMessages[i][j].minValue >= 0 && settings.potMessages[i][j].minValue <= 127) {
          potMessages[i][j].minValue = settings.potMessages[i][j].minValue;
        } else {
          potMessages[i][j].minValue = 0;
        }

        if (settings.potMessages[i][j].maxValue >= 0 && settings.potMessages[i][j].maxValue <= 127) {
          potMessages[i][j].maxValue = settings.potMessages[i][j].maxValue;
        } else {
          potMessages[i][j].maxValue = 127;
        }

        if (potMessages[i][j].minValue > potMessages[i][j].maxValue) {
          potMessages[i][j].maxValue = potMessages[i][j].minValue;
        }

        potMessages[i][j].value = settings.potMessages[i][j].value;
      }
    }
    Serial.println("MIDI settings loaded from SPI Flash");
  } else {
    Serial.println("No valid MIDI settings found, using defaults");
    // Set defaults for all pots
    for (int i = 0; i < N_POTS; i++) {
      messageCount[i] = 1;
      potMessages[i][0].channel = i;
      potMessages[i][0].cc = 7;
      potMessages[i][0].curve = CURVE_NOR;
      potMessages[i][0].minValue = 0;
      potMessages[i][0].maxValue = 127;

      for (int j = 1; j < MAX_MESSAGES_PER_POT; j++) {
        potMessages[i][j].channel = 0;
        potMessages[i][j].cc = 0;
        potMessages[i][j].curve = CURVE_NOR;
        potMessages[i][j].minValue = 0;
        potMessages[i][j].maxValue = 127;
      }
    }
  }
  savedMidiSum = midiConfigSum();
}

// *********************** //
// EEPROM FUNCTIONS
// *********************** //
// *********************** //
// FACTORY RESET FUNCTION
// *********************** //
void factoryReset() {
  Serial.println("=== FACTORY RESET INITIATED ===");

  // Reset all pots to default configuration
  for (int i = 0; i < N_POTS; i++) {
    messageCount[i] = 1;
    potMessages[i][0].channel = i;                  // Channel 1-8 (0-7 in code)
    potMessages[i][0].cc = 7;                       // CC 7
    potMessages[i][0].curve = CURVE_NOR;            // Normal direction
    potMessages[i][0].minValue = 0;                 // Min value 0
    potMessages[i][0].maxValue = 127;               // Max value 127
    potMessages[i][0].value = currentMidiValue[i];  // Current pot position

    // Clear any additional messages
    for (int j = 1; j < MAX_MESSAGES_PER_POT; j++) {
      potMessages[i][j].channel = 0;
      potMessages[i][j].cc = 0;
      potMessages[i][j].curve = CURVE_NOR;
      potMessages[i][j].minValue = 0;
      potMessages[i][j].maxValue = 127;
      potMessages[i][j].value = 0;
    }
  }

  // Reset display settings
  displayInverted = true;
  displayTheme = 0;

  // Reset edit pot selection
  editPotSelection = 7;

  // Reset scene settings
  sceneCC = 82;
  sceneMode = STEP_MODE;
  customSceneLength = 6;  // Set to 6 instead of 12
  currentSceneValue = 0;

  // Reset custom scene order
  for (int i = 0; i < MAX_CUSTOM_SCENES; i++) {
    customSceneOrder[i] = i;
  }

  // CRITICAL FIX: Force a complete SPI Flash write with verified data
  // First, save MIDI settings
  saveMidiSettingsToEEPROM();

  // Then save global settings
  saveGlobalSettingsToEEPROM();

  // Verify the write by reading back immediately
  Serial.println("Verifying factory reset save...");
  MidiSettings verifySettings;
  readStructFromFlash(FLASH_MIDI_START_ADDR, verifySettings);

  if (verifySettings.signature == FLASH_SIGNATURE) {
    Serial.println("MIDI settings verified successfully");
    for (int i = 0; i < N_POTS; i++) {
      Serial.print("Pot ");
      Serial.print(i);
      Serial.print(" messageCount: ");
      Serial.println(verifySettings.messageCount[i]);
    }
  } else {
    Serial.println("ERROR: MIDI settings verification failed - retrying");
    // Retry save if verification failed
    saveMidiSettingsToEEPROM();

    // Verify again
    readStructFromFlash(FLASH_MIDI_START_ADDR, verifySettings);
    if (verifySettings.signature == FLASH_SIGNATURE) {
      Serial.println("MIDI settings verified successfully on retry");
    } else {
      Serial.println("CRITICAL ERROR: MIDI settings verification failed on retry!");
    }
  }

  // Also verify global settings
  GlobalSettings verifyGlobalSettings;
  readStructFromFlash(FLASH_GLOBAL_START_ADDR, verifyGlobalSettings);

  if (verifyGlobalSettings.signature == FLASH_SIGNATURE) {
    Serial.println("Global settings verified successfully");
  } else {
    Serial.println("ERROR: Global settings verification failed");
  }

  // Reset navigation states
  selectedPot = 0;
  selectedMessage = 0;
  scrollOffset = 0;
  assignEditMode = ASSIGN_POT_SELECT;
  assignEditingMode = false;

  Serial.println("Factory reset completed!");
  Serial.println("All pots reset to: Channel 1-8, CC7, Range 0-127, Normal direction");
  Serial.println("Scene settings reset: CC=82, Mode=STEP, Custom Length=6");
  Serial.println("=== FACTORY RESET COMPLETE ===");

  // Visual feedback
  for (int i = 0; i < 3; i++) {
    display.invertDisplay(true);
    delay(100);
    display.invertDisplay(false);
    delay(100);
  }
}

// *********************** //
// SAVE GLOBAL SETTINGS TO SPI FLASH
// *********************** //
void saveGlobalSettingsToEEPROM() {
  GlobalSettings settings;
  settings.signature = FLASH_SIGNATURE;
  settings.version = GLOBAL_SETTINGS_VERSION;
  settings.displayInverted = displayInverted;
  settings.editPot = editPotSelection;
  settings.sceneCC = sceneCC;
  settings.sceneMode = sceneMode;
  settings.customSceneLength = customSceneLength;
  settings.displayTheme = displayTheme;

  for (int i = 0; i < MAX_CUSTOM_SCENES; i++) {
    settings.customSceneOrder[i] = customSceneOrder[i];
  }

  writeStructToFlash(FLASH_GLOBAL_START_ADDR, settings);
  savedGlobalSum = globalConfigSum();
  Serial.println("Global settings saved to SPI Flash");
}


// *********************** //
// LOAD GLOBAL SETTINGS FROM SPI FLASH
// *********************** //
void loadGlobalSettingsFromEEPROM() {
  GlobalSettings settings;
  readStructFromFlash(FLASH_GLOBAL_START_ADDR, settings);

  if (settings.signature == FLASH_SIGNATURE && settings.version == GLOBAL_SETTINGS_VERSION) {
    displayInverted = settings.displayInverted;
    editPotSelection = settings.editPot;
    sceneCC = settings.sceneCC;
    sceneMode = settings.sceneMode;
    customSceneLength = settings.customSceneLength;
    displayTheme = settings.displayTheme;
    if (displayTheme >= NUM_THEMES) displayTheme = 0;

    for (int i = 0; i < MAX_CUSTOM_SCENES; i++) {
      customSceneOrder[i] = settings.customSceneOrder[i];
    }

    // Validate values
    if (sceneMode != STEP_MODE && sceneMode != CUSTOM_MODE) {
      sceneMode = STEP_MODE;
    }
    if (customSceneLength < 1 || customSceneLength > MAX_CUSTOM_SCENES) {
      customSceneLength = 6;
    }
    if (sceneCC < 0 || sceneCC > 127) {
      sceneCC = 82;
    }
    if (editPotSelection < 0 || editPotSelection >= N_POTS) {
      editPotSelection = 7;
    }

    Serial.println("Global settings loaded from SPI Flash");
  } else {
    Serial.println("No valid global settings found, using defaults");
    displayInverted = false;
    editPotSelection = 7;
    sceneCC = 82;
    sceneMode = STEP_MODE;
    customSceneLength = 6;
    displayTheme = 0;

    for (int i = 0; i < MAX_CUSTOM_SCENES; i++) {
      customSceneOrder[i] = i;
    }
  }
  savedGlobalSum = globalConfigSum();
}
