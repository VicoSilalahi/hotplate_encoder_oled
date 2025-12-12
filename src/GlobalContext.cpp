#include "GlobalContext.h"
#include <Wire.h>

// --- Object Instantiation ---
Adafruit_SSD1306 g_oledDisplay(128, 64, &Wire, -1);
ESP32Encoder g_rotaryEncoder;

// --- UI Navigation State ---
SystemState g_currentState = STATE_MAIN_MENU;
int g_menuCursorIndex = 0;
int g_listScrollIndex = 0;
int g_editStepIndex = 0;
int g_editParameterIndex = 0;

// --- Hardware Input State ---
// Note: g_encoderPosition is removed; we use g_rotaryEncoder.getCount()
long g_lastEncoderPosition = 0;
unsigned long g_lastButtonPressTime = 0;

// --- Reflow Profile Data ---
std::vector<ReflowStep> g_manualProfileSteps;
std::vector<ReflowStep> g_activeProfileSteps;

// --- Process Logic State ---
bool g_isManualSource = false;
bool g_autoAdvanceSteps = true;
int g_currentStepIndex = 0;
unsigned long g_stepStartTime = 0;
float g_stepStartTemperature = 0.0f;
float g_currentSetpoint = 0.0f;
float g_currentTemperature = TEMP_AMBIENT;
bool g_heaterState = false;

// --- Helper Functions ---
void Global_AddManualStep() {
    if (g_manualProfileSteps.size() < 8) {
        g_manualProfileSteps.push_back({150.0f, 1.0f});
    }
}

void Global_RemoveManualStep() {
    if (g_manualProfileSteps.size() > 1) {
        g_manualProfileSteps.pop_back();
    }
}