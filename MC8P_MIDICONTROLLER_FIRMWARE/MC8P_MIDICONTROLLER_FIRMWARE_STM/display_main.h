// =============================================================================
// display_main.h — main screen in the three themes (MkII, MkI, YX)
//
// drawMainScreen() picks the theme (displayTheme). YX also shows a scene
// change overlay for sceneChangeOverlayDuration.
// =============================================================================
#pragma once
#include "hardware.h"
#include "state.h"
#include "prototypes.h"
#include "display_assets.h"

// *********************** //
// DISPLAY FUNCTIONS
// *********************** //
// *********************** //
// DRAW MAIN_SCREEN
// *********************** //
void drawMainScreen() {
  if (displayTheme == 1) {
    drawMainScreenMkI();  // MkI theme (circles)
  } else if (displayTheme == 2) {
    drawMainScreenYX();  // YX theme (rounded rectangles)
  } else {
    drawMainScreenMkII();  // MkII theme (default, also handles future themes)
  }
}

// *********************** //
// DRAW MAIN_SCREEN MKII
// *********************** //
void drawMainScreenMkII() {
  display.clearDisplay();

  // Set colors based on the inversion variable
  int bgColor = displayInverted ? 0 : 1;   // Black if inverted, White if normal
  int txtColor = displayInverted ? 1 : 0;  // White if inverted, Black if normal

  display.fillRect(0, 0, 128, 64, bgColor);

  // ============================================
  // OUTER UI BORDER - Modified height for new design
  // ============================================
  display.drawRoundRect(1, 1, 126, 47, 3, txtColor);

  // ============================================
  // DRAW VERTICAL POT BARS (Bottom to top)
  // Uses the stored MIDI value (0-127) via getBarHeightFromMidi()
  // ============================================
  for (int i = 0; i < N_POTS; i++) {
    int height = getBarHeightFromMidi(i);
    int xPos;

    // Position bars horizontally (optimized layout for 8 pots)
    if (i < 4) {
      xPos = 5 + (i * 15);
    } else {
      xPos = 66 + ((i - 4) * 15);
    }

    display.fillRoundRect(xPos, 44 - height, 12, height, 1, txtColor);
  }

  // ============================================
  // SCENE LABEL AND VALUE
  // ============================================
  display.setFont(&Picopixel);
  display.setTextColor(txtColor);
  display.setTextSize(2);
  display.setCursor(25, 59);
  display.print("Scene");

  int displaySceneValue;
  if (sceneMode == STEP_MODE) {
    displaySceneValue = currentSceneValue + 1;
  } else {
    displaySceneValue = (currentSceneValue >= 0 && currentSceneValue < customSceneLength)
                          ? customSceneOrder[currentSceneValue] + 1
                          : 1;
  }

  String sceneStr = (displaySceneValue < 10) ? "0" + String(displaySceneValue) : String(displaySceneValue);
  display.setTextColor(txtColor);
  display.setCursor(87, 59);
  display.print(sceneStr);

  display.display();
}

// *********************** //
// BAR HEIGHT HELPER (for MkII theme) — 3..38 range
// *********************** //
int getBarHeightFromMidi(int potIndex) {
  int midiValue;

  if (tempOverrideActive) {
    // During temp override: show the temp-mapped value
    midiValue = mapMidiValueWithParams(tempMidiValues[potIndex],
                                       potMessages[potIndex][0].minValue,
                                       potMessages[potIndex][0].maxValue,
                                       potMessages[potIndex][0].curve);
  } else {
    // Normal & catch-up: show the stored MIDI value (what was actually sent)
    midiValue = (messageCount[potIndex] > 0) ? potMessages[potIndex][0].value
                                             : currentMidiValue[potIndex];
  }

  float normalized = (float)midiValue / 127.0;

  CurveType curve = CURVE_NOR;
  if (messageCount[potIndex] > 0) {
    curve = potMessages[potIndex][0].curve;
  }

  float transformed;
  switch (curve) {
    case CURVE_LOG: transformed = normalized * normalized; break;
    case CURVE_ILG: transformed = 1.0 - ((1.0 - normalized) * (1.0 - normalized)); break;
    case CURVE_INV: transformed = normalized; break;
    default: transformed = normalized; break;
  }

  if (transformed <= 0.01) return 3;
  if (transformed >= 0.99) return 38;
  return 3 + (int)(transformed * 35);
}

// *********************** //
// DRAW MAIN_SCREEN MKI (ORIGINAL DISPLAY)
// *********************** //
void drawMainScreenMkI() {
  display.clearDisplay();

  // Set colors based on the inversion variable
  int bgColor = displayInverted ? 0 : 1;   // Black if inverted, White if normal
  int txtColor = displayInverted ? 1 : 0;  // White if inverted, Black if normal

  display.fillRect(0, 0, 128, 64, bgColor);

  // UI
  display.drawRoundRect(1, 1, 126, 62, 3, txtColor);
  display.drawBitmap(2, 2, image_mainScreenInnerLines_bits, 124, 60, txtColor);

  // POT INFO
  display.setTextColor(txtColor);
  display.setTextWrap(false);

  // Draw all pot displays
  for (int i = 0; i < N_POTS; i++) {
    drawPotDisplay(i);
  }

  // SCENE/PROJECT NUMBER - Display current scene
  display.setFont(&Picopixel);
  display.setCursor(61, 33);

  int displaySceneValue;
  if (sceneMode == STEP_MODE) {
    displaySceneValue = currentSceneValue + 1;
  } else {
    if (currentSceneValue >= 0 && currentSceneValue < customSceneLength) {
      displaySceneValue = customSceneOrder[currentSceneValue] + 1;
    } else {
      displaySceneValue = 1;
    }
  }

  // Format scene number with leading zero if needed
  if (displaySceneValue < 10) {
    display.print("0");
  }
  display.print(displaySceneValue);

  display.display();
}

// *********************** //
// DRAW POTENTIOMETERS ON MAIN_SCREEN (MkI circles)
// *********************** //
void drawPotDisplay(int potIndex) {
  const int maxRadius = 5;
  const PotDisplay& disp = potDisplays[potIndex];

  // Set colors based on the inversion variable
  int fgColor = displayInverted ? 1 : 0;  // Foreground color (circles, text) - opposite of bg
  int bgColor = displayInverted ? 0 : 1;  // Background color (fill)

  // Determine which raw MIDI value to display (0-127, no min/max scaling)
  int displayValue;

  if (tempOverrideActive) {
    // During temporary override, map raw pot position to raw MIDI (0-127)
    displayValue = map(tempMidiValues[potIndex], 0, 1023, 0, 127);
  } else if (catchUpActive[potIndex]) {
    // During catch-up, calculate the raw MIDI value
    int potMovement = potState[potIndex] - catchUpStartPotPos[potIndex];
    if (abs(potMovement) > 20) {
      if (potMovement > 0) {
        displayValue = map(potState[potIndex], catchUpStartPotPos[potIndex], 1023, 0, 127);
      } else {
        displayValue = map(potState[potIndex], catchUpStartPotPos[potIndex], 0, 0, 127);
      }
      displayValue = constrain(displayValue, 0, 127);
    } else {
      // Still in dead zone, show stored raw MIDI
      displayValue = currentMidiValue[potIndex];
    }
  } else {
    // Normal mode - show raw MIDI based on pot position
    displayValue = currentMidiValue[potIndex];
  }

  // CALCULATE CURRENT RADIUS BASED ON RAW MIDI VALUE (0-127)
  int currentRadius = map(displayValue, 0, 127, 0, maxRadius);

  // DRAW EXPANDING INNER CIRCLE TO REPRESENT MIDI VALUE
  if (currentRadius > 0) {
    display.fillCircle(disp.circleX, (disp.circleY - 1), currentRadius, fgColor);
  }

  // DRAW OUTER CIRCLES
  display.drawCircle(disp.circleX, (disp.circleY - 1), 7, fgColor);

  // DISPLAY MIDI VALUE
  // CENTERED BELOW THE CIRCLES
  String valueStr = String(displayValue);

  // CALCULATE TEXT WIDTH TO CENTRE TEXT
  int16_t x1, y1;
  uint16_t textWidth, textHeight;
  display.getTextBounds(valueStr, 0, 0, &x1, &y1, &textWidth, &textHeight);

  // CENTRE TEXT HORIZONTALLY
  int textX = disp.circleX - (textWidth / 2);

  display.setFont(NULL);
  display.setTextColor(fgColor);
  display.setCursor(textX, (disp.textY - 1));
  display.print(valueStr);
}

// *********************** //
// DRAW MAIN_SCREEN YX     //
// *********************** //
void drawMainScreenYX() {
  display.clearDisplay();

  // Set colors based on the inversion variable
  int bgColor = displayInverted ? 0 : 1;
  int txtColor = displayInverted ? 1 : 0;

  // Fill the whole screen with background
  display.fillRect(0, 0, 128, 64, bgColor);

  // ============================================================
  // 2 rows of 4 rounded rectangles.
  // Row 1 (top):    Y = 1,  height 30, bottom edge at Y=31
  // Row 2 (bottom): Y = 33, height 29, bottom edge at Y=62
  // Columns:        X = 1, 33, 65, 96  (1px gap between)
  // Widths:         30, 30, 30, 31     (fills to X=127)
  // ============================================================

  const int colX[4] = { 1, 33, 65, 96 };
  const int colW[4] = { 30, 30, 30, 31 };
  const int rowY[2] = { 1, 33 };
  const int rowH[2] = { 30, 29 };
  const int rowBottom[2] = { 31, 62 };  // bottom edge Y for each row

  // Map pot index → (row, col):
  //   Pots 0..3 → top row, columns 0..3
  //   Pots 4..7 → bottom row, columns 0..3
  for (int i = 0; i < N_POTS; i++) {
    int row = (i < 4) ? 0 : 1;
    int col = i % 4;

    // Get the bar height (3..rowH[row] range, using the existing helper)
    int barHeight = getBarHeightFromMidiYX(i);

    // Clamp to row height
    if (barHeight > rowH[row]) barHeight = rowH[row];
    if (barHeight < 1) barHeight = 1;

    // Rect grows from the bottom of its cell upward
    int x = colX[col];
    int w = colW[col];
    int y = rowBottom[row] - barHeight;
    int h = barHeight;

    // Draw the filled rounded rect (radius 3, but clamp radius for small heights)
    int r = 3;
    if (r > w / 2) r = w / 2;
    if (r > h / 2) r = h / 2;

    display.fillRoundRect(x, y, w, h, r, txtColor);
  }

  // ============================================================
  // SCENE CHANGE OVERLAY (temporary, middle of screen)
  // ============================================================
  if (sceneChangeOverlayActive) {
    // Auto-expire the overlay
    if (millis() - sceneChangeOverlayStart >= sceneChangeOverlayDuration) {
      sceneChangeOverlayActive = false;
    } else {
      // Compute current scene value (1-based, leading zero for single digits)
      int displaySceneValue;
      if (sceneMode == STEP_MODE) {
        displaySceneValue = currentSceneValue + 1;
      } else {
        displaySceneValue = (currentSceneValue >= 0 && currentSceneValue < customSceneLength)
                              ? customSceneOrder[currentSceneValue] + 1
                              : 1;
      }
      String sceneStr = (displaySceneValue < 10)
                          ? "0" + String(displaySceneValue)
                          : String(displaySceneValue);

      // Box dimensions
      const int boxW = 40;
      const int boxH = 20;
      const int boxX = (SCREEN_WIDTH - boxW) / 2;   // centered: 44
      const int boxY = (SCREEN_HEIGHT - boxH) / 2;  // centered: 22

      // Draw filled box (opposite of background so it stands out)
      display.fillRoundRect(boxX, boxY, boxW, boxH, 4, txtColor);
      display.drawRoundRect(boxX - 1, boxY - 1, boxW + 2, boxH + 2, 4, txtColor);

      // Draw the scene number centered in the box, in bg color
      display.setFont(&Picopixel);
      display.setTextSize(2);
      display.setTextColor(bgColor);
      int16_t tx1, ty1;
      uint16_t tw, th;
      display.getTextBounds(sceneStr, 0, 0, &tx1, &ty1, &tw, &th);
      int textX = boxX + (boxW - tw) / 2;
      int textY = boxY + (boxH + th) / 2 - 2;
      display.setCursor(textX, textY);
      display.print(sceneStr);

      // Restore text settings for the next frame
      display.setTextSize(1);
      display.setTextColor(txtColor);
    }
  }

  display.display();
}

// *********************** //
// DRAW OVERLAY FOR SCENES //
// *********************** //
void triggerSceneOverlay() {
  sceneChangeOverlayActive = true;
  sceneChangeOverlayStart = millis();
}

// *********************** //
// BAR HEIGHT HELPER (for YX theme) — 3..30 range
// *********************** //
int getBarHeightFromMidiYX(int potIndex) {
  int midiValue;

  if (tempOverrideActive) {
    midiValue = mapMidiValueWithParams(tempMidiValues[potIndex],
                                       potMessages[potIndex][0].minValue,
                                       potMessages[potIndex][0].maxValue,
                                       potMessages[potIndex][0].curve);
  } else {
    midiValue = (messageCount[potIndex] > 0) ? potMessages[potIndex][0].value
                                             : currentMidiValue[potIndex];
  }

  float normalized = (float)midiValue / 127.0;

  CurveType curve = CURVE_NOR;
  if (messageCount[potIndex] > 0) {
    curve = potMessages[potIndex][0].curve;
  }

  float transformed;
  switch (curve) {
    case CURVE_LOG: transformed = normalized * normalized; break;
    case CURVE_ILG: transformed = 1.0 - ((1.0 - normalized) * (1.0 - normalized)); break;
    case CURVE_INV: transformed = normalized; break;
    default: transformed = normalized; break;
  }

  if (transformed <= 0.01) return 3;
  if (transformed >= 0.99) return 30;
  return 3 + (int)(transformed * 27);
}

// *********************** //
