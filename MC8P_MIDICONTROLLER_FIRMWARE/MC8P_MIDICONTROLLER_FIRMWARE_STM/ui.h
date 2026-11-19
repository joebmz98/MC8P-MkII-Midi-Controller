// =============================================================================
// ui.h — draws whichever screen is current (called from loop())
// =============================================================================
#pragma once
#include "hardware.h"
#include "state.h"
#include "prototypes.h"
#include "display_main.h"
#include "display_assign.h"
#include "display_menus.h"

void drawCurrentScreen() {
  if (currentScreen == MAIN_SCREEN) {
    drawMainScreen();
  } else if (currentScreen == MENU_SCREEN) {
    drawMenuScreen();
  } else if (currentScreen == ASSIGN_SCREEN) {
    drawAssignScreen();
  } else if (currentScreen == SETTINGS_SCREEN) {
    drawSettingsMenuScreen();
  } else if (currentScreen == ASSIGN_SETTINGS_SCREEN) {
    drawAssignSettingsScreen();
  } else if (currentScreen == SCENE_SCREEN) {
    drawSceneScreen();
  } else if (currentScreen == DISPLAY_SETTINGS_SCREEN) {
    drawDisplaySettings();
  } else if (currentScreen == ABOUT_SCREEN) {
    drawAboutScreen();
  } else if (currentScreen == RESET_SCREEN) {
    drawConfirmResetPopup();
  } else if (currentScreen == CONFIRM_ASSIGN_SAVE_SCREEN) {
    drawConfirmAssignSavePopup();
  } else if (currentScreen == CONFIRM_RESET_SCREEN) {
    drawConfirmResetPopup();
  }
}
