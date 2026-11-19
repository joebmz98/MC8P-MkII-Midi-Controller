// =============================================================================
// temp_override.h — hold ENTER for temporary pot moves, then catch-up
//
// While ENTER is held, pot moves send CCs but are not stored. On release the
// stored values are re-sent and each pot enters catch-up: it scales from the
// stored value towards its own position, so nothing jumps.
// =============================================================================
#pragma once
#include "hardware.h"
#include "state.h"
#include "prototypes.h"
#include "midi_out.h"

// *********************** //
// TEMPORARY OVERRIDE FUNCTIONS
// *********************** //
// *********************** //
// ACTIVATE TEMPORARY OVERRIDE
// *********************** //
void activateTempOverride() {
  if (!tempOverrideActive) {
    tempOverrideActive = true;

    // Store original MIDI values for all pots
    for (int i = 0; i < N_POTS; i++) {
      originalMidiValues[i] = potMessages[i][0].value;  // Store the stored MIDI value, not current
      // Store the RAW pot position, not the mapped MIDI value
      tempMidiValues[i] = potState[i];  // Store raw pot value (0-1023)
    }

    Serial.println("Temporary MIDI override ACTIVATED");
    Serial.println("All pot changes are temporary until ENTER is released");

    // Store the original display inversion state before toggling
    originalDisplayInverted = displayInverted;

    // VISUAL FEEDBACK FOR ACTIVATING - toggle the display inversion
    // If display is normal, invert it; if it's inverted, make it normal
    if (displayInverted == false) {
      display.invertDisplay(!displayInverted);
    } else {
      display.invertDisplay(displayInverted);
    }

    delay(100);  // DEBOUNCE
  }
}

// *********************** //
// DEACTIVATE TEMPORARY OVERRIDE
// *********************** //
void deactivateTempOverride() {
  if (tempOverrideActive) {
    tempOverrideActive = false;

    // Send original MIDI values to restore state
    for (int i = 0; i < N_POTS; i++) {
      // Send all messages for this pot with original values
      for (int j = 0; j < messageCount[i]; j++) {
        // Check for OMNI channel
        if (potMessages[i][j].channel == 16) {  // OMNI
          for (byte ch = 0; ch < 16; ch++) {
            sendControlChange(potMessages[i][j].cc, potMessages[i][j].value, ch + 1);
          }
        } else {
          sendControlChange(potMessages[i][j].cc, potMessages[i][j].value, potMessages[i][j].channel + 1);
        }
      }

      // Store current potentiometer position for catch-up
      postOverridePotPositions[i] = potState[i];       // Store the raw pot value (0-1023)
      catchUpStartValue[i] = potMessages[i][0].value;  // Store the MIDI value we're returning to
      catchUpStartPotPos[i] = potState[i];             // Store the pot position where catch-up starts
      catchUpActive[i] = true;                         // Enable catch-up for this pot

      Serial.print("Pot ");
      Serial.print(i);
      Serial.print(": Catch-up enabled. Stored MIDI value=");
      Serial.print(potMessages[i][0].value);
      Serial.print(", Current pot position=");
      Serial.print(potState[i]);
      Serial.print(" (MIDI=");
      Serial.print(currentMidiValue[i]);
      Serial.println(")");
    }

    Serial.println("Temporary MIDI override DEACTIVATED");
    Serial.println("All MIDI values restored to original state");
    Serial.println("Catch-up mode activated for all pots");

    // Restore the original display inversion state
    if (displayInverted == true) {
      display.invertDisplay(!displayInverted);
    } else {
      display.invertDisplay(displayInverted);
    }

    // Clear only the temporary override values, NOT catch-up values
    for (int i = 0; i < N_POTS; i++) {
      originalMidiValues[i] = 0;  // Clear original values
      tempMidiValues[i] = 0;      // Clear temp values
    }

    // Force screen refresh to show stored values immediately
    if (currentScreen == MAIN_SCREEN) {
      drawMainScreen();
    }
  }
}

// *********************** //
// RESET CATCHUP
// *********************** //
void resetCatchUp(int potIndex) {
  catchUpActive[potIndex] = false;
  postOverridePotPositions[potIndex] = 0;
  catchUpStartValue[potIndex] = 0;
  catchUpStartPotPos[potIndex] = 0;
}
