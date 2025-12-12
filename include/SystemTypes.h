#pragma once
#include <vector>

// --- System State Enum ---
enum SystemState {
    STATE_MAIN_MENU,
    STATE_MANUAL_CONFIG,    // List of steps
    STATE_EDIT_STEP,        // Edit Target/Slope
    STATE_SELECT_RUN_MODE,  // New: Choose Auto vs Click-to-Next
    STATE_RUNNING_RAMP,     // Actively changing temp
    STATE_RUNNING_WAIT,     // Reached target, waiting for user
    STATE_COOLDOWN          // Post-run
};

// --- Data Structures ---
struct ReflowStep {
    float targetTemperature;
    float rampRate; // Degrees Celsius per SECOND
};