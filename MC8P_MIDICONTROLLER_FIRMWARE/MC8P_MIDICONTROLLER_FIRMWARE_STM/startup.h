// =============================================================================
// startup.h — boot sequence: splash + loading bar, load settings, first pot read
//
// initController() is the last step of setup(): loads both settings records,
// reads the pots with extra settling, sends the initial values and the
// scene, then shows the main screen.
// =============================================================================
#pragma once
#include "hardware.h"
#include "state.h"
#include "prototypes.h"
#include "storage.h"
#include "pots.h"
#include "scenes.h"
#include "display_main.h"

// *********************** //
// INITIALISE
// *********************** //
void initController() {

  // INIT BUTTONS
  for (int i = 0; i < NUM_BUTTONS; i++) {
    pinMode(BUTTON_PINS[i], INPUT_PULLDOWN);
  }

  // INIT I2C for OLED (using hardware I2C1 on PB6/PB7)
  Wire.setSCL(PB6);
  Wire.setSDA(PB7);
  Wire.begin();

  // -- LOAD SETTINGS
  Serial.println("=== BOOTUP DEBUG ===");
  Serial.println("Loading global settings...");
  loadGlobalSettingsFromEEPROM();

  // INIT OLED
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;)
      ;
  }

  // FIRST: Initialize display with default settings (normal orientation)
  display.clearDisplay();
  display.fillRect(0, 0, 128, 64, 1);          // WHITE BACKGROUND
  display.drawRoundRect(1, 1, 126, 62, 3, 0);  // OUTER BOX
  display.drawRoundRect(17, 38, 94, 8, 3, 0);  // LOADING BAR BORDER
  display.fillRoundRect(19, 40, 4, 4, 1, 0);   // LOADING BAR INNER

  // TEXT
  display.setTextColor(0);
  display.setTextSize(3);
  display.setTextWrap(false);
  display.setFont(&Picopixel);
  display.setCursor(29, 26);
  display.print("M C 8 P");
  display.setTextSize(1);
  display.setCursor(58, 55);
  display.print("v");
  display.print(FIRMWARE_VERSION);
  display.display();

  // INITIALIZE TEMP OVERRIDE VALUES TO 0
  for (int i = 0; i < N_POTS; i++) {
    originalMidiValues[i] = 0;
    tempMidiValues[i] = 0;
    postOverridePotPositions[i] = 0;
    catchUpStartValue[i] = 0;
    catchUpStartPotPos[i] = 0;
    catchUpActive[i] = false;
  }

  // LOADING BAR UPDATE
  display.fillRoundRect(19, 40, 30, 4, 1, 0);
  display.display();
  delay(250);


  // Load MIDI settings from EEPROM
  Serial.println("Loading MIDI settings...");
  loadMidiSettingsFromEEPROM();

  // DEBUG: Print loaded MIDI values for each pot
  Serial.println("=== LOADED MIDI VALUES ===");
  for (int i = 0; i < N_POTS; i++) {
    Serial.print("Pot ");
    Serial.print(i);
    Serial.print(": ");
    Serial.print("messageCount=");
    Serial.print(messageCount[i]);
    if (messageCount[i] > 0) {
      Serial.print(", channel=");
      if (potMessages[i][0].channel == 16) {
        Serial.print("OMNI");
      } else {
        Serial.print(potMessages[i][0].channel + 1);
      }
      Serial.print(", cc=");
      Serial.print(potMessages[i][0].cc);
      Serial.print(", curve=");
      switch (potMessages[i][0].curve) {
        case CURVE_NOR: Serial.print("NOR"); break;
        case CURVE_INV: Serial.print("INV"); break;
        case CURVE_LOG: Serial.print("LOG"); break;
        case CURVE_ILG: Serial.print("ILG"); break;
      }
      Serial.print(", min=");
      Serial.print(potMessages[i][0].minValue);
      Serial.print(", max=");
      Serial.print(potMessages[i][0].maxValue);
      Serial.print(", value=");
      Serial.println(potMessages[i][0].value);
    } else {
      Serial.println(" (no messages)");
    }
  }
  Serial.println("=== END DEBUG ===");

  // LOADING BAR UPDATE
  display.fillRoundRect(19, 40, 60, 4, 1, 0);
  display.display();
  delay(250);

  // INIT MIDI MESSAGES - WILL USE DEFAULT IF NONE ARE SAVED
  bool settingsLoaded = false;
  for (int i = 0; i < N_POTS; i++) {
    if (messageCount[i] > 0) {
      settingsLoaded = true;
      break;
    }
  }

  if (!settingsLoaded) {
    Serial.println("No valid settings loaded, using defaults");
    for (int i = 0; i < N_POTS; i++) {
      messageCount[i] = 1;
      potMessages[i][0].channel = potChannels[i];
      potMessages[i][0].cc = potCCs[i];
      potMessages[i][0].curve = CURVE_NOR;
      potMessages[i][0].minValue = 0;
      potMessages[i][0].maxValue = 127;
      // Don't set value here - it will be set from actual pot reading
      // potMessages[i][0].value = 0;  // REMOVED - was causing issues

      Serial.print("Default for Pot ");
      Serial.print(i);
      Serial.print(": channel=");
      Serial.print(potMessages[i][0].channel);
      Serial.print(", cc=");
      Serial.print(potMessages[i][0].cc);
      Serial.println();
    }
  }

  // ============================================
  // FIX: Initialize multiplexer with proper settling
  // ============================================

  // First, set all mux control pins to known state
  digitalWrite(MUX_S0, LOW);
  digitalWrite(MUX_S1, LOW);
  digitalWrite(MUX_S2, LOW);
  digitalWrite(MUX_S3, LOW);
  delay(10);  // Allow mux to settle

  // "Warm up" the multiplexer - cycle through all channels to stabilize
  Serial.println("=== WARMING UP MULTIPLEXER ===");
  for (int i = 0; i < N_POTS; i++) {
    setMuxChannel(i);
    delay(2);                             // Longer settle time for warm-up
    int dummy = analogRead(MUX_SIG_PIN);  // Discard first reading
    Serial.print("Warm-up channel ");
    Serial.print(i);
    Serial.print(": ");
    Serial.println(dummy);
  }
  delay(20);  // Final settle delay
  Serial.println("=== MULTIPLEXER WARM-UP COMPLETE ===");

  // Read initial pot positions with longer delays for stability
  Serial.println("=== READING INITIAL POT POSITIONS ===");
  for (int i = 0; i < N_POTS; i++) {
    // Read the channel with proper settling
    int rawReading = 0;

    // For problematic channels (1-3), take multiple readings and average
    if (i >= 1 && i <= 3) {
      // Read 3 times and average for problematic channels
      int sum = 0;
      for (int readCount = 0; readCount < 3; readCount++) {
        setMuxChannel(i);
        delayMicroseconds(200);  // Longer settling for problematic channels
        sum += analogRead(MUX_SIG_PIN);
        delay(2);  // Small delay between reads
      }
      rawReading = sum / 3;
      Serial.print("Pot ");
      Serial.print(i);
      Serial.print(" (averaged over 3 reads): ");
    } else {
      // Normal read for other channels
      setMuxChannel(i);
      delayMicroseconds(200);  // Increased from 50 to 200 microseconds
      rawReading = analogRead(MUX_SIG_PIN);
      Serial.print("Pot ");
      Serial.print(i);
      Serial.print(": ");
    }

    potReading[i] = rawReading;
    potState[i] = potReading[i];
    potPState[i] = potState[i];  // Initialize previous state to current
    currentMidiValue[i] = map(potState[i], 0, 1023, 0, 127);

    Serial.print("raw=");
    Serial.print(potReading[i]);
    Serial.print(", MIDI=");
    Serial.print(currentMidiValue[i]);

    if (messageCount[i] > 0) {
      int finalValue = mapMidiValueWithParams(currentMidiValue[i],
                                              potMessages[i][0].minValue,
                                              potMessages[i][0].maxValue,
                                              potMessages[i][0].curve);
      potMessages[i][0].value = finalValue;
      Serial.print(" (stored value set to ");
      Serial.print(finalValue);
      Serial.print(")");
    }
    Serial.println();

    // Small delay between channel reads
    delay(5);
  }
  Serial.println("=== END POT READING ===");

  // LOADING BAR UPDATE
  display.fillRoundRect(19, 40, 90, 4, 1, 0);
  display.display();
  delay(1000);

  // Send initial MIDI values
  Serial.println("=== SENDING INITIAL MIDI VALUES ===");
  for (int i = 0; i < N_POTS; i++) {
    for (int j = 0; j < messageCount[i]; j++) {
      int finalValue = mapMidiValueWithParams(potMessages[i][j].value,
                                              potMessages[i][j].minValue,
                                              potMessages[i][j].maxValue,
                                              potMessages[i][j].curve);

      if (potMessages[i][j].channel == 16) {  // OMNI
        for (byte ch = 0; ch < 16; ch++) {
          sendControlChange(potMessages[i][j].cc, finalValue, ch + 1);
        }
        Serial.print("Sent MIDI: Pot ");
        Serial.print(i);
        Serial.print(", Message ");
        Serial.print(j);
        Serial.print(": Ch=OMNI, CC=");
        Serial.print(potMessages[i][j].cc);
        Serial.print(", Final Value=");
        Serial.print(finalValue);
        Serial.print(", Range=[");
        Serial.print(potMessages[i][j].minValue);
        Serial.print("-");
        Serial.print(potMessages[i][j].maxValue);
        Serial.print("], Curve=");
        switch (potMessages[i][j].curve) {
          case CURVE_NOR: Serial.print("NOR"); break;
          case CURVE_INV: Serial.print("INV"); break;
          case CURVE_LOG: Serial.print("LOG"); break;
          case CURVE_ILG: Serial.print("ILG"); break;
        }
        Serial.println();
      } else {
        sendControlChange(potMessages[i][j].cc, finalValue, potMessages[i][j].channel + 1);
        Serial.print("Sent MIDI: Pot ");
        Serial.print(i);
        Serial.print(", Message ");
        Serial.print(j);
        Serial.print(": Ch=");
        Serial.print(potMessages[i][j].channel + 1);
        Serial.print(", CC=");
        Serial.print(potMessages[i][j].cc);
        Serial.print(", Final Value=");
        Serial.print(finalValue);
        Serial.print(", Range=[");
        Serial.print(potMessages[i][j].minValue);
        Serial.print("-");
        Serial.print(potMessages[i][j].maxValue);
        Serial.print("], Curve=");
        switch (potMessages[i][j].curve) {
          case CURVE_NOR: Serial.print("NOR"); break;
          case CURVE_INV: Serial.print("INV"); break;
          case CURVE_LOG: Serial.print("LOG"); break;
          case CURVE_ILG: Serial.print("ILG"); break;
        }
        Serial.println();
      }
    }
  }

  // SCENES/PROJECT INIT
  // Validate sceneCC range (0-127)
  if (sceneCC < 0 || sceneCC > 127) {
    sceneCC = 82;  // Default to CC 82 if invalid
    Serial.println("Invalid sceneCC value detected, reset to 82");
  }

  // Validate sceneMode
  if (sceneMode != STEP_MODE && sceneMode != CUSTOM_MODE) {
    sceneMode = STEP_MODE;  // Default to STEP mode if invalid
    Serial.println("Invalid sceneMode detected, reset to STEP_MODE");
  }

  // Validate customSceneLength (1-12)
  if (customSceneLength < 1 || customSceneLength > MAX_CUSTOM_SCENES) {
    customSceneLength = 6;  // Default to 6 if invalid
    Serial.println("Invalid customSceneLength detected, reset to 6");
  }

  // Validate customSceneOrder array values
  bool validOrder = true;
  for (int i = 0; i < customSceneLength; i++) {
    if (customSceneOrder[i] < 0 || customSceneOrder[i] > 98) {
      validOrder = false;
      break;
    }
  }
  if (!validOrder) {
    Serial.println("Invalid customSceneOrder detected, resetting to default sequence");
    for (int i = 0; i < MAX_CUSTOM_SCENES; i++) {
      customSceneOrder[i] = i;
    }
  }

  // Validate currentSceneValue range (0-98 for 99 scenes, or within customSceneLength for custom mode)
  if (sceneMode == STEP_MODE) {
    if (currentSceneValue < 0 || currentSceneValue > 98) {
      currentSceneValue = 0;  // Default to scene 1 if invalid
      Serial.println("Invalid currentSceneValue detected, reset to 0 (Scene 1)");
    }
  } else {  // CUSTOM_MODE
    if (currentSceneValue < 0 || currentSceneValue >= customSceneLength) {
      currentSceneValue = 0;  // Default to first custom scene if invalid
      Serial.println("Invalid currentSceneValue for custom mode, reset to 0");
    }
  }

  // Send initial scene value to sync with OP-XY using the loaded sceneCC
  int initialSceneValue;
  if (sceneMode == STEP_MODE) {
    initialSceneValue = currentSceneValue;
  } else {
    initialSceneValue = customSceneOrder[currentSceneValue];
  }
  sendControlChange(sceneCC, initialSceneValue, SCENE_CHANNEL);
  Serial.print("Scene initialized with CC: ");
  Serial.print(sceneCC);
  Serial.print(", Mode: ");
  Serial.print(sceneMode == STEP_MODE ? "STEP" : "CUSTOM");
  Serial.print(", Value: ");
  Serial.print(initialSceneValue);
  Serial.print(" (Scene ");
  Serial.print(initialSceneValue + 1);
  Serial.println(")");

  if (sceneMode == CUSTOM_MODE) {
    Serial.print("Custom scene length: ");
    Serial.print(customSceneLength);
    Serial.print(", Sequence: ");
    for (int i = 0; i < customSceneLength; i++) {
      Serial.print(customSceneOrder[i] + 1);
      if (i < customSceneLength - 1) Serial.print(", ");
    }
    Serial.println();
  }

  display.display();
  delay(500);

  // -- TRANSITION TO MAIN_SCREEN
  drawMainScreen();

  Serial.println("=== BOOTUP COMPLETE ===");
}
