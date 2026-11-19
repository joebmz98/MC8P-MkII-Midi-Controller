// =============================================================================
// editor_sysex.h — USB SysEx protocol for the MC8P web editor
//
// The full message list is at the top of MC8P_MIDICONTROLLER_FIRMWARE_STM.ino.
// onUsbSysEx() is registered with MIDI_USB in setup(); replies go out with
// editorSendSysEx(). Edits apply live and are only kept after a SAVE.
// =============================================================================
#pragma once
#include "hardware.h"
#include "state.h"
#include "prototypes.h"
#include "storage.h"

// WEB EDITOR (USB SysEx) - see the protocol notes at the top of the .ino
const uint8_t SYSEX_MFR = 0x7D;   // non-commercial / educational ID
const uint8_t SYSEX_ID1 = 0x4D;   // 'M'
const uint8_t SYSEX_ID2 = 0x38;   // '8'
const uint8_t SYSEX_PROTO = 1;    // bump if the message layout changes

enum EditorCmd : uint8_t {
  ED_GET_INFO = 0x01,
  ED_GET_POT = 0x02,
  ED_SET_POT = 0x03,
  ED_GET_GLOBAL = 0x04,
  ED_SET_GLOBAL = 0x05,
  ED_GET_STATUS = 0x06,
  ED_SAVE = 0x07,
  ED_REVERT = 0x08,
  ED_FACTORY_RESET = 0x09,
  ED_RPL_INFO = 0x41,
  ED_RPL_POT = 0x42,
  ED_RPL_GLOBAL = 0x44,
  ED_RPL_STATUS = 0x46
};

const uint8_t ED_GLOBAL_LEN = 6 + MAX_CUSTOM_SCENES;  // data bytes in SET_GLOBAL / 44 reply

// *********************** //
// WEB EDITOR (USB SysEx)
// *********************** //

// Fletcher-16 style checksum, cut to 14 bits so it fits two SysEx bytes.
// The web editor computes the same sums over its copy of the settings.
void sumByte(uint16_t& a, uint16_t& b, uint8_t v) {
  a = (a + v) % 255;
  b = (b + a) % 255;
}

uint16_t midiConfigSum() {
  uint16_t a = 0, b = 0;
  for (int i = 0; i < N_POTS; i++) {
    sumByte(a, b, messageCount[i]);
    for (int j = 0; j < messageCount[i] && j < MAX_MESSAGES_PER_POT; j++) {
      sumByte(a, b, potMessages[i][j].channel);
      sumByte(a, b, potMessages[i][j].cc);
      sumByte(a, b, (uint8_t)potMessages[i][j].curve);
      sumByte(a, b, potMessages[i][j].minValue);
      sumByte(a, b, potMessages[i][j].maxValue);
    }
  }
  return ((b << 8) | a) & 0x3FFF;
}

uint16_t globalConfigSum() {
  uint16_t a = 0, b = 0;
  sumByte(a, b, (uint8_t)editPotSelection);
  sumByte(a, b, sceneCC);
  sumByte(a, b, (uint8_t)sceneMode);
  sumByte(a, b, customSceneLength);
  for (int i = 0; i < MAX_CUSTOM_SCENES; i++) {
    sumByte(a, b, (uint8_t)customSceneOrder[i]);
  }
  sumByte(a, b, displayTheme);
  sumByte(a, b, displayInverted ? 1 : 0);
  return ((b << 8) | a) & 0x3FFF;
}

// Write a complete F0...F7 message to USB. If the USB buffer is full, keep
// servicing USB until it drains (the host normally reads within a few ms).
void editorSendSysEx(const uint8_t* m, size_t n) {
  if (!TinyUSBDevice.mounted()) return;
  size_t i = 0;
  uint32_t t0 = millis();
  while (i < n) {
    i += usb_midi.write(m + i, n - i);
    if (i < n) {
      TinyUSBDevice.task();
      if (millis() - t0 > 100) break;  // host isn't reading - the editor asks again
    }
  }
}

uint8_t editorHeader(uint8_t* m, uint8_t reply) {
  m[0] = 0xF0;
  m[1] = SYSEX_MFR;
  m[2] = SYSEX_ID1;
  m[3] = SYSEX_ID2;
  m[4] = reply;
  return 5;
}

void editorSendInfo() {
  uint8_t m[24];
  uint8_t n = editorHeader(m, ED_RPL_INFO);
  m[n++] = SYSEX_PROTO;
  m[n++] = N_POTS;
  m[n++] = MAX_MESSAGES_PER_POT;
  m[n++] = MAX_CUSTOM_SCENES;
  for (const char* s = FIRMWARE_VERSION; *s && n < sizeof(m) - 1; s++) m[n++] = (uint8_t)*s & 0x7F;
  m[n++] = 0xF7;
  editorSendSysEx(m, n);
}

void editorSendPot(uint8_t pot) {
  uint8_t m[8 + MAX_MESSAGES_PER_POT * 5];
  uint8_t n = editorHeader(m, ED_RPL_POT);
  m[n++] = pot;
  m[n++] = messageCount[pot];
  for (int j = 0; j < messageCount[pot] && j < MAX_MESSAGES_PER_POT; j++) {
    m[n++] = potMessages[pot][j].channel & 0x7F;
    m[n++] = potMessages[pot][j].cc & 0x7F;
    m[n++] = (uint8_t)potMessages[pot][j].curve & 0x7F;
    m[n++] = potMessages[pot][j].minValue & 0x7F;
    m[n++] = potMessages[pot][j].maxValue & 0x7F;
  }
  m[n++] = 0xF7;
  editorSendSysEx(m, n);
}

void editorSendGlobal() {
  uint8_t m[8 + ED_GLOBAL_LEN];
  uint8_t n = editorHeader(m, ED_RPL_GLOBAL);
  m[n++] = (uint8_t)editPotSelection;
  m[n++] = sceneCC & 0x7F;
  m[n++] = (uint8_t)sceneMode;
  m[n++] = customSceneLength;
  for (int i = 0; i < MAX_CUSTOM_SCENES; i++) m[n++] = (uint8_t)customSceneOrder[i] & 0x7F;
  m[n++] = displayTheme;
  m[n++] = displayInverted ? 1 : 0;
  m[n++] = 0xF7;
  editorSendSysEx(m, n);
}

// 46 <flags> <screen> <midi sum lo hi> <global sum lo hi> <pot 1-8 raw, lo hi each> <scene> <scene pos>
// flags: 1 = pot assignments unsaved, 2 = scene/display settings unsaved,
//        4 = temporary override held, 8 = panel is in a menu
void editorSendStatus() {
  uint8_t m[40];
  uint8_t n = editorHeader(m, ED_RPL_STATUS);
  uint16_t ms = midiConfigSum();
  uint16_t gs = globalConfigSum();
  m[n++] = (ms != savedMidiSum ? 0x01 : 0) | (gs != savedGlobalSum ? 0x02 : 0) | (tempOverrideActive ? 0x04 : 0) | (currentScreen != MAIN_SCREEN ? 0x08 : 0);
  m[n++] = (uint8_t)currentScreen;
  m[n++] = ms & 0x7F;
  m[n++] = (ms >> 7) & 0x7F;
  m[n++] = gs & 0x7F;
  m[n++] = (gs >> 7) & 0x7F;
  for (int i = 0; i < N_POTS; i++) {
    uint16_t v = constrain(potState[i], 0, 1023);
    m[n++] = v & 0x7F;
    m[n++] = (v >> 7) & 0x7F;
  }
  m[n++] = (uint8_t)getCurrentSceneMidiValue() & 0x7F;
  m[n++] = currentSceneValue & 0x7F;
  m[n++] = 0xF7;
  editorSendSysEx(m, n);
}

// Keep the panel's menus pointing at things that still exist after an edit
void editorClampSelections() {
  if (selectedPot >= N_POTS) selectedPot = 0;
  uint8_t count = messageCount[selectedPot];
  if (count < 1) count = 1;
  if (selectedMessage >= count) selectedMessage = count - 1;
  uint8_t maxScroll = (count > MAX_VISIBLE_MESSAGES) ? count - MAX_VISIBLE_MESSAGES : 0;
  if (scrollOffset > maxScroll) scrollOffset = maxScroll;
  if (selectedMessage < scrollOffset) scrollOffset = selectedMessage;

  if (sceneMode == CUSTOM_MODE && currentSceneValue >= customSceneLength) currentSceneValue = 0;
  if (sceneMode == STEP_MODE && currentSceneValue > 98) currentSceneValue = 98;
  if (customSceneEditPosition >= customSceneLength) customSceneEditPosition = customSceneLength - 1;
  if (lastCustomSceneEditPosition >= customSceneLength) lastCustomSceneEditPosition = customSceneLength - 1;
}

// SET_POT data: <count> {<ch> <cc> <curve> <min> <max>} x count
// The whole pot is rejected if anything is out of range.
void editorApplyPot(uint8_t pot, const uint8_t* p, unsigned len) {
  if (len < 1) return;
  uint8_t count = p[0];
  if (count < 1 || count > MAX_MESSAGES_PER_POT || len < 1u + count * 5u) return;
  for (int j = 0; j < count; j++) {
    const uint8_t* q = p + 1 + j * 5;
    if (q[0] > 16 || q[1] > 127 || q[2] > CURVE_ILG || q[3] > 127 || q[4] > 127) return;
  }
  for (int j = 0; j < count; j++) {
    const uint8_t* q = p + 1 + j * 5;
    MidiMessageParams& msg = potMessages[pot][j];
    msg.channel = q[0];
    msg.cc = q[1];
    msg.curve = (CurveType)q[2];
    msg.minValue = q[3];
    msg.maxValue = q[4];
    if (msg.minValue > msg.maxValue) msg.maxValue = msg.minValue;  // same rule as the ASSIGN menu
    // Value follows the knob with the new range/curve, as if it had just been turned
    msg.value = mapMidiValueWithParams(potState[pot], msg.minValue, msg.maxValue, msg.curve);
  }
  messageCount[pot] = count;
  resetCatchUp(pot);
  editorClampSelections();
}

// SET_GLOBAL data: <edit pot> <scene cc> <scene mode> <custom len> <order x12> <theme> <inverted>
void editorApplyGlobal(const uint8_t* p) {
  if (p[0] >= N_POTS || p[1] > 127 || p[2] > 1 || p[3] < 1 || p[3] > MAX_CUSTOM_SCENES) return;
  for (int i = 0; i < MAX_CUSTOM_SCENES; i++) {
    if (p[4 + i] > 98) return;
  }
  if (p[4 + MAX_CUSTOM_SCENES] >= NUM_THEMES || p[5 + MAX_CUSTOM_SCENES] > 1) return;

  editPotSelection = p[0];
  sceneCC = p[1];
  SceneNavigationMode newMode = p[2] ? CUSTOM_MODE : STEP_MODE;
  if (newMode != sceneMode) {
    sceneMode = newMode;
    currentSceneValue = 0;
    sceneSelectedParameter = 0;
  }
  customSceneLength = p[3];
  for (int i = 0; i < MAX_CUSTOM_SCENES; i++) customSceneOrder[i] = p[4 + i];
  displayTheme = p[4 + MAX_CUSTOM_SCENES];
  displayInverted = p[5 + MAX_CUSTOM_SCENES] != 0;
  editorClampSelections();
}

// Called by the MIDI library for every complete SysEx message from USB
void onUsbSysEx(byte* data, unsigned size) {
  const uint8_t* d = data;
  unsigned n = size;
  if (n && d[0] == 0xF0) { d++; n--; }
  if (n && d[n - 1] == 0xF7) n--;
  if (n < 4 || d[0] != SYSEX_MFR || d[1] != SYSEX_ID1 || d[2] != SYSEX_ID2) return;

  const uint8_t* p = d + 4;
  unsigned len = n - 4;

  switch (d[3]) {
    case ED_GET_INFO:
      editorSendInfo();
      break;
    case ED_GET_POT:
      if (len >= 1 && p[0] < N_POTS) editorSendPot(p[0]);
      break;
    case ED_SET_POT:
      if (len >= 2 && p[0] < N_POTS) {
        editorApplyPot(p[0], p + 1, len - 1);
        editorSendPot(p[0]);
      }
      break;
    case ED_GET_GLOBAL:
      editorSendGlobal();
      break;
    case ED_SET_GLOBAL:
      if (len >= ED_GLOBAL_LEN) editorApplyGlobal(p);
      editorSendGlobal();
      break;
    case ED_GET_STATUS:
      editorSendStatus();
      break;
    case ED_SAVE:
      if (len >= 3 && p[0] == 'S' && p[1] == 'A' && p[2] == 'V') {
        Serial.println("Web editor: saving settings");
        saveMidiSettingsToEEPROM();
        saveGlobalSettingsToEEPROM();
      }
      editorSendStatus();
      break;
    case ED_REVERT:
      Serial.println("Web editor: reloading saved settings");
      loadMidiSettingsFromEEPROM();
      loadGlobalSettingsFromEEPROM();
      for (int i = 0; i < N_POTS; i++) resetCatchUp(i);
      editorClampSelections();
      editorSendStatus();
      break;
    case ED_FACTORY_RESET:
      if (len >= 3 && p[0] == 'D' && p[1] == 'E' && p[2] == 'F') {
        Serial.println("Web editor: factory reset");
        factoryReset();
        editorClampSelections();
      }
      editorSendStatus();
      break;
  }
}
