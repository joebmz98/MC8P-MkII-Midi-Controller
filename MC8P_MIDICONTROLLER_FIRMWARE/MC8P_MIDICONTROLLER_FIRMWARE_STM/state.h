// =============================================================================
// state.h — shared types and every global the modules read and write
//
// The firmware is one Arduino sketch, so all of this lives in one translation
// unit: the module headers are only ever included by the .ino. Grouped by
// what uses it: pots and their MIDI messages, screen / menu navigation, the
// temporary override, scenes, and the display settings.
// =============================================================================
#pragma once
#include "hardware.h"

// -- FIRMWARE VERSION --
const char* FIRMWARE_VERSION = "3.2";

// CC MESSAGE DIRECTION/CURVE
enum CurveType {
  CURVE_NOR = 0,  // Normal linear (0-127)
  CURVE_INV = 1,  // Inverted linear (127-0)
  CURVE_LOG = 2,  // Logarithmic (slow start, fast end)
  CURVE_ILG = 3   // Inverse Logarithmic (fast start, slow end)
};

// STRUCTURE TO HOLD MULTIPLE MIDI MESSAGES PER POT
struct MidiMessageParams {
  byte channel;
  byte cc;
  CurveType curve;  // 0 = NOR // 1 = INV // 2 = LOG // 3 = ILG
  byte minValue;    // Minimum value for CC range (0-127)
  byte maxValue;    // Maximum value for CC range (0-127)
  uint16_t value;   // Current stored value
};

// -- BUTTON STATE --
int buttonState[NUM_BUTTONS] = { 0 };
int lastButtonState[NUM_BUTTONS] = { 0 };
unsigned long lastDebounceTime[NUM_BUTTONS] = { 0 };
const unsigned long debounceDelay = 15;  // BUTTON DEBOUNCE

// SCREEN STATE MANAGEMENT
enum ScreenState { MAIN_SCREEN,
                   MENU_SCREEN,
                   ASSIGN_SCREEN,
                   SETTINGS_SCREEN,
                   ASSIGN_SETTINGS_SCREEN,
                   DISPLAY_SETTINGS_SCREEN,
                   ABOUT_SCREEN,
                   RESET_SCREEN,
                   CONFIRM_ASSIGN_SAVE_SCREEN,
                   CONFIRM_RESET_SCREEN,
                   SCENE_SCREEN };
ScreenState currentScreen = MAIN_SCREEN;

// ASSIGN SCREEN NAVIGATION STATES
enum AssignEditMode {
  ASSIGN_POT_SELECT,      // Selecting which pot to edit
  ASSIGN_MESSAGE_SELECT,  // Selecting which message to edit
  ASSIGN_ADD_REMOVE,      // Selecting Add or Remove button
  ASSIGN_EDIT_CHANNEL,    // Editing channel
  ASSIGN_EDIT_CC,         // Editing CC number
  ASSIGN_EDIT_INVERT,     // Editing invert setting
  ASSIGN_EDIT_MIN,        // Editing min value
  ASSIGN_EDIT_MAX         // Editing max value
};

AssignEditMode assignEditMode = ASSIGN_POT_SELECT;
bool assignEditingMode = false;  // True when editing values, false when selecting

// ASSIGN SETTINGS
int editPotSelection = 7;           // Default to pot 8 (index 7)
bool editingAssignSetting = false;  // Flag for edit mode

// DISPLAY SETTINGS
#define NUM_THEMES 3                 // 0 = MkII, 1 = MkI, 2 = YX
bool displayInverted = true;         // Default: Yes (inverted)
bool editingDisplaySetting = false;  // Flag to indicate we're editing the value
unsigned long editFlashTimer = 0;    // Timer for flashing effect
bool flashState = false;             // Current flash state
bool originalDisplayInverted;        // Store original value when entering edit mode
uint8_t displayTheme = 0;            // 0 = MkII, 1 = MkI, 2+ = future themes
bool editingDisplayTheme = false;    // Flag for editing theme
uint8_t originalDisplayTheme;        // Store original theme when editing


// MENU NAVIGATION
uint8_t selectedMenuItem = 0;  // 0: Assign, 1: States, 2: Settings

// BUTTON STATE MANAGEMENT
// ASSIGN
bool assignButtonHeld = false;
// PREV/NEXT
bool prevButtonPressed = false;
bool nextButtonPressed = false;
// ENTER
bool enterButtonHeld = false;

// POTENTIOMETER CONFIG
const uint8_t N_POTS = 8;  // Eight potentiometers

// MIDI CONFIG
// START-UP CONFIG
byte potChannels[N_POTS] = { 0, 1, 2, 3, 4, 5, 6, 7 };  // Channels 1-8
byte potCCs[N_POTS] = { 7, 7, 7, 7, 7, 7, 7, 7 };       // CC numbers

const int MAX_MESSAGES_PER_POT = 10;                          // Maximum number of messages per pot
MidiMessageParams potMessages[N_POTS][MAX_MESSAGES_PER_POT];  // Array to store messages
byte messageCount[N_POTS] = { 0 };                            // Track how many messages each pot has

// -- ARRAYS TO STORE POT VALUES AND STATES
int potReading[N_POTS] = { 0 };
int potState[N_POTS] = { 0 };
int potPState[N_POTS] = { 0 };
int currentMidiValue[N_POTS] = { 0 };  // Store MIDI values for display

const int POT_THRESHOLD = 10;

const int POT_TIMEOUT = 300;
unsigned long pPotTime[N_POTS] = { 0 };
unsigned long potTimer[N_POTS] = { 0 };

// -- POT SELECTION AND EDITING
uint8_t selectedPot = 0;      // Default to pot 0
uint8_t selectedMessage = 0;  //

// TEMPORARY MIDI OVERRIDE SYSTEM
bool tempOverrideActive = false;                // Flag for temporary override mode
int originalMidiValues[N_POTS] = { 0 };         // Store original values when ENTER is pressed - INIT TO 0
int tempMidiValues[N_POTS] = { 0 };             // Store temporary values during override - INIT TO 0
unsigned long enterPressTime = 0;               // Time when ENTER was pressed
const unsigned long tempOverrideHoldTime = 20;  // Debounce time to activate override
int postOverridePotPositions[N_POTS] = { 0 };   // Store pot positions when temp override ends - INIT TO 0
bool catchUpActive[N_POTS] = { false };         // Flag to track if catch-up is active for each pot - INIT TO FALSE
int catchUpStartValue[N_POTS] = { 0 };          // MIDI value when catch-up starts - INIT TO 0
int catchUpStartPotPos[N_POTS] = { 0 };         // Pot position when catch-up starts - INIT TO 0

// Add this variable with your other global variables
bool confirmAssignSave = true;  // Track Y/N selection for ASSIGN save confirmation

// SCENE/PROJECT NAVIGATION FOR OP-XY
uint8_t currentSceneValue = 0;  // MIDI value 0-98 for scenes (display shows +1)
uint8_t sceneCC = 82;           // MIDI CC for scene selection (editable)
int editingSceneCC = false;     // Flag for editing scene CC in Scene Mode
uint8_t originalSceneCC = 82;   // Store original value when entering edit mode
const int SCENE_CHANNEL = 1;    // MIDI channel for scene messages (adjust as needed)
// SCENE CHANGE OVERLAY (YX theme)
bool sceneChangeOverlayActive = false;
unsigned long sceneChangeOverlayStart = 0;
const unsigned long sceneChangeOverlayDuration = 600;  // ms

// Scene Mode Navigation
enum SceneNavigationMode { STEP_MODE,
                           CUSTOM_MODE };
SceneNavigationMode sceneMode = STEP_MODE;          // Default to STEP mode
int editingSceneMode = false;                       // Flag for editing scene mode
SceneNavigationMode originalSceneMode = STEP_MODE;  // Store original mode when editing         // Store original mode when editing

// Custom scene order (max 16 scenes)
const int MAX_CUSTOM_SCENES = 12;
int customSceneOrder[MAX_CUSTOM_SCENES] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11 };  // Default sequence
uint8_t customSceneLength = 12;                                                      // Number of scenes in custom order (1-16)
int editingCustomScene = false;                                                      // Flag for editing custom scenes
uint8_t editingCustomSceneIndex = 0;                                                 // Which scene in custom order is being edited
uint8_t customSceneEditPosition = 0;                                                 // Position in custom order being edited
uint8_t originalCustomSceneValue = 0;                                                // Store original value when editing custom scene
uint8_t lastCustomSceneEditPosition = 0;                                             // Store last selected step position

// Scene screen navigation states
enum SceneScreenEditMode {
  SCENE_EDIT_NONE,
  SCENE_EDIT_CC,
  SCENE_EDIT_MODE,
  SCENE_EDIT_CUSTOM_LENGTH,
  SCENE_EDIT_CUSTOM_VALUE
};

SceneScreenEditMode sceneEditMode = SCENE_EDIT_NONE;
uint8_t sceneSelectedParameter = 0;  // 0=CC, 1=Mode, 2=Custom (if in custom mode)
bool selectingStep = false;          // True when navigating between sequence steps

// Checksums of what is in flash, so GET_STATUS can say "unsaved"
uint16_t savedMidiSum = 0;
uint16_t savedGlobalSum = 0;

// SCROLLING CONFIG
const int MAX_VISIBLE_MESSAGES = 3;  // Number of messages visible at once
uint8_t scrollOffset = 0;            // Current scroll position

// ASSIGN PARAMETER EDITING
uint16_t selectedAddRemove = 0;  // 0 = Add, 1 = Remove
bool confirmSelection = true;    // Track Y/N selection (true = Yes, false = No)

int lastPot8Value = -1;
unsigned long lastPot8ChangeTime = 0;
