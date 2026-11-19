// =============================================================================
// midi_out.h — sends a CC to DIN MIDI and (when mounted) USB MIDI
// =============================================================================
#pragma once
#include "hardware.h"
#include "state.h"
#include "prototypes.h"

// *********************** //
// MIDI FUNCTIONS
// *********************** //
// *********************** //
// MIDI HANDLING           //
// *********************** //
void sendControlChange(byte cc, byte value, byte channel) {
  // Send to DIN MIDI (Serial2)
  MIDI.sendControlChange(cc, value, channel);

  // Send to USB MIDI
  if (TinyUSBDevice.mounted()) {
    MIDI_USB.sendControlChange(cc, value, channel);
  }
}
