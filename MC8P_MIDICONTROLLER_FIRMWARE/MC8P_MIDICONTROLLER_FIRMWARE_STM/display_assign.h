// =============================================================================
// display_assign.h — the ASSIGN screen (per-pot message list and editing)
// =============================================================================
#pragma once
#include "hardware.h"
#include "state.h"
#include "prototypes.h"
#include "display_assets.h"

// DRAW ASSIGN_SCREEN
// *********************** //
void drawAssignScreen() {
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
  display.setCursor(6, 11);
  display.print("MIDI assign");

  // DISPLAY CURRENTLY SELECTED POTENTIOMETER WITH SELECTION BOX
  display.setCursor(77, 11);
  display.print("Pot ");

  // Draw selection box around pot number if in pot select mode
  if (assignEditMode == ASSIGN_POT_SELECT && !assignEditingMode) {
    // Flash the selection box
    if ((millis() / 300) % 2 == 0) {
      display.fillRoundRect(90, 5, 7, 10, 1, txtColor);
      display.setTextColor(bgColor);
      display.setCursor(92, 11);
      display.print(selectedPot + 1);
      display.setTextColor(txtColor);
    } else {
      display.drawRoundRect(90, 5, 7, 10, 1, txtColor);
      display.setCursor(92, 11);
      display.print(selectedPot + 1);
    }
  } else {
    display.setCursor(92, 11);
    display.print(selectedPot + 1);
  }

  // Ensure scroll offset is within valid range
  int maxOffset = max(0, messageCount[selectedPot] - MAX_VISIBLE_MESSAGES);
  scrollOffset = constrain(scrollOffset, 0, maxOffset);

  // Also ensure selected message is visible by adjusting scroll offset if needed
  if (selectedMessage < scrollOffset) {
    scrollOffset = selectedMessage;
  } else if (selectedMessage >= scrollOffset + MAX_VISIBLE_MESSAGES) {
    scrollOffset = selectedMessage - MAX_VISIBLE_MESSAGES + 1;
  }

  // Re-constrain after adjustments
  scrollOffset = constrain(scrollOffset, 0, maxOffset);

  // DISPLAY MIDI MESSAGES UP TO MAX_VISIBLE_MESSAGES
  for (int i = 0; i < MAX_VISIBLE_MESSAGES; i++) {
    int messageIndex = i + scrollOffset;
    if (messageIndex >= messageCount[selectedPot]) {
      break;
    }

    int yPos = 26 + (i * 10);

    // Only display if it won't overlap with Add/Remove buttons
    if (yPos <= 48) {
      // Draw the bar indicator for the selected message
      if (assignEditMode == ASSIGN_MESSAGE_SELECT && !assignEditingMode && messageIndex == selectedMessage && selectedAddRemove == 2) {
        // Draw the bar with flashing effect for message selection
        if ((millis() / 400) % 2 == 0) {
          display.setCursor(4, yPos);
          display.setTextColor(txtColor);
          display.print("[                                                      ]");
        }
      }

      display.setCursor(8, yPos);

      // DISPLAY MIDI MESSAGES
      // FORMAT: Ch X | CC X | 0-127 | NOR
      display.print("Ch ");

      // Handle OMNI option (channel 17 = OMNI)
      if (potMessages[selectedPot][messageIndex].channel == 16) {  // 16 = OMNI in 0-based index
        display.print("OMNI");
      } else {
        display.print(potMessages[selectedPot][messageIndex].channel + 1);
      }

      display.print(" | CC ");
      display.print(potMessages[selectedPot][messageIndex].cc);
      display.print(" | ");
      display.print(potMessages[selectedPot][messageIndex].minValue);
      display.print("-");
      display.print(potMessages[selectedPot][messageIndex].maxValue);
      display.print(" | ");

      // Show curve type (NOR, INV, LOG, ILG)
      switch (potMessages[selectedPot][messageIndex].curve) {
        case CURVE_NOR:
          display.print("NOR");
          break;
        case CURVE_INV:
          display.print("INV");
          break;
        case CURVE_LOG:
          display.print("LOG");
          break;
        case CURVE_ILG:
          display.print("ILG");
          break;
      }

      // Show parameter edit indicator for selected message when in edit mode
      if (messageIndex == selectedMessage && assignEditingMode) {
        // Calculate the X position and width for each parameter
        int editX = 8;  // Start after the cursor position
        int parameterWidth = 0;

        // First, we need to calculate the X position by measuring the text before the parameter
        switch (assignEditMode) {
          case ASSIGN_EDIT_CHANNEL:
            {
              // Measure "Ch " prefix
              String prefix = "Ch ";
              int16_t x1, y1;
              uint16_t w, h;
              display.getTextBounds(prefix, 0, 0, &x1, &y1, &w, &h);
              editX = 8 + w;

              // Get the channel string to measure its width
              String channelStr;
              if (potMessages[selectedPot][selectedMessage].channel == 16) {
                channelStr = "OMNI";
              } else {
                channelStr = String(potMessages[selectedPot][selectedMessage].channel + 1);
              }
              display.getTextBounds(channelStr, 0, 0, &x1, &y1, &w, &h);
              parameterWidth = w;
              break;
            }
          case ASSIGN_EDIT_CC:
            {
              // Measure "Ch X | CC " prefix
              String channelStr;
              if (potMessages[selectedPot][selectedMessage].channel == 16) {
                channelStr = "OMNI";
              } else {
                channelStr = String(potMessages[selectedPot][selectedMessage].channel + 1);
              }
              String prefix = "Ch " + channelStr + " | CC ";
              int16_t x1, y1;
              uint16_t w, h;
              display.getTextBounds(prefix, 0, 0, &x1, &y1, &w, &h);
              editX = 8 + w;

              // Get the CC value string width
              String ccStr = String(potMessages[selectedPot][selectedMessage].cc);
              display.getTextBounds(ccStr, 0, 0, &x1, &y1, &w, &h);
              parameterWidth = w;
              break;
            }
          case ASSIGN_EDIT_MIN:
            {
              // Measure up to the min value
              String channelStr;
              if (potMessages[selectedPot][selectedMessage].channel == 16) {
                channelStr = "OMNI";
              } else {
                channelStr = String(potMessages[selectedPot][selectedMessage].channel + 1);
              }
              String prefix = "Ch " + channelStr + " | CC " + String(potMessages[selectedPot][selectedMessage].cc) + " | ";
              int16_t x1, y1;
              uint16_t w, h;
              display.getTextBounds(prefix, 0, 0, &x1, &y1, &w, &h);
              editX = 8 + w;

              // Get the min value string width
              String minStr = String(potMessages[selectedPot][selectedMessage].minValue);
              display.getTextBounds(minStr, 0, 0, &x1, &y1, &w, &h);
              parameterWidth = w;
              break;
            }
          case ASSIGN_EDIT_MAX:
            {
              // Measure up to the max value (including the dash)
              String channelStr;
              if (potMessages[selectedPot][selectedMessage].channel == 16) {
                channelStr = "OMNI";
              } else {
                channelStr = String(potMessages[selectedPot][selectedMessage].channel + 1);
              }
              String prefix = "Ch " + channelStr + " | CC " + String(potMessages[selectedPot][selectedMessage].cc) + " | " + String(potMessages[selectedPot][selectedMessage].minValue) + "-";
              int16_t x1, y1;
              uint16_t w, h;
              display.getTextBounds(prefix, 0, 0, &x1, &y1, &w, &h);
              editX = 8 + w;

              // Get the max value string width
              String maxStr = String(potMessages[selectedPot][selectedMessage].maxValue);
              display.getTextBounds(maxStr, 0, 0, &x1, &y1, &w, &h);
              parameterWidth = w;
              break;
            }
          case ASSIGN_EDIT_INVERT:
            {
              // Measure up to the direction (including the " | " before it)
              String channelStr;
              if (potMessages[selectedPot][selectedMessage].channel == 16) {
                channelStr = "OMNI";
              } else {
                channelStr = String(potMessages[selectedPot][selectedMessage].channel + 1);
              }
              String prefix = "Ch " + channelStr + " | CC " + String(potMessages[selectedPot][selectedMessage].cc) + " | " + String(potMessages[selectedPot][selectedMessage].minValue) + "-" + String(potMessages[selectedPot][selectedMessage].maxValue) + " | ";
              int16_t x1, y1;
              uint16_t w, h;
              display.getTextBounds(prefix, 0, 0, &x1, &y1, &w, &h);
              editX = 8 + w;

              // Get the curve string width
              String curveStr;
              switch (potMessages[selectedPot][selectedMessage].curve) {
                case CURVE_NOR: curveStr = "NOR"; break;
                case CURVE_INV: curveStr = "INV"; break;
                case CURVE_LOG: curveStr = "LOG"; break;
                case CURVE_ILG: curveStr = "ILG"; break;
              }
              display.getTextBounds(curveStr, 0, 0, &x1, &y1, &w, &h);
              parameterWidth = w;
              break;
            }
        }

        // Flash the parameter being edited
        if (editX > 0 && (millis() / 200) % 2 == 0) {
          display.fillRect(editX, yPos - 5, parameterWidth + 2, 7, txtColor);
          display.setTextColor(bgColor);
          // Redraw the parameter with highlighted background
          display.setCursor(editX + 1, yPos);

          // Redraw just the parameter being edited
          switch (assignEditMode) {
            case ASSIGN_EDIT_CHANNEL:
              if (potMessages[selectedPot][selectedMessage].channel == 16) {
                display.print("OMNI");
              } else {
                display.print(potMessages[selectedPot][selectedMessage].channel + 1);
              }
              break;
            case ASSIGN_EDIT_CC:
              display.print(potMessages[selectedPot][selectedMessage].cc);
              break;
            case ASSIGN_EDIT_MIN:
              display.print(potMessages[selectedPot][selectedMessage].minValue);
              break;
            case ASSIGN_EDIT_MAX:
              display.print(potMessages[selectedPot][selectedMessage].maxValue);
              break;
            case ASSIGN_EDIT_INVERT:
              switch (potMessages[selectedPot][selectedMessage].curve) {
                case CURVE_NOR: display.print("NOR"); break;
                case CURVE_INV: display.print("INV"); break;
                case CURVE_LOG: display.print("LOG"); break;
                case CURVE_ILG: display.print("ILG"); break;
              }
              break;
          }
          display.setTextColor(txtColor);
        }
      }
    }
  }

  // Draw Add/Remove buttons at the bottom
  int buttonYPos = 56;

  // Get text widths for positioning
  int16_t x1, y1;
  uint16_t w, h, w2, h2;
  display.getTextBounds("+ Add", 0, 0, &x1, &y1, &w, &h);
  display.getTextBounds("- Remove", 0, 0, &x1, &y1, &w2, &h2);

  // Draw selection indicators for Add/Remove buttons using text brackets
  if ((assignEditMode == ASSIGN_MESSAGE_SELECT || assignEditMode == ASSIGN_ADD_REMOVE) && !assignEditingMode) {
    // Draw Add button with brackets if selected
    if (selectedAddRemove == 0) {
      // Position for Add button brackets
      int addBracketStart = 4;
      int addButtonX = 8;
      int addBracketEnd = addButtonX + w + 2;

      // Draw Add button with brackets that flash
      if ((millis() / 400) % 2 == 0) {
        // Flash state - draw brackets (visible)
        display.setCursor(addBracketStart, buttonYPos);
        display.print("[");
        display.setCursor(addButtonX, buttonYPos);
        display.print("+ Add");
        display.setCursor(addBracketEnd, buttonYPos);
        display.print("]");
      } else {
        // Non-flash state - draw brackets as spaces (invisible)
        display.setCursor(addBracketStart, buttonYPos);
        display.print(" ");
        display.setCursor(addButtonX, buttonYPos);
        display.print("+ Add");
        display.setCursor(addBracketEnd, buttonYPos);
        display.print(" ");
      }

      // Draw Remove button normally
      display.setCursor(8 + w + 10, buttonYPos);
      display.print("- Remove");

    }
    // Draw Remove button with brackets if selected
    else if (selectedAddRemove == 1) {
      int removeX = 8 + w + 10;
      int removeBracketStart = removeX - 4;
      int removeBracketEnd = removeX + w2 + 2;

      // Draw Add button normally
      display.setCursor(8, buttonYPos);
      display.print("+ Add");

      // Draw Remove button with brackets that flash
      if ((millis() / 400) % 2 == 0) {
        // Flash state - draw brackets (visible)
        display.setCursor(removeBracketStart, buttonYPos);
        display.print("[");
        display.setCursor(removeX, buttonYPos);
        display.print("- Remove");
        display.setCursor(removeBracketEnd, buttonYPos);
        display.print("]");
      } else {
        // Non-flash state - draw brackets as spaces (invisible)
        display.setCursor(removeBracketStart, buttonYPos);
        display.print(" ");
        display.setCursor(removeX, buttonYPos);
        display.print("- Remove");
        display.setCursor(removeBracketEnd, buttonYPos);
        display.print(" ");
      }

    } else {
      // Neither selected - normal display without brackets
      display.setCursor(8, buttonYPos);
      display.print("+ Add");
      display.setCursor(8 + w + 10, buttonYPos);
      display.print("- Remove");
    }
  } else {
    // Normal display without brackets
    display.setCursor(8, buttonYPos);
    display.print("+ Add");
    display.setCursor(8 + w + 10, buttonYPos);
    display.print("- Remove");
  }

  // SCROLL BAR - Show when there are 4 or more messages
  if (messageCount[selectedPot] >= MAX_VISIBLE_MESSAGES) {
    display.drawLine(121, 20, 121, 59, txtColor);
    int scrollBarHeight = 59 - 21;
    int thumbHeight = max(5, scrollBarHeight * MAX_VISIBLE_MESSAGES / messageCount[selectedPot]);
    int thumbPosition = 21 + (scrollOffset * (scrollBarHeight - thumbHeight) / max(1, (messageCount[selectedPot] - MAX_VISIBLE_MESSAGES)));
    display.fillRoundRect(119, thumbPosition, 5, thumbHeight, 2, txtColor);
    display.fillRect(120, (thumbPosition + 1), 3, (thumbHeight - 2), bgColor);
  }

  // POT INDICATOR GRID
  display.drawBitmap(105, 8, image_potMatrixGrid_bits, 10, 4, txtColor);
  display.drawRoundRect(101, 5, 18, 10, 1, txtColor);
  display.drawCircle(potCirclePositions[selectedPot].x + 3, potCirclePositions[selectedPot].y, 1, txtColor);

  display.display();
}
