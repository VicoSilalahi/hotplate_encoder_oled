#pragma once
#include <vector>

// --- System State Enum ---
enum SystemState {
    STATE_MAIN_MENU,
    STATE_MANUAL_CONFIG,    // List of steps
    STATE_EDIT_STEP,        // Edit Target/Slope/Hold
    STATE_SELECT_RUN_MODE,  // Choose Auto vs Click-to-Next
    STATE_RUNNING_RAMP,     // Actively changing temp
    STATE_RUNNING_HOLD,     // Holding at target temp for duration
    STATE_RUNNING_WAIT,     // Reached target/hold, waiting for user input
    STATE_COOLDOWN          // Post-run cooldown phase
};

// --- Data Structures ---
struct ReflowStep {
    float targetTemperature;
    float rampRate;    // Degrees Celsius per SECOND
    int holdSeconds;   // Time to stay at target temperature
};