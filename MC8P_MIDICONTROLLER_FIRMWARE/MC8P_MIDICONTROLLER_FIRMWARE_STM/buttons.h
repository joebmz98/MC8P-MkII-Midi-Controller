// =============================================================================
// buttons.h — ASSIGN / ENTER / PREV / NEXT handling for every screen
//
// readButtons() runs every loop(): debounces the four buttons, then acts on
// the current screen (menu navigation, ASSIGN editing, scenes, settings,
// save / reset popups). Holding ENTER on the main screen starts the
// temporary override (temp_override.h).
// =============================================================================
#pragma once
#include "hardware.h"
#include "state.h"
#include "prototypes.h"
#include "assign.h"
#include "scenes.h"
#include "storage.h"
#include "temp_override.h"

// *************** //
// BUTTON HANDLING
// *************** //
void readButtons() {

  // TRACK BUTTONS HELD
  assignButtonHeld = (buttonState[0] == HIGH);
  enterButtonHeld = (buttonState[1] == HIGH);

  // TRACKING ADD/REMOVE OPERATION
  static bool inAddRemoveOperation = false;

  for (int i = 0; i < NUM_BUTTONS; i++) {
    int reading = digitalRead(BUTTON_PINS[i]);

    if (reading != lastButtonState[i]) {
      lastDebounceTime[i] = millis();
    }

    if ((millis() - lastDebounceTime[i]) > debounceDelay) {
      if (reading != buttonState[i]) {
        if (reading == HIGH) {

          // BUTTON PRESS CHECK
          buttonState[i] = HIGH;
          Serial.print("Button pressed: ");
          Serial.println(BUTTON_NAMES[i]);

          // *********************** //
          // SINGLE PRESS ACTION BUTTON HANDLING
          // *********************** //
          switch (i) {

            case 0:  // ASSIGN BUTTON
              if (currentScreen == MAIN_SCREEN) {
                // Check if we're in TEMP mode
                if (tempOverrideActive) {
                  // Deactivate temp mode first
                  deactivateTempOverride();
                }
                // Single press from MAIN_SCREEN goes to MENU_SCREEN
                currentScreen = MENU_SCREEN;
                selectedMenuItem = 0;  // Reset to first menu item
                Serial.println("ASSIGN pressed - switching to MENU_SCREEN");
              } else if (currentScreen == SCENE_SCREEN) {
                // From SCENE_SCREEN, ASSIGN button cancels editing and reverts value
                if (sceneEditMode != SCENE_EDIT_NONE) {
                  // Revert to original value and exit edit mode
                  if (sceneEditMode == SCENE_EDIT_CC) {
                    sceneCC = originalSceneCC;
                    Serial.print("Scene CC edit cancelled, reverted to: ");
                    Serial.println(sceneCC);
                  } else if (sceneEditMode == SCENE_EDIT_MODE) {
                    sceneMode = originalSceneMode;
                    Serial.print("Scene mode edit cancelled, reverted to: ");
                    Serial.println(sceneMode == STEP_MODE ? "STEP" : "CUSTOM");
                  } else if (sceneEditMode == SCENE_EDIT_CUSTOM_LENGTH || sceneEditMode == SCENE_EDIT_CUSTOM_VALUE) {
                    // Reload custom scene data from EEPROM or revert
                    Serial.println("Custom scene edit cancelled");
                  }
                  sceneEditMode = SCENE_EDIT_NONE;
                  selectingStep = false;
                  drawSceneScreen();
                } else {
                  // If not editing, save current step position before leaving
                  if (sceneMode == CUSTOM_MODE && selectingStep) {
                    lastCustomSceneEditPosition = customSceneEditPosition;
                  }
                  // If not editing, just go back to MENU_SCREEN
                  currentScreen = MENU_SCREEN;
                  selectedMenuItem = 1;  // Keep Scene Mode selected in menu
                  Serial.println("ASSIGN pressed - returning to MENU_SCREEN from SCENE_SCREEN");
                }
              } else if (currentScreen == MENU_SCREEN) {
                // From MENU_SCREEN, ASSIGN button goes back to MAIN_SCREEN
                currentScreen = MAIN_SCREEN;
                Serial.println("ASSIGN pressed - returning to MAIN_SCREEN");
              } else if (currentScreen == ASSIGN_SCREEN) {
                // New ASSIGN button behavior for ASSIGN_SCREEN
                if (assignEditMode == ASSIGN_POT_SELECT && !assignEditingMode) {
                  // In POT_SELECT mode - show confirmation popup to save
                  currentScreen = CONFIRM_ASSIGN_SAVE_SCREEN;
                  confirmAssignSave = true;  // Default to Yes
                  Serial.println("ASSIGN pressed in POT_SELECT mode - showing save confirmation");
                } else if (assignEditMode == ASSIGN_MESSAGE_SELECT && !assignEditingMode) {
                  // In MESSAGE_SELECT mode - go back to POT_SELECT mode
                  assignEditMode = ASSIGN_POT_SELECT;
                  selectedMessage = 0;
                  scrollOffset = 0;
                  selectedAddRemove = 2;  // Reset to message selection
                  Serial.println("ASSIGN pressed in MESSAGE_SELECT mode - returning to POT_SELECT mode");
                } else if (assignEditingMode) {
                  // In any EDIT mode (CHANNEL, CC, MIN, MAX, INVERT) - go back to MESSAGE_SELECT mode
                  assignEditingMode = false;
                  assignEditMode = ASSIGN_MESSAGE_SELECT;
                  selectedAddRemove = 2;  // Reset to message selection
                  Serial.println("ASSIGN pressed in EDIT mode - returning to MESSAGE_SELECT mode");
                } else {
                  // Default behavior for any other state
                  currentScreen = CONFIRM_ASSIGN_SAVE_SCREEN;
                  confirmAssignSave = true;
                  Serial.println("ASSIGN pressed - showing save confirmation");
                }
              } else if (currentScreen == CONFIRM_ASSIGN_SAVE_SCREEN) {
                // On the confirm screen, ASSIGN button cancels and returns to ASSIGN_SCREEN
                currentScreen = ASSIGN_SCREEN;
                Serial.println("ASSIGN pressed - returning to ASSIGN_SCREEN");
              } else if (currentScreen == ASSIGN_SETTINGS_SCREEN) {
                if (editingAssignSetting) {
                  // Exit edit mode without saving changes
                  editingAssignSetting = false;
                  Serial.println("ASSIGN pressed - exiting edit mode without saving");
                } else {
                  // Save settings before exiting
                  saveGlobalSettingsToEEPROM();
                  // Go back to SETTINGS_SCREEN
                  currentScreen = SETTINGS_SCREEN;
                  selectedMenuItem = 0;  // Keep Assign Settings selected
                  Serial.println("ASSIGN pressed - saving and returning to SETTINGS_SCREEN");
                }
              } else if (currentScreen == SETTINGS_SCREEN) {
                // From SETTINGS_SCREEN, ASSIGN button goes back to MENU_SCREEN
                currentScreen = MENU_SCREEN;
                selectedMenuItem = 2;  // Keep Settings selected in menu
                Serial.println("ASSIGN pressed - returning to MENU_SCREEN from SETTINGS_SCREEN");
              } else if (currentScreen == DISPLAY_SETTINGS_SCREEN) {
                // Handle both Theme (0) and Invert (1) menu items
                if (selectedMenuItem == 0) {
                  // Theme menu item
                  if (editingDisplayTheme) {
                    // Exit theme edit mode without saving - revert to original
                    editingDisplayTheme = false;
                    displayTheme = originalDisplayTheme;
                    Serial.print("ASSIGN pressed - exiting theme edit mode, reverted to: ");
                    Serial.println(displayTheme == 0 ? "MkII" : (displayTheme == 1 ? "MkI" : "YX"));
                  } else {
                    // Save display settings before exiting
                    saveGlobalSettingsToEEPROM();
                    // From DISPLAY_SETTINGS_SCREEN, ASSIGN button goes back to SETTINGS_SCREEN
                    currentScreen = SETTINGS_SCREEN;
                    selectedMenuItem = 1;  // Keep Display selected in settings menu
                    Serial.println("ASSIGN pressed - saving and returning to SETTINGS_SCREEN from DISPLAY_SETTINGS_SCREEN");
                  }
                } else if (selectedMenuItem == 1) {
                  // Invert Display menu item
                  if (editingDisplaySetting) {
                    // Exit edit mode without saving changes - revert to original value
                    editingDisplaySetting = false;
                    displayInverted = originalDisplayInverted;  // Revert to saved state
                    Serial.print("ASSIGN pressed - exiting invert edit mode, reverted to: ");
                    Serial.println(displayInverted ? "YES" : "NO");
                  } else {
                    // Save display settings before exiting
                    saveGlobalSettingsToEEPROM();
                    // From DISPLAY_SETTINGS_SCREEN, ASSIGN button goes back to SETTINGS_SCREEN
                    currentScreen = SETTINGS_SCREEN;
                    selectedMenuItem = 1;  // Keep Display selected in settings menu
                    Serial.println("ASSIGN pressed - saving and returning to SETTINGS_SCREEN from DISPLAY_SETTINGS_SCREEN");
                  }
                }
              } else if (currentScreen == ABOUT_SCREEN) {
                // From ABOUT_SCREEN, ASSIGN button goes back to SETTINGS_SCREEN
                currentScreen = SETTINGS_SCREEN;
                selectedMenuItem = 2;  // Keep About selected in settings menu
                Serial.println("ASSIGN pressed - returning to SETTINGS_SCREEN from ABOUT_SCREEN");
              } else if (currentScreen == RESET_SCREEN) {
                // From RESET_SCREEN, ASSIGN button goes back to SETTINGS_SCREEN
                currentScreen = SETTINGS_SCREEN;
                selectedMenuItem = 3;  // Keep Factory Reset selected in settings menu
                Serial.println("ASSIGN pressed - returning to SETTINGS_SCREEN from RESET_SCREEN");
              } else if (currentScreen == CONFIRM_RESET_SCREEN) {
                // Cancel reset and return to SETTINGS_SCREEN
                currentScreen = SETTINGS_SCREEN;
                selectedMenuItem = 3;  // Keep Factory Reset selected
                Serial.println("ASSIGN pressed - Factory reset cancelled, returning to SETTINGS_SCREEN");
              }
              break;

            case 1:  // ENTER BUTTON SINGLE PRESS
              // RESET ADD/REMOVE STATE ON ENTER PRESS
              inAddRemoveOperation = false;

              // MENU SCREEN - CONFIRM SELECTION
              if (currentScreen == MENU_SCREEN) {
                switch (selectedMenuItem) {
                  case 0:  // CC assign
                    currentScreen = ASSIGN_SCREEN;
                    selectedPot = 0;
                    selectedMessage = 0;
                    scrollOffset = 0;
                    selectedAddRemove = 2;  // Reset to message selection
                    // Reset assign screen navigation states
                    assignEditMode = ASSIGN_POT_SELECT;
                    assignEditingMode = false;
                    Serial.println("ENTER pressed - switching to ASSIGN_SCREEN");
                    break;
                  case 1:  // Scene Mode
                    currentScreen = SCENE_SCREEN;
                    sceneEditMode = SCENE_EDIT_NONE;  // Start in viewing mode
                    sceneSelectedParameter = 0;       // Start with CC selected

                    // Restore last step position if we were in step selection mode before
                    if (sceneMode == CUSTOM_MODE) {
                      customSceneEditPosition = lastCustomSceneEditPosition;
                      // Validate the position is within bounds
                      if (customSceneEditPosition >= customSceneLength) {
                        customSceneEditPosition = customSceneLength - 1;
                      }
                      // Don't automatically enter step selection mode
                      selectingStep = false;
                    } else {
                      selectingStep = false;
                    }
                    Serial.println("ENTER pressed - switching to SCENE_SCREEN");
                    break;
                  case 2:  // Settings
                    currentScreen = SETTINGS_SCREEN;
                    selectedMenuItem = 0;
                    Serial.println("ENTER pressed - switching to SETTINGS_SCREEN");
                    break;
                }
              }

              // SETTINGS SCREEN - CONFIRM SELECTION
              else if (currentScreen == SETTINGS_SCREEN) {
                switch (selectedMenuItem) {
                  case 0:  // Assign Settings
                    currentScreen = ASSIGN_SETTINGS_SCREEN;
                    selectedMenuItem = 0;  // Reset selection
                    editingAssignSetting = false;
                    Serial.println("ENTER pressed - switching to ASSIGN_SETTINGS_SCREEN");
                    break;
                  case 1:  // Display
                    currentScreen = DISPLAY_SETTINGS_SCREEN;
                    selectedMenuItem = 0;                       // Reset selection for Display settings
                    editingDisplaySetting = false;              // Ensure not in edit mode when entering
                    editingDisplayTheme = false;                // Ensure theme edit mode is off
                    originalDisplayInverted = displayInverted;  // Sync original with current
                    originalDisplayTheme = displayTheme;        // Sync original theme with current
                    Serial.print("ENTER pressed - switching to DISPLAY_SETTINGS_SCREEN, current invert: ");
                    Serial.println(displayInverted ? "YES" : "NO");
                    Serial.print("Current theme: ");
                    Serial.println(displayTheme == 0 ? "MkII" : (displayTheme == 1 ? "MkI" : "YX"));
                    break;
                  case 2:  // About
                    currentScreen = ABOUT_SCREEN;
                    selectedMenuItem = 0;  // Reset selection for About screen
                    Serial.println("ENTER pressed - switching to ABOUT_SCREEN");
                    break;
                  case 3:  // Factory Reset
                    currentScreen = CONFIRM_RESET_SCREEN;
                    confirmSelection = true;  // Default to Yes
                    Serial.println("ENTER pressed - switching to CONFIRM_RESET_SCREEN");
                    break;
                }
              }

              // ASSIGN SETTINGS SCREEN
              else if (currentScreen == ASSIGN_SETTINGS_SCREEN) {
                if (editingAssignSetting) {
                  // Exit edit mode and save setting
                  editingAssignSetting = false;
                  // Save to global settings
                  saveGlobalSettingsToEEPROM();
                  Serial.print("Assign Settings - Edit Pot saved as: Pot ");
                  Serial.println(editPotSelection + 1);
                } else {
                  // Enter edit mode
                  editingAssignSetting = true;
                  Serial.println("Assign Settings - EDIT MODE ACTIVATED");
                }
              }

              // DISPLAY SETTINGS SCREEN - CONFIRM SELECTION
              else if (currentScreen == DISPLAY_SETTINGS_SCREEN) {
                Serial.print("ENTER on DISPLAY_SETTINGS_SCREEN, selectedMenuItem=");
                Serial.print(selectedMenuItem);
                Serial.print(", editingDisplaySetting=");
                Serial.print(editingDisplaySetting);
                Serial.print(", editingDisplayTheme=");
                Serial.print(editingDisplayTheme);
                Serial.print(", displayInverted=");
                Serial.println(displayInverted ? "YES" : "NO");

                switch (selectedMenuItem) {
                  case 0:  // Theme
                    if (!editingDisplayTheme) {
                      // Store the current setting before entering edit mode
                      originalDisplayTheme = displayTheme;
                      // Enter edit mode
                      editingDisplayTheme = true;
                      editFlashTimer = millis();
                      flashState = true;
                      Serial.println("ENTER pressed - Theme EDIT MODE ACTIVATED");
                      Serial.print("Original theme stored as: ");
                      Serial.println(originalDisplayTheme == 0 ? "MkII" : (originalDisplayTheme == 1 ? "MkI" : "YX"));
                    } else {
                      // Exit edit mode and save setting
                      editingDisplayTheme = false;
                      Serial.print("ENTER pressed - Theme saved as: ");
                      Serial.println(displayTheme == 0 ? "MkII" : (displayTheme == 1 ? "MkI" : "YX"));
                      // Save to global EEPROM
                      saveGlobalSettingsToEEPROM();
                    }
                    break;
                  case 1:  // Invert Display
                    if (!editingDisplaySetting) {
                      // Store the current setting before entering edit mode
                      originalDisplayInverted = displayInverted;
                      // Enter edit mode
                      editingDisplaySetting = true;
                      editFlashTimer = millis();
                      flashState = true;
                      Serial.println("ENTER pressed - Invert EDIT MODE ACTIVATED");
                      Serial.print("Original value stored as: ");
                      Serial.println(originalDisplayInverted ? "YES" : "NO");
                    } else {
                      // Exit edit mode and save setting
                      editingDisplaySetting = false;
                      Serial.println("ENTER pressed - Display invert setting saved");
                      // Save to global EEPROM
                      saveGlobalSettingsToEEPROM();
                    }
                    break;
                }
              }

              // SCENE SCREEN - Handle ENTER press
              else if (currentScreen == SCENE_SCREEN) {
                if (sceneEditMode != SCENE_EDIT_NONE) {
                  // Exit edit mode and save
                  if (sceneEditMode == SCENE_EDIT_CC) {
                    saveGlobalSettingsToEEPROM();
                    Serial.print("Scene CC saved as: ");
                    Serial.println(sceneCC);
                    // Keep focus on CC parameter after saving
                    sceneSelectedParameter = 0;
                  } else if (sceneEditMode == SCENE_EDIT_MODE) {
                    saveGlobalSettingsToEEPROM();
                    Serial.print("Scene mode saved as: ");
                    Serial.println(sceneMode == STEP_MODE ? "STEP" : "CUSTOM");
                    // Keep focus on Mode parameter after saving
                    sceneSelectedParameter = 1;
                  } else if (sceneEditMode == SCENE_EDIT_CUSTOM_LENGTH) {
                    saveGlobalSettingsToEEPROM();
                    Serial.print("Custom scene length saved as: ");
                    Serial.println(customSceneLength);
                    // Ensure currentSceneValue and edit position are within new bounds
                    if (currentSceneValue >= customSceneLength) {
                      currentSceneValue = customSceneLength - 1;
                    }
                    if (customSceneEditPosition >= customSceneLength) {
                      customSceneEditPosition = customSceneLength - 1;
                    }
                    // Keep focus on Length parameter after saving
                    sceneSelectedParameter = 2;
                  } else if (sceneEditMode == SCENE_EDIT_CUSTOM_VALUE) {
                    saveGlobalSettingsToEEPROM();
                    Serial.print("Custom scene order saved");
                    // Keep focus on the sequence position after saving
                    sceneSelectedParameter = 3;
                    selectingStep = true;
                  }
                  sceneEditMode = SCENE_EDIT_NONE;
                  // Don't reset selectingStep or sceneSelectedParameter here
                  drawSceneScreen();
                } else {
                  // Enter edit mode based on selected parameter
                  if (sceneMode == STEP_MODE) {
                    // STEP mode only has CC and Mode
                    switch (sceneSelectedParameter) {
                      case 0:  // Scene CC
                        originalSceneCC = sceneCC;
                        sceneEditMode = SCENE_EDIT_CC;
                        Serial.println("Scene CC - EDIT MODE ACTIVATED");
                        break;
                      case 1:  // Scene Mode
                        originalSceneMode = sceneMode;
                        sceneEditMode = SCENE_EDIT_MODE;
                        Serial.println("Scene Mode - EDIT MODE ACTIVATED");
                        break;
                    }
                  } else {
                    // CUSTOM mode - handle all parameters
                    if (sceneSelectedParameter == 3) {
                      // Sequence position selected - enter value edit mode
                      sceneEditMode = SCENE_EDIT_CUSTOM_VALUE;
                      Serial.println("Custom Value - EDIT MODE ACTIVATED");
                    } else {
                      switch (sceneSelectedParameter) {
                        case 0:  // Scene CC
                          originalSceneCC = sceneCC;
                          sceneEditMode = SCENE_EDIT_CC;
                          Serial.println("Scene CC - EDIT MODE ACTIVATED");
                          break;
                        case 1:  // Scene Mode
                          originalSceneMode = sceneMode;
                          sceneEditMode = SCENE_EDIT_MODE;
                          Serial.println("Scene Mode - EDIT MODE ACTIVATED");
                          break;
                        case 2:  // Custom Length
                          sceneEditMode = SCENE_EDIT_CUSTOM_LENGTH;
                          Serial.println("Custom Length - EDIT MODE ACTIVATED");
                          break;
                      }
                    }
                  }
                  drawSceneScreen();
                }
              }

              // CONFIRM ASSIGN SAVE SCREEN - CONFIRM SELECTION
              else if (currentScreen == CONFIRM_ASSIGN_SAVE_SCREEN) {
                if (confirmAssignSave) {
                  // User selected YES - save settings
                  Serial.println("CONFIRM ASSIGN - Saving settings");
                  saveMidiSettingsToEEPROM();  // Save MIDI settings directly
                  // Return to MAIN_SCREEN after successful save
                  currentScreen = MAIN_SCREEN;
                  Serial.println("Returning to MAIN_SCREEN");
                } else {
                  // User selected NO - don't save, return to ASSIGN_SCREEN
                  Serial.println("CONFIRM ASSIGN - Save cancelled");
                  currentScreen = ASSIGN_SCREEN;
                  Serial.println("Returning to ASSIGN_SCREEN");
                }
              }

              // CONFIRM RESET SCREEN - CONFIRM SELECTION
              else if (currentScreen == CONFIRM_RESET_SCREEN) {
                if (confirmSelection) {
                  // User selected YES - perform factory reset
                  Serial.println("CONFIRM - Performing factory reset");
                  factoryReset();
                  // Return to MAIN_SCREEN after reset
                  currentScreen = MAIN_SCREEN;
                  Serial.println("Returning to MAIN_SCREEN");
                } else {
                  // User selected NO - cancel reset
                  Serial.println("CONFIRM - Factory reset cancelled");
                  currentScreen = SETTINGS_SCREEN;
                  selectedMenuItem = 3;  // Keep Factory Reset selected
                  Serial.println("Returning to SETTINGS_SCREEN");
                }
              }

              // ABOUT SCREEN - No selections to confirm
              else if (currentScreen == ABOUT_SCREEN) {
                Serial.println("ENTER pressed on ABOUT_SCREEN - no action");
              }

              // RESET SCREEN - No selections to confirm
              else if (currentScreen == RESET_SCREEN) {
                Serial.println("ENTER pressed on RESET_SCREEN - no action");
              }

              // ASSIGN SCREEN - Handle ENTER press for navigation and editing
              else if (currentScreen == ASSIGN_SCREEN) {
                if (assignEditingMode) {
                  // Cycle to next parameter or exit edit mode
                  switch (assignEditMode) {
                    case ASSIGN_EDIT_CHANNEL:
                      assignEditMode = ASSIGN_EDIT_CC;
                      Serial.println("Now editing CC");
                      break;
                    case ASSIGN_EDIT_CC:
                      assignEditMode = ASSIGN_EDIT_MIN;
                      Serial.println("Now editing Min Value");
                      break;
                    case ASSIGN_EDIT_MIN:
                      assignEditMode = ASSIGN_EDIT_MAX;
                      Serial.println("Now editing Max Value");
                      break;
                    case ASSIGN_EDIT_MAX:
                      assignEditMode = ASSIGN_EDIT_INVERT;
                      Serial.println("Now editing Direction");
                      break;
                    case ASSIGN_EDIT_INVERT:
                      assignEditMode = ASSIGN_MESSAGE_SELECT;
                      assignEditingMode = false;
                      selectedAddRemove = 2;  // Reset to message selection
                      Serial.println("Exited edit mode");
                      break;
                    default:
                      assignEditMode = ASSIGN_MESSAGE_SELECT;
                      assignEditingMode = false;
                      selectedAddRemove = 2;
                      break;
                  }
                } else {
                  // Enter edit mode based on current selection
                  if (assignEditMode == ASSIGN_POT_SELECT) {
                    // Enter message selection mode
                    assignEditMode = ASSIGN_MESSAGE_SELECT;
                    selectedAddRemove = 2;  // Reset to message selection
                    Serial.println("Entered message selection mode");
                  } else if (assignEditMode == ASSIGN_MESSAGE_SELECT) {
                    // Check if we're on Add or Remove button
                    if (selectedAddRemove == 0) {
                      // Add button selected
                      if (messageCount[selectedPot] < MAX_MESSAGES_PER_POT) {
                        addMidiControl();
                        Serial.println("Added new MIDI control");
                      } else {
                        Serial.println("Maximum messages reached for this pot");
                      }
                    } else if (selectedAddRemove == 1) {
                      // Remove button selected
                      if (messageCount[selectedPot] > 1) {
                        removeMidiControl();
                        Serial.println("Removed MIDI control");
                      } else {
                        Serial.println("Cannot remove the last MIDI message");
                      }
                    } else if (messageCount[selectedPot] > 0) {
                      // Enter edit mode for selected message
                      assignEditingMode = true;
                      assignEditMode = ASSIGN_EDIT_CHANNEL;
                      Serial.println("Entered edit mode - editing Channel");
                    }
                  }
                }
              }

              // Check if we're in MAIN_SCREEN and this is the start of a press
              if (currentScreen == MAIN_SCREEN && !tempOverrideActive) {
                // Store the current time for potential override activation
                enterPressTime = millis();
                Serial.println("ENTER pressed in MAIN_SCREEN - waiting to activate temporary override");
              }
              break;

            case 2:  // PREV BUTTON SINGLE PRESS
              prevButtonPressed = true;

              // *********************** //
              // SCENE NAVIGATION ON MAIN_SCREEN
              // *********************** //
              if (currentScreen == MAIN_SCREEN && !tempOverrideActive) {
                if (sceneMode == STEP_MODE) {
                  if (currentSceneValue > 0) {
                    currentSceneValue--;
                    sendControlChange(sceneCC, currentSceneValue, SCENE_CHANNEL);
                    Serial.print("Scene changed to: ");
                    Serial.println(currentSceneValue + 1);
                    triggerSceneOverlay();   // <-- YX theme overlay
                    drawMainScreen();
                  } else {
                    Serial.println("Already at scene 1 - cannot decrease further");
                  }
                } else {  // CUSTOM_MODE
                  if (currentSceneValue > 0) {
                    currentSceneValue--;
                    sendControlChange(sceneCC, customSceneOrder[currentSceneValue], SCENE_CHANNEL);
                    Serial.print("Custom scene changed to position ");
                    Serial.print(currentSceneValue + 1);
                    Serial.print(" (value: ");
                    Serial.print(customSceneOrder[currentSceneValue] + 1);
                    Serial.println(")");
                    triggerSceneOverlay();   // <-- YX theme overlay
                    drawMainScreen();
                  } else {
                    Serial.println("Already at first custom scene - cannot decrease further");
                  }
                }
              }

              // *********************** //
              // SCENE SCREEN NAVIGATION
              // *********************** //
              else if (currentScreen == SCENE_SCREEN) {
                if (sceneEditMode == SCENE_EDIT_NONE) {
                  // Check if we're in step selection mode (for CUSTOM mode)
                  if (sceneMode == CUSTOM_MODE && selectingStep) {
                    // Navigate between steps in the sequence
                    if (customSceneEditPosition > 0) {
                      customSceneEditPosition--;
                      Serial.print("Selected step position: ");
                      Serial.println(customSceneEditPosition + 1);
                      drawSceneScreen();
                    } else if (customSceneEditPosition == 0) {
                      // At first step, move back to Length parameter
                      selectingStep = false;
                      sceneSelectedParameter = 2;  // Length parameter
                      Serial.println("Moved back to Length parameter");
                      drawSceneScreen();
                    }
                  } else {
                    // Navigate between parameters
                    if (sceneMode == STEP_MODE) {
                      sceneSelectedParameter = (sceneSelectedParameter - 1 + 2) % 2;  // Only CC and Mode
                      drawSceneScreen();
                    } else {
                      // CUSTOM mode parameter navigation
                      if (sceneSelectedParameter == 0) {
                        // At CC, go to sequence positions (last step)
                        sceneSelectedParameter = 3;
                        selectingStep = true;
                        customSceneEditPosition = customSceneLength - 1;  // Start at last step
                        Serial.println("Entered step selection mode at last step");
                      } else if (sceneSelectedParameter == 3) {
                        // At sequence positions, move to Length
                        sceneSelectedParameter = 2;
                        selectingStep = false;
                        Serial.println("Moved to Length parameter");
                      } else if (sceneSelectedParameter == 2) {
                        // At Length, move to Mode
                        sceneSelectedParameter = 1;
                      } else if (sceneSelectedParameter == 1) {
                        // At Mode, move to CC
                        sceneSelectedParameter = 0;
                      }
                      drawSceneScreen();
                    }
                  }
                } else if (sceneEditMode == SCENE_EDIT_CC) {
                  // Decrease scene CC value
                  if (sceneCC > 0) {
                    sceneCC--;
                    Serial.print("Scene CC decreased to: ");
                    Serial.println(sceneCC);
                    drawSceneScreen();
                  }
                } else if (sceneEditMode == SCENE_EDIT_MODE) {
                  // Toggle between STEP and CUSTOM modes
                  sceneMode = (sceneMode == STEP_MODE) ? CUSTOM_MODE : STEP_MODE;
                  Serial.print("Scene mode toggled to: ");
                  Serial.println(sceneMode == STEP_MODE ? "STEP" : "CUSTOM");
                  // Reset selected parameter when switching modes
                  sceneSelectedParameter = 0;
                  selectingStep = false;
                  drawSceneScreen();
                } else if (sceneEditMode == SCENE_EDIT_CUSTOM_LENGTH) {
                  // Decrease custom scene length
                  if (customSceneLength > 1) {
                    customSceneLength--;
                    if (customSceneEditPosition >= customSceneLength) {
                      customSceneEditPosition = customSceneLength - 1;
                    }
                    if (currentSceneValue >= customSceneLength) {
                      currentSceneValue = customSceneLength - 1;
                    }
                    Serial.print("Custom scene length decreased to: ");
                    Serial.println(customSceneLength);
                    drawSceneScreen();
                  }
                } else if (sceneEditMode == SCENE_EDIT_CUSTOM_VALUE) {
                  // Decrease the scene value at current position
                  if (customSceneOrder[customSceneEditPosition] > 0) {
                    customSceneOrder[customSceneEditPosition]--;
                    Serial.print("Custom scene at position ");
                    Serial.print(customSceneEditPosition + 1);
                    Serial.print(" set to: ");
                    Serial.println(customSceneOrder[customSceneEditPosition] + 1);
                    drawSceneScreen();
                  }
                }
              }

              // *********************** //
              // MENU SCREEN NAVIGATION
              // *********************** //
              else if (currentScreen == MENU_SCREEN) {
                selectedMenuItem = (selectedMenuItem - 1 + 3) % 3;  // 3 menu items (CC assign, Scenes, Settings)
                Serial.print("Menu selection: ");
                switch (selectedMenuItem) {
                  case 0: Serial.println("CC assign"); break;
                  case 1: Serial.println("Scenes"); break;
                  case 2: Serial.println("Settings"); break;
                }
              }

              // *********************** //
              // SETTINGS SCREEN NAVIGATION
              // *********************** //
              else if (currentScreen == SETTINGS_SCREEN) {
                selectedMenuItem = (selectedMenuItem - 1 + 4) % 4;  // 4 settings menu items
                Serial.print("Settings menu selection: ");
                switch (selectedMenuItem) {
                  case 0: Serial.println("Assign Settings"); break;
                  case 1: Serial.println("Display"); break;
                  case 2: Serial.println("About"); break;
                  case 3: Serial.println("Factory Reset"); break;
                }
              }

              // *********************** //
              // ASSIGN SETTINGS SCREEN NAVIGATION
              // *********************** //
              else if (currentScreen == ASSIGN_SETTINGS_SCREEN && editingAssignSetting) {
                editPotSelection = (editPotSelection - 1 + N_POTS) % N_POTS;
                // Save immediately when changing
                saveGlobalSettingsToEEPROM();
                Serial.print("Edit Pot set to: Pot ");
                Serial.println(editPotSelection + 1);
              }

              // *********************** //
              // DISPLAY SETTINGS SCREEN NAVIGATION
              // *********************** //
              else if (currentScreen == DISPLAY_SETTINGS_SCREEN) {
                Serial.print("PREV on DISPLAY_SETTINGS_SCREEN, editingDisplaySetting=");
                Serial.print(editingDisplaySetting);
                Serial.print(", editingDisplayTheme=");
                Serial.print(editingDisplayTheme);
                Serial.print(", selectedMenuItem=");
                Serial.println(selectedMenuItem);

                if (selectedMenuItem == 0 && editingDisplayTheme) {
                  // Cycle theme backwards
                  if (displayTheme > 0) {
                    displayTheme--;
                  } else {
                    displayTheme = NUM_THEMES - 1;
                  }
                  Serial.print("PREV changed theme to: ");
                  Serial.println(displayTheme == 0 ? "MkII" : (displayTheme == 1 ? "MkI" : "YX"));
                  drawDisplaySettings();
                } else if (selectedMenuItem == 1 && editingDisplaySetting) {
                  // Toggle the actual boolean that moves the box
                  bool oldValue = displayInverted;
                  displayInverted = !displayInverted;

                  // Save immediately when toggling
                  saveGlobalSettingsToEEPROM();

                  Serial.print("PREV toggled displayInverted from ");
                  Serial.print(oldValue ? "YES" : "NO");
                  Serial.print(" to ");
                  Serial.println(displayInverted ? "YES" : "NO");
                  drawDisplaySettings();
                } else {
                  // Not in edit mode - navigate between Theme (0) and Invert (1)
                  if (selectedMenuItem == 0) {
                    selectedMenuItem = 1;
                  } else {
                    selectedMenuItem = 0;
                  }
                  Serial.print("PREV navigated to menu item: ");
                  Serial.println(selectedMenuItem);
                  drawDisplaySettings();
                }
              }

              // *********************** //
              // CONFIRM ASSIGN SAVE SCREEN NAVIGATION
              // *********************** //
              else if (currentScreen == CONFIRM_ASSIGN_SAVE_SCREEN) {
                // Toggle between Y and N
                confirmAssignSave = !confirmAssignSave;
                Serial.print("Confirm assign save toggled to: ");
                Serial.println(confirmAssignSave ? "YES" : "NO");
              }

              // *********************** //
              // CONFIRM RESET SCREEN NAVIGATION
              // *********************** //
              else if (currentScreen == CONFIRM_RESET_SCREEN) {
                // Toggle between Y and N
                confirmSelection = !confirmSelection;
                Serial.print("Confirm selection toggled to: ");
                Serial.println(confirmSelection ? "YES" : "NO");
              }

              // *********************** //
              // ASSIGN SCREEN NAVIGATION
              // *********************** //
              else if (currentScreen == ASSIGN_SCREEN) {
                if (assignEditingMode) {
                  // Edit mode - modify values
                  switch (assignEditMode) {
                    case ASSIGN_EDIT_CHANNEL:
                      // Handle channel with OMNI option (channel 17 = OMNI)
                      if (potMessages[selectedPot][selectedMessage].channel == 0) {
                        // If at channel 1, wrap to OMNI
                        potMessages[selectedPot][selectedMessage].channel = 16;  // OMNI (16 in 0-based)
                      } else if (potMessages[selectedPot][selectedMessage].channel == 16) {
                        // If at OMNI, wrap to channel 16
                        potMessages[selectedPot][selectedMessage].channel = 15;  // Channel 16 (15 in 0-based)
                      } else {
                        // Normal decrement
                        potMessages[selectedPot][selectedMessage].channel--;
                      }
                      Serial.print("Channel set to: ");
                      if (potMessages[selectedPot][selectedMessage].channel == 16) {
                        Serial.println("OMNI");
                      } else {
                        Serial.println(potMessages[selectedPot][selectedMessage].channel + 1);
                      }
                      break;
                    case ASSIGN_EDIT_CC:
                      potMessages[selectedPot][selectedMessage].cc =
                        (potMessages[selectedPot][selectedMessage].cc - 1 + 128) % 128;
                      Serial.print("CC set to: ");
                      Serial.println(potMessages[selectedPot][selectedMessage].cc);
                      break;
                    case ASSIGN_EDIT_INVERT:
                      // Cycle backwards through curve types: NOR -> ILG -> LOG -> INV -> NOR
                      switch (potMessages[selectedPot][selectedMessage].curve) {
                        case CURVE_NOR:
                          potMessages[selectedPot][selectedMessage].curve = CURVE_ILG;
                          Serial.println("Curve set to: ILG");
                          break;
                        case CURVE_INV:
                          potMessages[selectedPot][selectedMessage].curve = CURVE_NOR;
                          Serial.println("Curve set to: NOR");
                          break;
                        case CURVE_LOG:
                          potMessages[selectedPot][selectedMessage].curve = CURVE_INV;
                          Serial.println("Curve set to: INV");
                          break;
                        case CURVE_ILG:
                          potMessages[selectedPot][selectedMessage].curve = CURVE_LOG;
                          Serial.println("Curve set to: LOG");
                          break;
                      }
                      break;
                    case ASSIGN_EDIT_MIN:
                      if (potMessages[selectedPot][selectedMessage].minValue > 0) {
                        potMessages[selectedPot][selectedMessage].minValue--;
                        if (potMessages[selectedPot][selectedMessage].minValue > potMessages[selectedPot][selectedMessage].maxValue) {
                          potMessages[selectedPot][selectedMessage].maxValue =
                            potMessages[selectedPot][selectedMessage].minValue;
                        }
                      }
                      Serial.print("Min value set to: ");
                      Serial.println(potMessages[selectedPot][selectedMessage].minValue);
                      break;
                    case ASSIGN_EDIT_MAX:
                      // PREV button should DECREASE the max value (move down)
                      if (potMessages[selectedPot][selectedMessage].maxValue > 0) {
                        potMessages[selectedPot][selectedMessage].maxValue--;
                        if (potMessages[selectedPot][selectedMessage].maxValue < potMessages[selectedPot][selectedMessage].minValue) {
                          potMessages[selectedPot][selectedMessage].minValue =
                            potMessages[selectedPot][selectedMessage].maxValue;
                        }
                      }
                      Serial.print("Max value set to: ");
                      Serial.println(potMessages[selectedPot][selectedMessage].maxValue);
                      break;
                    default:
                      break;
                  }
                } else {
                  // Selection mode - navigate between pots, messages, or add/remove buttons
                  if (assignEditMode == ASSIGN_POT_SELECT) {
                    selectedPot = (selectedPot - 1 + N_POTS) % N_POTS;
                    selectedMessage = 0;
                    scrollOffset = 0;
                    Serial.print("Selected Pot ");
                    Serial.println(selectedPot + 1);
                  } else if (assignEditMode == ASSIGN_MESSAGE_SELECT) {
                    // Navigate through messages and Add/Remove buttons
                    if (messageCount[selectedPot] > 0) {
                      if (selectedAddRemove == 2 && selectedMessage == 0) {
                        // At first message, prev goes to Remove button
                        selectedAddRemove = 1;
                      } else if (selectedAddRemove == 1) {
                        // On Remove button, prev goes to Add button
                        selectedAddRemove = 0;
                      } else if (selectedAddRemove == 0) {
                        // On Add button, prev goes to last message
                        selectedAddRemove = 2;
                        selectedMessage = messageCount[selectedPot] - 1;
                        // Update scroll offset to show the last message
                        scrollOffset = max(0, selectedMessage - MAX_VISIBLE_MESSAGES + 1);
                        Serial.print("Selected Message ");
                        Serial.println(selectedMessage);
                        Serial.print("Scroll offset: ");
                        Serial.println(scrollOffset);
                      } else {
                        // Normal message navigation - decrement message index
                        selectedMessage = (selectedMessage - 1 + messageCount[selectedPot]) % messageCount[selectedPot];
                        // Update scroll offset to keep selected message visible
                        if (selectedMessage < scrollOffset) {
                          // Selected message is above visible area, scroll up
                          scrollOffset = selectedMessage;
                        } else if (selectedMessage >= scrollOffset + MAX_VISIBLE_MESSAGES) {
                          // Selected message is below visible area, scroll down
                          scrollOffset = selectedMessage - MAX_VISIBLE_MESSAGES + 1;
                        }
                        // Constrain scroll offset
                        scrollOffset = constrain(scrollOffset, 0, max(0, messageCount[selectedPot] - MAX_VISIBLE_MESSAGES));
                        Serial.print("Selected Message ");
                        Serial.println(selectedMessage);
                        Serial.print("Scroll offset: ");
                        Serial.println(scrollOffset);
                      }
                    } else {
                      // No messages, just toggle between Add and Remove
                      selectedAddRemove = (selectedAddRemove - 1 + 2) % 2;
                    }
                  }
                }
              }
              break;

            case 3:  // NEXT BUTTON
              nextButtonPressed = true;

              // *********************** //
              // SCENE NAVIGATION ON MAIN_SCREEN
              // *********************** //
              if (currentScreen == MAIN_SCREEN && !tempOverrideActive) {
                if (sceneMode == STEP_MODE) {
                  if (currentSceneValue < 98) {  // Maximum value 98 for 99 scenes
                    currentSceneValue++;
                    sendControlChange(sceneCC, currentSceneValue, SCENE_CHANNEL);
                    Serial.print("Scene changed to: ");
                    Serial.println(currentSceneValue + 1);
                    triggerSceneOverlay();   // <-- YX theme overlay
                    drawMainScreen();
                  } else {
                    Serial.println("Already at scene 99 - cannot increase further");
                  }
                } else {  // CUSTOM_MODE
                  if (currentSceneValue < customSceneLength - 1) {
                    currentSceneValue++;
                    sendControlChange(sceneCC, customSceneOrder[currentSceneValue], SCENE_CHANNEL);
                    Serial.print("Custom scene changed to position ");
                    Serial.print(currentSceneValue + 1);
                    Serial.print(" (value: ");
                    Serial.print(customSceneOrder[currentSceneValue] + 1);
                    Serial.println(")");
                    triggerSceneOverlay();   // <-- YX theme overlay
                    drawMainScreen();
                  } else {
                    Serial.println("Already at last custom scene - cannot increase further");
                  }
                }
              }

              // *********************** //
              // SCENE SCREEN NAVIGATION
              // *********************** //
              else if (currentScreen == SCENE_SCREEN) {
                if (sceneEditMode == SCENE_EDIT_NONE) {
                  // Check if we're in step selection mode (for CUSTOM mode)
                  if (sceneMode == CUSTOM_MODE && selectingStep) {
                    // Navigate between steps in the sequence
                    if (customSceneEditPosition < customSceneLength - 1) {
                      customSceneEditPosition++;
                      Serial.print("Selected step position: ");
                      Serial.println(customSceneEditPosition + 1);
                      drawSceneScreen();
                    } else if (customSceneEditPosition == customSceneLength - 1) {
                      // At last step, move to CC parameter
                      selectingStep = false;
                      sceneSelectedParameter = 0;  // CC parameter
                      Serial.println("Moved to CC parameter");
                      drawSceneScreen();
                    }
                  } else {
                    // Navigate between parameters
                    if (sceneMode == STEP_MODE) {
                      sceneSelectedParameter = (sceneSelectedParameter + 1) % 2;  // Only CC and Mode
                      drawSceneScreen();
                    } else {
                      // CUSTOM mode parameter navigation
                      if (sceneSelectedParameter == 0) {
                        // At CC, move to Mode
                        sceneSelectedParameter = 1;
                      } else if (sceneSelectedParameter == 1) {
                        // At Mode, move to Length
                        sceneSelectedParameter = 2;
                      } else if (sceneSelectedParameter == 2) {
                        // At Length, move to sequence positions (first step)
                        sceneSelectedParameter = 3;
                        selectingStep = true;
                        customSceneEditPosition = 0;  // Start at first step
                        Serial.println("Entered step selection mode at first step");
                      } else if (sceneSelectedParameter == 3) {
                        // At sequence positions, wrap to CC
                        sceneSelectedParameter = 0;
                        selectingStep = false;
                        Serial.println("Moved to CC parameter");
                      }
                      drawSceneScreen();
                    }
                  }
                } else if (sceneEditMode == SCENE_EDIT_CC) {
                  // Increase scene CC value
                  if (sceneCC < 127) {
                    sceneCC++;
                    Serial.print("Scene CC increased to: ");
                    Serial.println(sceneCC);
                    drawSceneScreen();
                  }
                } else if (sceneEditMode == SCENE_EDIT_MODE) {
                  // Toggle between STEP and CUSTOM modes
                  sceneMode = (sceneMode == STEP_MODE) ? CUSTOM_MODE : STEP_MODE;
                  Serial.print("Scene mode toggled to: ");
                  Serial.println(sceneMode == STEP_MODE ? "STEP" : "CUSTOM");
                  // Reset selected parameter when switching modes
                  sceneSelectedParameter = 0;
                  selectingStep = false;
                  drawSceneScreen();
                } else if (sceneEditMode == SCENE_EDIT_CUSTOM_LENGTH) {
                  // Increase custom scene length (max 12)
                  if (customSceneLength < MAX_CUSTOM_SCENES) {
                    customSceneLength++;
                    Serial.print("Custom scene length increased to: ");
                    Serial.println(customSceneLength);
                    drawSceneScreen();
                  }
                } else if (sceneEditMode == SCENE_EDIT_CUSTOM_VALUE) {
                  // Increase the scene value at current position
                  if (customSceneOrder[customSceneEditPosition] < 98) {
                    customSceneOrder[customSceneEditPosition]++;
                    Serial.print("Custom scene at position ");
                    Serial.print(customSceneEditPosition + 1);
                    Serial.print(" set to: ");
                    Serial.println(customSceneOrder[customSceneEditPosition] + 1);
                    drawSceneScreen();
                  }
                }
              }

              // *********************** //
              // MENU SCREEN NAVIGATION
              // *********************** //
              else if (currentScreen == MENU_SCREEN) {
                selectedMenuItem = (selectedMenuItem + 1) % 3;  // 3 menu items (CC assign, Scenes, Settings)
                Serial.print("Menu selection: ");
                switch (selectedMenuItem) {
                  case 0: Serial.println("CC assign"); break;
                  case 1: Serial.println("Scenes"); break;
                  case 2: Serial.println("Settings"); break;
                }
              }

              // *********************** //
              // SETTINGS SCREEN NAVIGATION
              // *********************** //
              else if (currentScreen == SETTINGS_SCREEN) {
                selectedMenuItem = (selectedMenuItem + 1) % 4;  // 4 settings menu items
                Serial.print("Settings menu selection: ");
                switch (selectedMenuItem) {
                  case 0: Serial.println("Assign Settings"); break;
                  case 1: Serial.println("Display"); break;
                  case 2: Serial.println("About"); break;
                  case 3: Serial.println("Factory Reset"); break;
                }
              }

              // *********************** //
              // ASSIGN SETTINGS SCREEN NAVIGATION
              // *********************** //
              else if (currentScreen == ASSIGN_SETTINGS_SCREEN && editingAssignSetting) {
                editPotSelection = (editPotSelection + 1) % N_POTS;
                // Save immediately when changing
                saveGlobalSettingsToEEPROM();
                Serial.print("Edit Pot set to: Pot ");
                Serial.println(editPotSelection + 1);
              }

              // *********************** //
              // DISPLAY SETTINGS SCREEN NAVIGATION
              // *********************** //
              else if (currentScreen == DISPLAY_SETTINGS_SCREEN) {
                Serial.print("NEXT on DISPLAY_SETTINGS_SCREEN, editingDisplaySetting=");
                Serial.print(editingDisplaySetting);
                Serial.print(", editingDisplayTheme=");
                Serial.print(editingDisplayTheme);
                Serial.print(", selectedMenuItem=");
                Serial.println(selectedMenuItem);

                if (selectedMenuItem == 0 && editingDisplayTheme) {
                  // Cycle theme forwards
                  displayTheme = (displayTheme + 1) % NUM_THEMES;
                  Serial.print("NEXT changed theme to: ");
                  Serial.println(displayTheme == 0 ? "MkII" : (displayTheme == 1 ? "MkI" : "YX"));
                  drawDisplaySettings();
                } else if (selectedMenuItem == 1 && editingDisplaySetting) {
                  // Toggle the actual boolean that moves the box
                  displayInverted = !displayInverted;

                  // Save immediately when toggling
                  saveGlobalSettingsToEEPROM();

                  Serial.print("NEXT toggled displayInverted to: ");
                  Serial.println(displayInverted ? "YES" : "NO");
                  drawDisplaySettings();
                } else {
                  // Not in edit mode - navigate between Theme (0) and Invert (1)
                  if (selectedMenuItem == 0) {
                    selectedMenuItem = 1;
                  } else {
                    selectedMenuItem = 0;
                  }
                  Serial.print("NEXT navigated to menu item: ");
                  Serial.println(selectedMenuItem);
                  drawDisplaySettings();
                }
              }

              // *********************** //
              // CONFIRM ASSIGN SAVE SCREEN NAVIGATION
              // *********************** //
              else if (currentScreen == CONFIRM_ASSIGN_SAVE_SCREEN) {
                // Toggle between Y and N
                confirmAssignSave = !confirmAssignSave;
                Serial.print("Confirm assign save toggled to: ");
                Serial.println(confirmAssignSave ? "YES" : "NO");
              }

              // *********************** //
              // CONFIRM RESET SCREEN NAVIGATION
              // *********************** //
              else if (currentScreen == CONFIRM_RESET_SCREEN) {
                // Toggle between Y and N
                confirmSelection = !confirmSelection;
                Serial.print("Confirm selection toggled to: ");
                Serial.println(confirmSelection ? "YES" : "NO");
              }

              // *********************** //
              // ASSIGN SCREEN NAVIGATION
              // *********************** //
              else if (currentScreen == ASSIGN_SCREEN) {
                if (assignEditingMode) {
                  // Edit mode - modify values
                  switch (assignEditMode) {
                    case ASSIGN_EDIT_CHANNEL:
                      // Handle channel with OMNI option (channel 17 = OMNI)
                      if (potMessages[selectedPot][selectedMessage].channel == 15) {
                        // If at channel 16, wrap to OMNI
                        potMessages[selectedPot][selectedMessage].channel = 16;  // OMNI (16 in 0-based)
                      } else if (potMessages[selectedPot][selectedMessage].channel == 16) {
                        // If at OMNI, wrap to channel 1
                        potMessages[selectedPot][selectedMessage].channel = 0;
                      } else {
                        // Normal increment
                        potMessages[selectedPot][selectedMessage].channel++;
                      }
                      Serial.print("Channel set to: ");
                      if (potMessages[selectedPot][selectedMessage].channel == 16) {
                        Serial.println("OMNI");
                      } else {
                        Serial.println(potMessages[selectedPot][selectedMessage].channel + 1);
                      }
                      break;
                    case ASSIGN_EDIT_CC:
                      potMessages[selectedPot][selectedMessage].cc =
                        (potMessages[selectedPot][selectedMessage].cc + 1) % 128;
                      Serial.print("CC set to: ");
                      Serial.println(potMessages[selectedPot][selectedMessage].cc);
                      break;
                    case ASSIGN_EDIT_INVERT:
                      // Cycle forward through curve types: NOR -> INV -> LOG -> ILG -> NOR
                      switch (potMessages[selectedPot][selectedMessage].curve) {
                        case CURVE_NOR:
                          potMessages[selectedPot][selectedMessage].curve = CURVE_INV;
                          Serial.println("Curve set to: INV");
                          break;
                        case CURVE_INV:
                          potMessages[selectedPot][selectedMessage].curve = CURVE_LOG;
                          Serial.println("Curve set to: LOG");
                          break;
                        case CURVE_LOG:
                          potMessages[selectedPot][selectedMessage].curve = CURVE_ILG;
                          Serial.println("Curve set to: ILG");
                          break;
                        case CURVE_ILG:
                          potMessages[selectedPot][selectedMessage].curve = CURVE_NOR;
                          Serial.println("Curve set to: NOR");
                          break;
                      }
                      break;
                    case ASSIGN_EDIT_MIN:
                      if (potMessages[selectedPot][selectedMessage].minValue < 127) {
                        potMessages[selectedPot][selectedMessage].minValue++;
                        if (potMessages[selectedPot][selectedMessage].minValue > potMessages[selectedPot][selectedMessage].maxValue) {
                          potMessages[selectedPot][selectedMessage].maxValue =
                            potMessages[selectedPot][selectedMessage].minValue;
                        }
                      }
                      Serial.print("Min value set to: ");
                      Serial.println(potMessages[selectedPot][selectedMessage].minValue);
                      break;
                    case ASSIGN_EDIT_MAX:
                      // NEXT button should INCREASE the max value (move up)
                      if (potMessages[selectedPot][selectedMessage].maxValue < 127) {
                        potMessages[selectedPot][selectedMessage].maxValue++;
                        if (potMessages[selectedPot][selectedMessage].maxValue < potMessages[selectedPot][selectedMessage].minValue) {
                          potMessages[selectedPot][selectedMessage].minValue =
                            potMessages[selectedPot][selectedMessage].maxValue;
                        }
                      }
                      Serial.print("Max value set to: ");
                      Serial.println(potMessages[selectedPot][selectedMessage].maxValue);
                      break;
                    default:
                      break;
                  }
                } else {
                  // Selection mode - navigate between pots, messages, or add/remove buttons
                  if (assignEditMode == ASSIGN_POT_SELECT) {
                    selectedPot = (selectedPot + 1) % N_POTS;
                    selectedMessage = 0;
                    scrollOffset = 0;
                    Serial.print("Selected Pot ");
                    Serial.println(selectedPot + 1);
                  } else if (assignEditMode == ASSIGN_MESSAGE_SELECT) {
                    // Navigate through messages and Add/Remove buttons
                    if (messageCount[selectedPot] > 0) {
                      if (selectedAddRemove == 2 && selectedMessage == messageCount[selectedPot] - 1) {
                        // At last message, next goes to Add button
                        selectedAddRemove = 0;
                      } else if (selectedAddRemove == 0) {
                        // On Add button, next goes to Remove button
                        selectedAddRemove = 1;
                      } else if (selectedAddRemove == 1) {
                        // On Remove button, next goes to first message
                        selectedAddRemove = 2;
                        selectedMessage = 0;
                        // Update scroll offset to show the first message
                        scrollOffset = 0;
                        Serial.print("Selected Message ");
                        Serial.println(selectedMessage);
                        Serial.print("Scroll offset: ");
                        Serial.println(scrollOffset);
                      } else {
                        // Normal message navigation - increment message index
                        selectedMessage = (selectedMessage + 1) % messageCount[selectedPot];
                        // Update scroll offset to keep selected message visible
                        if (selectedMessage < scrollOffset) {
                          // Selected message is above visible area, scroll up
                          scrollOffset = selectedMessage;
                        } else if (selectedMessage >= scrollOffset + MAX_VISIBLE_MESSAGES) {
                          // Selected message is below visible area, scroll down
                          scrollOffset = selectedMessage - MAX_VISIBLE_MESSAGES + 1;
                        }
                        // Constrain scroll offset
                        scrollOffset = constrain(scrollOffset, 0, max(0, messageCount[selectedPot] - MAX_VISIBLE_MESSAGES));
                        Serial.print("Selected Message ");
                        Serial.println(selectedMessage);
                        Serial.print("Scroll offset: ");
                        Serial.println(scrollOffset);
                      }
                    } else {
                      // No messages, just toggle between Add and Remove
                      selectedAddRemove = (selectedAddRemove + 1) % 2;
                    }
                  }
                }
              }
              break;
          }
        } else {

          // *********************** //
          // BUTTON RELEASE CHECK
          // *********************** //

          buttonState[i] = LOW;

          // *********************** //
          // HANDLE BUTTON RELEASES
          // *********************** //
          switch (i) {
            case 0:  // ASSIGN BUTTON RELEASE
              assignButtonHeld = false;
              break;

            case 1:  // ENTER BUTTON RELEASE
              // RESET ADD/REMOVE STATE ON ENTER RELEASE
              inAddRemoveOperation = false;

              // Handle temporary override deactivation if active
              if (currentScreen == MAIN_SCREEN && tempOverrideActive) {
                deactivateTempOverride();
              }

              // Reset the press time
              enterPressTime = 0;
              break;

            case 2:  // PREV BUTTON RELEASE
              if (prevButtonPressed && enterButtonHeld && currentScreen == ASSIGN_SCREEN && !inAddRemoveOperation) {
                if (messageCount[selectedPot] > 0) {
                  selectedMessage = (selectedMessage - 1 + messageCount[selectedPot]) % messageCount[selectedPot];
                  // Update scroll offset based on new selected message
                  if (selectedMessage < scrollOffset) {
                    scrollOffset = selectedMessage;
                  } else if (selectedMessage >= scrollOffset + MAX_VISIBLE_MESSAGES) {
                    scrollOffset = selectedMessage - MAX_VISIBLE_MESSAGES + 1;
                  }
                  // Constrain scroll offset
                  scrollOffset = constrain(scrollOffset, 0, max(0, messageCount[selectedPot] - MAX_VISIBLE_MESSAGES));
                  Serial.print("Selected Message ");
                  Serial.println(selectedMessage);
                  Serial.print("Scroll offset: ");
                  Serial.println(scrollOffset);
                }
              }
              prevButtonPressed = false;
              break;

            case 3:  // NEXT BUTTON RELEASE
              if (nextButtonPressed && enterButtonHeld && currentScreen == ASSIGN_SCREEN && !inAddRemoveOperation) {
                if (messageCount[selectedPot] > 0) {
                  selectedMessage = (selectedMessage + 1) % messageCount[selectedPot];
                  // Update scroll offset based on new selected message
                  if (selectedMessage < scrollOffset) {
                    scrollOffset = selectedMessage;
                  } else if (selectedMessage >= scrollOffset + MAX_VISIBLE_MESSAGES) {
                    scrollOffset = selectedMessage - MAX_VISIBLE_MESSAGES + 1;
                  }
                  // Constrain scroll offset
                  scrollOffset = constrain(scrollOffset, 0, max(0, messageCount[selectedPot] - MAX_VISIBLE_MESSAGES));
                  Serial.print("Selected Message ");
                  Serial.println(selectedMessage);
                  Serial.print("Scroll offset: ");
                  Serial.println(scrollOffset);
                }
              }
              nextButtonPressed = false;
              break;
          }
        }
      }
    }
    lastButtonState[i] = reading;
  }

  // *********************** //
  // TEMPORARY MIDI OVERRIDE ACTIVATION
  // *********************** //
  // In MAIN_SCREEN, holding ENTER for a short time activates temporary override
  if (currentScreen == MAIN_SCREEN && buttonState[1] == HIGH && buttonState[0] == LOW && buttonState[2] == LOW && buttonState[3] == LOW) {
    if (!tempOverrideActive && enterPressTime > 0) {
      if (millis() - enterPressTime >= tempOverrideHoldTime) {
        activateTempOverride();
      }
    }
  }
}
