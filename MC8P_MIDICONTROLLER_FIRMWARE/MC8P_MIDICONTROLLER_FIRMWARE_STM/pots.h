// =============================================================================
// pots.h — reads the 8 pots through the multiplexer and sends their messages
//
// processPotentiometers() runs every loop(): a pot that moved by POT_THRESHOLD
// sends all its messages (OMNI = all 16 channels). While the temporary
// override is on the values are sent but not stored; after it, catch-up
// scales from the stored value (see temp_override.h).
// =============================================================================
#pragma once
#include "hardware.h"
#include "state.h"
#include "prototypes.h"
#include "curves.h"
#include "midi_out.h"

// *****************************
// MULTIPLEXER FUNCTIONS
// *****************************

// Set multiplexer channel (0-15)
void setMuxChannel(uint8_t channel) {
  // Channel selection is 4 bits (S0-S3)
  digitalWrite(MUX_S0, (channel >> 0) & 1);
  digitalWrite(MUX_S1, (channel >> 1) & 1);
  digitalWrite(MUX_S2, (channel >> 2) & 1);
  digitalWrite(MUX_S3, (channel >> 3) & 1);

  // Small delay for signal stabilization
  delayMicroseconds(10);
}

// Read analog value from specific multiplexer channel
int readMuxChannel(uint8_t channel) {
  setMuxChannel(channel);
  // Allow time for the analog signal to settle
  delayMicroseconds(50);
  return analogRead(MUX_SIG_PIN);
}

// Read all potentiometers from multiplexer
void readAllPotentiometers() {
  for (int i = 0; i < N_POTS; i++) {
    potReading[i] = readMuxChannel(i);
  }
}

// *****************************
// PROCESS POTENTIOMETERS (called from loop())
// *****************************
void processPotentiometers() {
  // Process all potentiometers
  for (int i = 0; i < N_POTS; i++) {
    // Simple raw reading - no smoothing
    potState[i] = potReading[i];
    currentMidiValue[i] = map(potState[i], 0, 1023, 0, 127);

    // Check if pot has changed by more than threshold (1%)
    int potVar = abs(potState[i] - potPState[i]);

    if (potVar >= POT_THRESHOLD) {
      // Only send MIDI messages when in main screen
      if (currentScreen == MAIN_SCREEN) {
        // Check if temporary override is active
        if (tempOverrideActive) {
          // Store temporary values but don't update the original messages
          tempMidiValues[i] = potState[i];
          // Send all messages for this pot with temporary values
          for (int j = 0; j < messageCount[i]; j++) {
            int finalValue = mapMidiValueWithParams(tempMidiValues[i],
                                                    potMessages[i][j].minValue,
                                                    potMessages[i][j].maxValue,
                                                    potMessages[i][j].curve);
            if (potMessages[i][j].channel == 16) {  // OMNI
              for (byte ch = 0; ch < 16; ch++) {
                sendControlChange(potMessages[i][j].cc, finalValue, ch + 1);
              }
            } else {
              sendControlChange(potMessages[i][j].cc, finalValue, potMessages[i][j].channel + 1);
            }
          }
        } else {
          // Normal operation
          if (catchUpActive[i]) {
            // Calculate how far the pot has moved from the catch-up start position
            int potMovement = potState[i] - catchUpStartPotPos[i];

            if (abs(potMovement) > 20) {
              int scaledMidiValue;
              if (potMovement > 0) {
                int targetRange = 127 - catchUpStartValue[i];
                int totalMovement = 1023 - catchUpStartPotPos[i];
                if (totalMovement > 0) {
                  scaledMidiValue = catchUpStartValue[i] + (targetRange * potMovement / totalMovement);
                } else {
                  scaledMidiValue = 127;
                }
              } else {
                int targetRange = catchUpStartValue[i];
                int totalMovement = catchUpStartPotPos[i];
                int movementProgress = -potMovement;
                if (totalMovement > 0) {
                  scaledMidiValue = catchUpStartValue[i] - (targetRange * movementProgress / totalMovement);
                } else {
                  scaledMidiValue = 0;
                }
              }
              scaledMidiValue = constrain(scaledMidiValue, 0, 127);

              for (int j = 0; j < messageCount[i]; j++) {
                int finalValue = mapMidiValueWithParams(scaledMidiValue,
                                                        potMessages[i][j].minValue,
                                                        potMessages[i][j].maxValue,
                                                        potMessages[i][j].curve);
                potMessages[i][j].value = finalValue;

                if (potMessages[i][j].channel == 16) {
                  for (byte ch = 0; ch < 16; ch++) {
                    sendControlChange(potMessages[i][j].cc, finalValue, ch + 1);
                  }
                } else {
                  sendControlChange(potMessages[i][j].cc, finalValue, potMessages[i][j].channel + 1);
                }
              }

              int expectedMidiForPot = map(potState[i], 0, 1023, 0, 127);
              int storedMidiValue = potMessages[i][0].value;

              if (abs(expectedMidiForPot - storedMidiValue) < 10) {
                catchUpActive[i] = false;
              }
            } else {
              for (int j = 0; j < messageCount[i]; j++) {
                int finalValue = mapMidiValueWithParams(potMessages[i][j].value,
                                                        potMessages[i][j].minValue,
                                                        potMessages[i][j].maxValue,
                                                        potMessages[i][j].curve);

                if (potMessages[i][j].channel == 16) {
                  for (byte ch = 0; ch < 16; ch++) {
                    sendControlChange(potMessages[i][j].cc, finalValue, ch + 1);
                  }
                } else {
                  sendControlChange(potMessages[i][j].cc, finalValue, potMessages[i][j].channel + 1);
                }
              }
            }
          } else {
            // Normal operation - update and send all messages
            for (int j = 0; j < messageCount[i]; j++) {
              int finalValue = mapMidiValueWithParams(potState[i],
                                                      potMessages[i][j].minValue,
                                                      potMessages[i][j].maxValue,
                                                      potMessages[i][j].curve);
              potMessages[i][j].value = finalValue;

              if (potMessages[i][j].channel == 16) {
                for (byte ch = 0; ch < 16; ch++) {
                  sendControlChange(potMessages[i][j].cc, finalValue, ch + 1);
                }
              } else {
                sendControlChange(potMessages[i][j].cc, finalValue, potMessages[i][j].channel + 1);
              }
            }
          }
        }
      }

      // Debug output
      Serial.print("Pot ");
      Serial.print(i);
      Serial.print(": ");
      Serial.print(currentMidiValue[i]);
      if (catchUpActive[i]) {
        Serial.print(" (Catch-up active)");
      }
      Serial.print(" -> ");
      if (messageCount[i] > 0) {
        Serial.print(potMessages[i][0].value);
      } else {
        Serial.print("0");
      }
      Serial.println("/127");

      // Update previous state
      potPState[i] = potState[i];
    }

    // Check if pot has returned to normal position and disable catch-up
    if (!tempOverrideActive && catchUpActive[i]) {
      int expectedMidiForPot = map(potState[i], 0, 1023, 0, 127);
      int storedMidiValue = potMessages[i][0].value;

      if (abs(expectedMidiForPot - storedMidiValue) < 10) {
        catchUpActive[i] = false;
      }
    }
  }
}
