// =============================================================================
// prototypes.h — every firmware function, declared once
//
// The Arduino IDE only writes prototypes for functions in the .ino itself.
// The modules call each other in both directions (buttons draw screens,
// screens read state the buttons set), so they are all declared here, before
// any module is compiled. Add new functions here, under their file.
// =============================================================================
#pragma once
#include "hardware.h"
#include "state.h"

// curves.h
int mapMidiValueWithParams(int rawValue, byte minVal, byte maxVal, CurveType curve);

// spi_flash.h
void initFlash();
uint32_t readFlashID();
void waitFlashReady();
void flashWriteEnable();
void flashEraseSector(uint32_t address);
void writeFlash(uint32_t address, const uint8_t* data, uint32_t length);
void readFlash(uint32_t address, uint8_t* data, uint32_t length);
template<typename T> void writeStructToFlash(uint32_t address, const T& data);
template<typename T> void readStructFromFlash(uint32_t address, T& data);

// storage.h
void saveMidiSettingsToEEPROM();
void loadMidiSettingsFromEEPROM();
void factoryReset();
void saveGlobalSettingsToEEPROM();
void loadGlobalSettingsFromEEPROM();

// midi_out.h
void sendControlChange(byte cc, byte value, byte channel);

// editor_sysex.h
void sumByte(uint16_t& a, uint16_t& b, uint8_t v);
uint16_t midiConfigSum();
uint16_t globalConfigSum();
void editorSendSysEx(const uint8_t* m, size_t n);
uint8_t editorHeader(uint8_t* m, uint8_t reply);
void editorSendInfo();
void editorSendPot(uint8_t pot);
void editorSendGlobal();
void editorSendStatus();
void editorClampSelections();
void editorApplyPot(uint8_t pot, const uint8_t* p, unsigned len);
void editorApplyGlobal(const uint8_t* p);
void onUsbSysEx(byte* data, unsigned size);

// assign.h
void addMidiControl();
void removeMidiControl();

// scenes.h
int getCurrentSceneMidiValue();
void setCurrentScene(int newValue);

// temp_override.h
void activateTempOverride();
void deactivateTempOverride();
void resetCatchUp(int potIndex);

// pots.h
void setMuxChannel(uint8_t channel);
int readMuxChannel(uint8_t channel);
void readAllPotentiometers();
void processPotentiometers();

// pot_edit.h
void handleParameterEditWithPot();

// buttons.h
void readButtons();

// display_main.h
void drawMainScreen();
void drawMainScreenMkII();
int getBarHeightFromMidi(int potIndex);
void drawMainScreenMkI();
void drawPotDisplay(int potIndex);
void drawMainScreenYX();
void triggerSceneOverlay();
int getBarHeightFromMidiYX(int potIndex);

// display_assign.h
void drawAssignScreen();

// display_menus.h
void drawMenuScreen();
void drawSceneScreen();
void drawConfirmAssignSavePopup();
void drawSettingsMenuScreen();
void drawAssignSettingsScreen();
void drawDisplaySettings();
void drawAboutScreen();
void drawConfirmResetPopup();

// startup.h
void initController();

// ui.h
void drawCurrentScreen();
