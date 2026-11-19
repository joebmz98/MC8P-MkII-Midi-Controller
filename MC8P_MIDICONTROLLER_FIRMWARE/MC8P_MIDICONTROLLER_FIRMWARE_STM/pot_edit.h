// =============================================================================
// pot_edit.h — editing values in the menus with the edit pot (pot 8 by default)
//
// The edit pot is chosen in SETTINGS > ASSIGN (editPotSelection).
// =============================================================================
#pragma once
#include "hardware.h"
#include "state.h"
#include "prototypes.h"
#include "pots.h"

// *********************** //
// HANDLE POT PARAMETER EDIT
// *********************** //
void handleParameterEditWithPot() {
  // Handle scene screen editing
  if (currentScreen == SCENE_SCREEN && sceneEditMode != SCENE_EDIT_NONE) {
    // Read the selected edit potentiometer via multiplexer
    int potValue = readMuxChannel(editPotSelection);

    // Only process if value has changed significantly (1% threshold = 10 units)
    if (abs(potValue - lastPot8Value) >= POT_THRESHOLD) {
      lastPot8Value = potValue;
      lastPot8ChangeTime = millis();

      bool valueChanged = false;

      // Handle different edit modes
      if (sceneEditMode == SCENE_EDIT_CC) {
        // Map 0-1023 to 0-127 for CC value
        int newCC = map(potValue, 0, 1023, 0, 127);
        if (newCC != sceneCC) {
          sceneCC = newCC;
          valueChanged = true;
          Serial.print("Scene CC adjusted to: ");
          Serial.println(sceneCC);
        }
      } else if (sceneEditMode == SCENE_EDIT_MODE) {
        // Map 0-1023 to 0-1 (STEP=0, CUSTOM=1)
        int newMode = (potValue > 512) ? 1 : 0;
        SceneNavigationMode newSceneMode = (newMode == 0) ? STEP_MODE : CUSTOM_MODE;
        if (newSceneMode != sceneMode) {
          sceneMode = newSceneMode;
          valueChanged = true;
          Serial.print("Scene mode changed to: ");
          Serial.println(sceneMode == STEP_MODE ? "STEP" : "CUSTOM");
          // Reset selected parameter when switching modes
          sceneSelectedParameter = 0;
        }
      } else if (sceneEditMode == SCENE_EDIT_CUSTOM_LENGTH) {
        // Map 0-1023 to 1-12 for custom length
        int newLength = map(potValue, 0, 1023, 1, MAX_CUSTOM_SCENES);
        if (newLength != customSceneLength) {
          customSceneLength = newLength;
          // Ensure currentSceneValue and edit position are within new bounds
          if (currentSceneValue >= customSceneLength) {
            currentSceneValue = customSceneLength - 1;
          }
          if (customSceneEditPosition >= customSceneLength) {
            customSceneEditPosition = customSceneLength - 1;
          }
          valueChanged = true;
          Serial.print("Custom scene length adjusted to: ");
          Serial.println(customSceneLength);
        }
      } else if (sceneEditMode == SCENE_EDIT_CUSTOM_VALUE) {
        // Map 0-1023 to 0-98 for scene value
        int newValue = map(potValue, 0, 1023, 0, 98);
        if (newValue != customSceneOrder[customSceneEditPosition]) {
          customSceneOrder[customSceneEditPosition] = newValue;
          valueChanged = true;
          Serial.print("Custom scene at position ");
          Serial.print(customSceneEditPosition + 1);
          Serial.print(" adjusted to: ");
          Serial.println(customSceneOrder[customSceneEditPosition] + 1);
        }
      }

      // Force screen refresh if value changed
      if (valueChanged) {
        drawSceneScreen();
      }
    }
    return;
  }

  // Only process assign screen editing
  if (currentScreen != ASSIGN_SCREEN || !assignEditingMode) {
    return;
  }

  // Read the selected edit potentiometer via multiplexer
  int potValue = readMuxChannel(editPotSelection);

  // Only process if value has changed significantly (1% threshold = 10 units)
  if (abs(potValue - lastPot8Value) >= POT_THRESHOLD) {
    lastPot8Value = potValue;
    lastPot8ChangeTime = millis();

    // Map to appropriate range based on what we're editing
    bool valueChanged = false;

    switch (assignEditMode) {
      case ASSIGN_EDIT_CHANNEL:
        {
          // Map 0-1023 to 0-16 (channels 1-16 + OMNI)
          int newChannel = map(potValue, 0, 1023, 0, 16);
          if (newChannel != potMessages[selectedPot][selectedMessage].channel) {
            potMessages[selectedPot][selectedMessage].channel = newChannel;
            valueChanged = true;
            Serial.print("Channel set to: ");
            if (potMessages[selectedPot][selectedMessage].channel == 16) {
              Serial.println("OMNI");
            } else {
              Serial.println(potMessages[selectedPot][selectedMessage].channel + 1);
            }
          }
          break;
        }

      case ASSIGN_EDIT_CC:
        {
          // Map 0-1023 to 0-127
          int newCC = map(potValue, 0, 1023, 0, 127);
          if (newCC != potMessages[selectedPot][selectedMessage].cc) {
            potMessages[selectedPot][selectedMessage].cc = newCC;
            valueChanged = true;
            Serial.print("CC set to: ");
            Serial.println(potMessages[selectedPot][selectedMessage].cc);
          }
          break;
        }

      case ASSIGN_EDIT_MIN:
        {
          // Map 0-1023 to 0-127
          int newMin = map(potValue, 0, 1023, 0, 127);
          if (newMin != potMessages[selectedPot][selectedMessage].minValue) {
            potMessages[selectedPot][selectedMessage].minValue = newMin;
            // Ensure max value is at least min value
            if (potMessages[selectedPot][selectedMessage].maxValue < newMin) {
              potMessages[selectedPot][selectedMessage].maxValue = newMin;
            }
            valueChanged = true;
            Serial.print("Min value set to: ");
            Serial.println(potMessages[selectedPot][selectedMessage].minValue);
          }
          break;
        }

      case ASSIGN_EDIT_MAX:
        {
          // Map 0-1023 to 0-127
          int newMax = map(potValue, 0, 1023, 0, 127);
          if (newMax != potMessages[selectedPot][selectedMessage].maxValue) {
            potMessages[selectedPot][selectedMessage].maxValue = newMax;
            // Ensure min value is at most max value
            if (potMessages[selectedPot][selectedMessage].minValue > newMax) {
              potMessages[selectedPot][selectedMessage].minValue = newMax;
            }
            valueChanged = true;
            Serial.print("Max value set to: ");
            Serial.println(potMessages[selectedPot][selectedMessage].maxValue);
          }
          break;
        }

      case ASSIGN_EDIT_INVERT:
        {
          // Map 0-1023 to 0-3 (four curve types)
          int newCurve = map(potValue, 0, 1023, 0, 3);
          CurveType newCurveType = (CurveType)newCurve;
          if (newCurveType != potMessages[selectedPot][selectedMessage].curve) {
            potMessages[selectedPot][selectedMessage].curve = newCurveType;
            valueChanged = true;
            Serial.print("Curve set to: ");
            switch (potMessages[selectedPot][selectedMessage].curve) {
              case CURVE_NOR: Serial.println("NOR"); break;
              case CURVE_INV: Serial.println("INV"); break;
              case CURVE_LOG: Serial.println("LOG"); break;
              case CURVE_ILG: Serial.println("ILG"); break;
            }
          }
          break;
        }

      default:
        break;
    }

    // Force screen redraw if value changed
    if (valueChanged) {
      drawAssignScreen();
    }
  }
}
