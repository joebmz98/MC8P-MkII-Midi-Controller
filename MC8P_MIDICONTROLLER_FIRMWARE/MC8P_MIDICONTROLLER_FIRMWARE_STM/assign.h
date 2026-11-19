// =============================================================================
// assign.h — add / remove a MIDI message on the selected pot (ASSIGN screen)
// =============================================================================
#pragma once
#include "hardware.h"
#include "state.h"
#include "prototypes.h"

// *********************** //
// ADD MIDI CONTROL FNC
// *********************** //
void addMidiControl() {
  if (messageCount[selectedPot] < MAX_MESSAGES_PER_POT) {
    potMessages[selectedPot][messageCount[selectedPot]].channel = 0;
    potMessages[selectedPot][messageCount[selectedPot]].cc = 1;
    potMessages[selectedPot][messageCount[selectedPot]].curve = CURVE_NOR;
    potMessages[selectedPot][messageCount[selectedPot]].minValue = 0;
    potMessages[selectedPot][messageCount[selectedPot]].maxValue = 127;
    potMessages[selectedPot][messageCount[selectedPot]].value = potState[selectedPot];
    messageCount[selectedPot]++;

    // Update scroll offset to show the newly added message
    if (messageCount[selectedPot] > MAX_VISIBLE_MESSAGES) {
      scrollOffset = messageCount[selectedPot] - MAX_VISIBLE_MESSAGES;
    } else {
      scrollOffset = 0;
    }

    // Select the newly added message
    selectedMessage = messageCount[selectedPot] - 1;

    Serial.print("Added new MIDI message to pot ");
    Serial.print(selectedPot);
    Serial.print(". Total messages: ");
    Serial.println(messageCount[selectedPot]);
  } else {
    Serial.println("Maximum messages reached for this pot");
  }
}
// *********************** //
// REMOVE MIDI CONTROL FNC
// *********************** //
void removeMidiControl() {
  if (messageCount[selectedPot] > 1) {  // DON'T REMOVE LAST MESSAGE
    // SHIFT ALL MESSAGES DOWN ONE BASED ON WHICH WAS REMOVED
    for (int i = selectedMessage; i < messageCount[selectedPot] - 1; i++) {
      potMessages[selectedPot][i] = potMessages[selectedPot][i + 1];
    }
    messageCount[selectedPot]--;

    // Adjust selected message if needed
    if (selectedMessage >= messageCount[selectedPot]) {
      selectedMessage = messageCount[selectedPot] - 1;
    }

    // Update scroll offset
    if (selectedMessage < scrollOffset) {
      scrollOffset = selectedMessage;
    } else if (selectedMessage >= scrollOffset + MAX_VISIBLE_MESSAGES) {
      scrollOffset = selectedMessage - MAX_VISIBLE_MESSAGES + 1;
    }

    // Ensure scroll offset stays within valid range
    scrollOffset = constrain(scrollOffset, 0, max(0, messageCount[selectedPot] - MAX_VISIBLE_MESSAGES));

    Serial.print("Removed MIDI message from pot ");
    Serial.print(selectedPot);
    Serial.print(". Total messages: ");
    Serial.println(messageCount[selectedPot]);
  } else {
    Serial.println("Cannot remove the last MIDI message");
  }
}
