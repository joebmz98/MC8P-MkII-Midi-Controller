// =============================================================================
// display_menus.h — menu, scene, settings, about and confirm popups
// =============================================================================
#pragma once
#include "hardware.h"
#include "state.h"
#include "prototypes.h"
#include "display_assets.h"

// *********************** //
// DRAW NAVIGATION MENU    //
// *********************** //
void drawMenuScreen() {

  display.clearDisplay();  // CLEAR

  // Set colors based on the inversion variable
  int bgColor = displayInverted ? 0 : 1;   // Black if inverted, White if normal
  int txtColor = displayInverted ? 1 : 0;  // White if inverted, Black if normal

  display.setFont(&Picopixel);
  display.setTextSize(1);

  // DRAW UI
  display.fillRect(0, 0, 128, 64, bgColor);
  display.drawRoundRect(1, 1, 126, 62, 3, txtColor);

  display.setTextColor(txtColor);
  display.setCursor(6, 11);
  display.println("Menu");

  // MENU OPTIONS - Now 3 items
  display.setCursor(5, 25);
  display.println("- CC assign");
  display.setCursor(5, 35);
  display.println("- Scenes");
  display.setCursor(5, 45);
  display.println("- Settings");

  // Draw the bitmap indicator based on selection
  int indicatorY = 21 + (selectedMenuItem * 10);
  display.drawBitmap(55, indicatorY, image_ctrlMessageIndicator_bits, 5, 5, txtColor);

  display.display();
}

// *********************** //
// DRAW SCENE SCREEN       //
// *********************** //
void drawSceneScreen() {
  display.clearDisplay();

  // Set colors based on the inversion variable
  int bgColor = displayInverted ? 0 : 1;
  int txtColor = displayInverted ? 1 : 0;

  display.setFont(&Picopixel);
  display.setTextSize(1);

  // DRAW UI
  display.fillRect(0, 0, 128, 64, bgColor);
  display.drawRoundRect(1, 1, 126, 62, 3, txtColor);

  display.setTextColor(txtColor);
  display.setCursor(6, 11);
  display.println("Scenes");

  // Only show bitmap indicator for non-sequence parameters (CC, Mode, Length)
  // NOT when in edit mode
  if (sceneEditMode == SCENE_EDIT_NONE) {
    if (sceneMode == STEP_MODE) {
      // STEP mode only has CC and Mode
      if (sceneSelectedParameter < 2) {
        int indicatorX = 100;
        int indicatorY = 21 + (sceneSelectedParameter * 10);
        display.drawBitmap(indicatorX, indicatorY, image_ctrlMessageIndicator_bits, 5, 5, txtColor);
      }
    } else {
      // CUSTOM mode - show indicator for CC, Mode, Length (but not for sequence values)
      if (sceneSelectedParameter < 3) {
        int indicatorX = 100;
        int indicatorY = 21 + (sceneSelectedParameter * 10);
        display.drawBitmap(indicatorX, indicatorY, image_ctrlMessageIndicator_bits, 5, 5, txtColor);
      }
    }
  }

  // SCENE CC
  display.setCursor(5, 25);
  display.print("- Scene CC: ");

  // Calculate box width based on sceneCC value length
  int boxWidth;
  if (sceneCC >= 100) {
    boxWidth = 15;
  } else if (sceneCC >= 10) {
    boxWidth = 11;
  } else {
    boxWidth = 8;
  }

  int boxX = 66;
  int boxY = 18;
  int boxHeight = 11;

  // Draw the CC value with editing indicator
  if (sceneEditMode == SCENE_EDIT_CC) {
    if ((millis() / 300) % 2 == 0) {
      display.fillRoundRect(boxX, boxY, boxWidth, boxHeight, 1, txtColor);
      display.setTextColor(bgColor);
      display.setCursor(68, 25);
      display.print(sceneCC);
      display.setTextColor(txtColor);
    } else {
      display.drawRoundRect(boxX, boxY, boxWidth, boxHeight, 1, txtColor);
      display.setCursor(68, 25);
      display.print(sceneCC);
    }
  } else {
    display.setCursor(68, 25);
    display.print(sceneCC);
  }

  // SCENE MODE
  display.setCursor(5, 35);
  display.print("- Mode: ");

  int modeBoxX = 66;
  int modeBoxY = 28;
  int modeBoxWidth = 30;

  if (sceneEditMode == SCENE_EDIT_MODE) {
    if ((millis() / 300) % 2 == 0) {
      display.fillRoundRect(modeBoxX, modeBoxY, modeBoxWidth, boxHeight, 1, txtColor);
      display.setTextColor(bgColor);
      display.setCursor(68, 35);
      if (sceneMode == STEP_MODE) {
        display.print("STEP");
      } else {
        display.print("CUSTOM");
      }
      display.setTextColor(txtColor);
    } else {
      display.drawRoundRect(modeBoxX, modeBoxY, modeBoxWidth, boxHeight, 1, txtColor);
      display.setCursor(68, 35);
      if (sceneMode == STEP_MODE) {
        display.print("STEP");
      } else {
        display.print("CUSTOM");
      }
    }
  } else {
    display.setCursor(68, 35);
    if (sceneMode == STEP_MODE) {
      display.print("STEP");
    } else {
      display.print("CUSTOM");
    }
  }

  // Custom scene settings (only show in CUSTOM mode)
  if (sceneMode == CUSTOM_MODE) {
    // Custom scene length (max 12)
    display.setCursor(5, 45);
    display.print("- Length: ");

    int lengthBoxX = 66;
    int lengthBoxY = 38;
    int lengthBoxWidth = 10;

    if (sceneEditMode == SCENE_EDIT_CUSTOM_LENGTH) {
      if ((millis() / 300) % 2 == 0) {
        display.fillRoundRect(lengthBoxX, lengthBoxY, lengthBoxWidth, boxHeight, 1, txtColor);
        display.setTextColor(bgColor);
        display.setCursor(68, 45);
        display.print(customSceneLength);
        display.setTextColor(txtColor);
      } else {
        display.drawRoundRect(lengthBoxX, lengthBoxY, lengthBoxWidth, boxHeight, 1, txtColor);
        display.setCursor(68, 45);
        display.print(customSceneLength);
      }
    } else {
      display.setCursor(68, 45);
      display.print(customSceneLength);
    }

    // Display custom scene order horizontally
    int startX = 5;
    int startY = 55;
    int spacing = 9;  // 9 pixels between each scene value

    // Draw the scene sequence values horizontally
    for (int i = 0; i < customSceneLength; i++) {
      int xPos = startX + (i * spacing);

      // Format the scene value (add leading zero for single digits)
      String sceneValue;
      if (customSceneOrder[i] + 1 < 10) {
        sceneValue = "0" + String(customSceneOrder[i] + 1);
      } else {
        sceneValue = String(customSceneOrder[i] + 1);
      }

      // Draw a line above the currently selected sequence position (when not editing)
      if (sceneEditMode == SCENE_EDIT_NONE && sceneSelectedParameter == 3 && customSceneEditPosition == i) {
        // Draw line above the selected value
        display.drawLine(xPos + 3, startY - 7, xPos + 3, startY - 6, txtColor);
        display.drawLine(xPos + 3, startY + 2, xPos + 3, startY + 3, txtColor);
      }

      // If this position is being edited, highlight it with flashing box
      if (sceneEditMode == SCENE_EDIT_CUSTOM_VALUE && customSceneEditPosition == i) {
        if ((millis() / 300) % 2 == 0) {
          // Draw filled background for the selected scene value
          display.fillRect(xPos - 2, startY - 6, 10, 9, txtColor);
          display.setTextColor(bgColor);
          display.setCursor(xPos, startY);
          display.print(sceneValue);
          display.setTextColor(txtColor);
        } else {
          // Draw outline box for the selected scene value
          display.drawRect(xPos - 2, startY - 6, 10, 9, txtColor);
          display.setCursor(xPos, startY);
          display.print(sceneValue);
        }
      } else {
        // Normal display
        display.setCursor(xPos, startY);
        display.print(sceneValue);
      }
    }
  }

  display.display();
}

// *********************** //
// DRAW CONFIRM ASSIGN SAVE POPUP //
// *********************** //
void drawConfirmAssignSavePopup() {

  display.clearDisplay();  // CLEAR

  // Set colors based on the inversion variable
  int bgColor = displayInverted ? 0 : 1;   // Black if inverted, White if normal
  int txtColor = displayInverted ? 1 : 0;  // White if inverted, Black if normal
  int boxColor = txtColor;

  display.setFont(&Picopixel);
  display.setTextSize(1);

  // Background
  display.fillRect(0, 0, 128, 64, bgColor);

  // Popup border
  display.drawRoundRect(10, 12, 108, 40, 4, boxColor);

  // Warning text
  display.setTextColor(txtColor);
  display.setTextSize(1);
  display.setCursor(38, 22);
  display.print("Apply Changes?");

  // Y/N with box around selected option
  display.setCursor(45, 44);
  display.print("Y");
  display.setCursor(75, 44);
  display.print("N");

  // Determine box position and width based on selection
  int boxX;
  int boxWidth;
  int boxY = 38;
  int boxHeight = 9;

  if (confirmAssignSave) {
    boxX = 43;     // Box over Y
    boxWidth = 7;  // Width 7 for Y
  } else {
    boxX = 73;     // Box over N
    boxWidth = 8;  // Width 8 for N
  }

  // Draw the box around selected Y or N with flashing effect
  if ((millis() / 300) % 2 == 0) {
    // Draw filled box when flashing
    display.fillRoundRect(boxX, boxY, boxWidth, boxHeight, 1, boxColor);
    // Draw the selected letter in background color on the box
    display.setTextColor(bgColor);
    if (confirmAssignSave) {
      display.setCursor(45, 44);
      display.print("Y");
      display.setTextColor(txtColor);
      display.setCursor(75, 44);
      display.print("N");
    } else {
      display.setCursor(75, 44);
      display.print("N");
      display.setTextColor(txtColor);
      display.setCursor(45, 44);
      display.print("Y");
    }
  } else {
    // Draw outline box when not flashing
    display.drawRoundRect(boxX, boxY, boxWidth, boxHeight, 1, boxColor);
    // Draw both letters in text color
    display.setTextColor(txtColor);
    display.setCursor(45, 44);
    display.print("Y");
    display.setCursor(75, 44);
    display.print("N");
  }

  display.display();
}
// *********************** //
// DRAW SETTINGS MENU      //
// *********************** //
void drawSettingsMenuScreen() {

  display.clearDisplay();  // CLEAR

  // Set colors based on the inversion variable
  int bgColor = displayInverted ? 0 : 1;   // Black if inverted, White if normal
  int txtColor = displayInverted ? 1 : 0;  // White if inverted, Black if normal

  display.setFont(&Picopixel);
  display.setTextSize(1);

  // DRAW UI
  display.fillRect(0, 0, 128, 64, bgColor);
  display.drawRoundRect(1, 1, 126, 62, 3, txtColor);

  display.setTextColor(txtColor);
  display.setCursor(6, 11);
  display.println("Settings");

  // MENU OPTIONS
  display.setCursor(5, 25);
  display.println("- Assign Settings");
  display.setCursor(5, 35);
  display.println("- Display Settings");
  display.setCursor(5, 45);
  display.println("- About");
  display.setCursor(5, 55);
  display.println("- Factory Reset");

  // Draw the bitmap indicator based on selection
  int indicatorY = 21 + (selectedMenuItem * 10);
  display.drawBitmap(70, indicatorY, image_ctrlMessageIndicator_bits, 5, 5, txtColor);

  display.display();
}

// *********************** //
// DRAW ASSIGN SETTINGS SCREEN
// *********************** //
void drawAssignSettingsScreen() {
  display.clearDisplay();

  // Set colors based on the inversion variable
  int bgColor = displayInverted ? 0 : 1;
  int txtColor = displayInverted ? 1 : 0;

  // DRAW UI
  display.fillRect(0, 0, 128, 64, bgColor);
  display.drawRoundRect(1, 1, 126, 62, 3, txtColor);

  display.setTextColor(txtColor);
  display.setTextSize(1);
  display.setTextWrap(false);
  display.setFont(&Picopixel);
  display.setTextSize(1);
  display.setCursor(6, 11);
  display.print("Assign Settings");

  // Display the current edit pot selection
  display.setCursor(5, 25);
  display.print("- Edit Pot:");

  // Display the pot number with selection box
  int potX = 65;
  int potY = 25;

  display.setCursor(potX, potY);
  display.print("Pot ");

  if (editingAssignSetting) {
    // Flash the selection box when editing
    if ((millis() / 300) % 2 == 0) {
      display.fillRoundRect(potX + 15, potY - 6, 7, 9, 1, txtColor);
      display.setTextColor(bgColor);
      display.setCursor(potX + 17, potY);
      display.print(editPotSelection + 1);
      display.setTextColor(txtColor);
    } else {
      display.drawRoundRect(potX + 15, potY - 6, 7, 9, 1, txtColor);
      display.setCursor(potX + 17, potY);
      display.print(editPotSelection + 1);
    }
  } else {
    display.setCursor(potX + 17, potY);
    display.print(editPotSelection + 1);
  }

  display.display();
}

// *********************** //
// DRAW DISPLAY SETTINGS   //
// *********************** //
void drawDisplaySettings() {

  display.clearDisplay();  // CLEAR
  display.setFont(&Picopixel);
  display.setTextSize(1);

  // Set colors based on the inversion variable
  int bgColor = displayInverted ? 0 : 1;   // Black if inverted, White if normal
  int txtColor = displayInverted ? 1 : 0;  // White if inverted, Black if normal
  int boxColor = txtColor;                 // Box border uses text color

  // DRAW UI
  display.fillRect(0, 0, 128, 64, bgColor);
  display.drawRoundRect(1, 1, 126, 62, 3, txtColor);

  display.setTextColor(txtColor);
  display.setCursor(6, 11);
  display.println("Display settings");

  // MENU OPTIONS
  display.setCursor(5, 25);
  display.println("- Theme");

  display.setCursor(5, 35);
  display.println("- Invert Display");

  // Display current theme value (right-aligned)
  String themeStr;
  if (displayTheme == 0) {
    themeStr = "MkII";
  } else if (displayTheme == 1) {
    themeStr = "MkI";
  } else if (displayTheme == 2) {
    themeStr = "YX";
  } else {
    themeStr = "???";
  }

  // Calculate position to right-align theme value
  int16_t x1, y1;
  uint16_t w, h;
  display.getTextBounds(themeStr, 0, 0, &x1, &y1, &w, &h);
  int themeX = 93 - w;  // Right-aligned
  display.setCursor(themeX, 25);
  display.print(themeStr);

  // Display Y and N for invert
  display.setCursor(79, 35);
  if (displayInverted) {
    display.print("Y");
  } else {
    display.print("N");
  }

  // Draw selection box based on selectedMenuItem
  if (selectedMenuItem == 0) {
    // Theme selected - draw box around the theme value
    int boxX = themeX - 2;
    int boxY = 19;
    int boxW = w + 4;
    int boxH = 10;

    if (editingDisplayTheme) {
      // Flash the box when editing
      if ((millis() / 300) % 2 == 0) {
        display.fillRoundRect(boxX, boxY, boxW, boxH, 1, txtColor);
        display.setTextColor(bgColor);
        display.setCursor(themeX, 25);
        display.print(themeStr);
        display.setTextColor(txtColor);
      } else {
        display.drawRoundRect(boxX, boxY, boxW, boxH, 1, txtColor);
        display.setCursor(themeX, 25);
        display.print(themeStr);
      }
    } else {
      // Draw selection box around theme
      display.drawRoundRect(boxX, boxY, boxW, boxH, 1, txtColor);
      display.setCursor(themeX, 25);
      display.print(themeStr);
    }
  } else if (selectedMenuItem == 1) {
    // Invert Display selected
    int boxX = 77;
    int boxY = 29;
    int boxW = displayInverted ? 7 : 8;
    int boxH = 9;

    if (editingDisplaySetting) {
      // Flash the box when editing
      if ((millis() / 300) % 2 == 0) {
        display.fillRoundRect(boxX, boxY, boxW, boxH, 1, txtColor);
        display.setTextColor(bgColor);
        if (displayInverted) {
          display.setCursor(79, 35);
          display.print("Y");
          display.setTextColor(txtColor);
        } else {
          display.setCursor(79, 35);
          display.print("N");
          display.setTextColor(txtColor);
        }
      } else {
        display.drawRoundRect(boxX, boxY, boxW, boxH, 1, txtColor);
        display.setTextColor(txtColor);
        display.setCursor(79, 35);
        if (displayInverted) {
          display.print("Y");
        } else {
          display.print("N");
        }
      }
    } else {
      // Draw selection box around selected Y/N
      display.drawRoundRect(boxX, boxY, boxW, boxH, 1, txtColor);
      display.setTextColor(txtColor);
      display.setCursor(79, 35);
      if (displayInverted) {
        display.print("Y");
      } else {
        display.print("N");
      }
    }
  } else {
    // Neither selected - just draw normally
    display.setTextColor(txtColor);
    display.setCursor(79, 35);
    if (displayInverted) {
      display.print("Y");
    } else {
      display.print("N");
    }
  }

  // Draw the bitmap indicator based on selection
  int indicatorY = 21 + (selectedMenuItem * 10);
  display.drawBitmap(105, indicatorY, image_ctrlMessageIndicator_bits, 5, 5, txtColor);

  display.display();
}

// *********************** //
// DRAW ABOUT SCREEN       //
// *********************** //
void drawAboutScreen() {
  display.clearDisplay();

  // Set colors based on the inversion variable
  int bgColor = displayInverted ? 0 : 1;   // Black if inverted, White if normal
  int txtColor = displayInverted ? 1 : 0;  // White if inverted, Black if normal

  display.setFont(&Picopixel);
  display.setTextSize(1);

  // DRAW UI
  display.fillRect(0, 0, 128, 64, bgColor);
  display.drawRoundRect(1, 1, 126, 62, 3, txtColor);
  display.drawRoundRect(1, 2, 126, 60, 4, txtColor);

  display.setTextColor(txtColor);
  display.setTextSize(2);
  display.setTextWrap(false);
  display.setCursor(46, 14);
  display.print("MC8P");

  display.setTextSize(1);
  display.setCursor(33, 35);
  display.print("8 pot open-source");

  display.setCursor(41, 25);
  display.print("firmware v2.1");

  display.setCursor(34, 57);
  display.print(".axs instruments");

  display.setCursor(39, 43);
  display.print("MIDI controller");

  display.display();
}

// *********************** //
// DRAW CONFIRM RESET POPUP //
// *********************** //
void drawConfirmResetPopup() {

  display.clearDisplay();  // CLEAR

  // Set colors based on the inversion variable
  int bgColor = displayInverted ? 0 : 1;   // Black if inverted, White if normal
  int txtColor = displayInverted ? 1 : 0;  // White if inverted, Black if normal
  int boxColor = txtColor;

  display.setFont(&Picopixel);
  display.setTextSize(1);

  // Background
  display.fillRect(0, 0, 128, 64, bgColor);

  // Popup border
  display.drawRoundRect(10, 12, 108, 40, 4, boxColor);

  // Warning text
  display.setTextColor(txtColor);
  display.setTextSize(1);
  display.setCursor(35, 22);
  display.print("Factory Reset?");
  display.setCursor(20, 32);
  display.print("All settings will be");
  display.setCursor(20, 42);
  display.print("lost!");

  // Y/N with box around selected option
  display.setCursor(45, 45);
  display.print("Y");
  display.setCursor(75, 45);
  display.print("N");

  // Determine box position and width based on selection
  int boxX;
  int boxWidth;
  int boxY = 39;
  int boxHeight = 9;

  if (confirmSelection) {
    boxX = 43;     // Box over Y
    boxWidth = 7;  // Width 7 for Y
  } else {
    boxX = 73;     // Box over N
    boxWidth = 8;  // Width 8 for N
  }

  // Draw the box around selected Y or N with flashing effect
  if ((millis() / 300) % 2 == 0) {
    // Draw filled box when flashing
    display.fillRoundRect(boxX, boxY, boxWidth, boxHeight, 1, boxColor);
    // Draw the selected letter in background color on the box
    display.setTextColor(bgColor);
    if (confirmSelection) {
      display.setCursor(45, 45);
      display.print("Y");
      display.setTextColor(txtColor);
      display.setCursor(75, 45);
      display.print("N");
    } else {
      display.setCursor(75, 45);
      display.print("N");
      display.setTextColor(txtColor);
      display.setCursor(45, 45);
      display.print("Y");
    }
  } else {
    // Draw outline box when not flashing
    display.drawRoundRect(boxX, boxY, boxWidth, boxHeight, 1, boxColor);
    // Draw both letters in text color
    display.setTextColor(txtColor);
    display.setCursor(45, 45);
    display.print("Y");
    display.setCursor(75, 45);
    display.print("N");
  }

  display.display();
}
