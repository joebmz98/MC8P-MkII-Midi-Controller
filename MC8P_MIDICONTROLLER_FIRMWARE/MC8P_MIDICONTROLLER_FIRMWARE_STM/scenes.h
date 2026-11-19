// =============================================================================
// scenes.h — scene / project navigation (OP-XY): STEP and CUSTOM order modes
//
// Scenes go out as sceneCC on SCENE_CHANNEL, value 0-98 (shown as 1-99).
// =============================================================================
#pragma once
#include "hardware.h"
#include "state.h"
#include "prototypes.h"
#include "midi_out.h"

// *********************** //
// GET SCENE VALUE
// *********************** //
int getCurrentSceneMidiValue() {
  if (sceneMode == STEP_MODE) {
    return currentSceneValue;
  } else {
    // CUSTOM_MODE - return the custom scene value at current position
    if (currentSceneValue >= 0 && currentSceneValue < customSceneLength) {
      return customSceneOrder[currentSceneValue];
    }
    return 0;
  }
}

// *********************** //
// SET SCENE VALUE
// *********************** //
void setCurrentScene(int newValue) {
  if (sceneMode == STEP_MODE) {
    currentSceneValue = constrain(newValue, 0, 98);
    sendControlChange(sceneCC, currentSceneValue, SCENE_CHANNEL);
  } else {
    // In custom mode, find the position of the scene value in custom order
    for (int i = 0; i < customSceneLength; i++) {
      if (customSceneOrder[i] == newValue) {
        currentSceneValue = i;
        sendControlChange(sceneCC, customSceneOrder[currentSceneValue], SCENE_CHANNEL);
        return;
      }
    }
    // If scene not found in custom order, don't change
  }
}
