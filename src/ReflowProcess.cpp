#include "ReflowProcess.h"
#include "GlobalContext.h"

void Process_Init() {
    pinMode(PIN_HEATER_LED, OUTPUT);
    // Initialize Default Manual Profile (2 Steps)
    g_manualProfileSteps.push_back({100.0f, 1.0f});
    g_manualProfileSteps.push_back({180.0f, 2.0f});
}

void Process_Start(bool useManualSource, bool autoAdvance) {
    g_isManualSource = useManualSource;
    g_autoAdvanceSteps = autoAdvance;

    if (g_isManualSource) {
        g_activeProfileSteps = g_manualProfileSteps;
    } else {
        // Load dummy preset if selected (simplified for now)
        g_activeProfileSteps.clear();
        g_activeProfileSteps.push_back({150.0f, 1.5f});
        g_activeProfileSteps.push_back({250.0f, 2.0f});
        g_activeProfileSteps.push_back({25.0f, 2.0f});
    }
    
    g_currentStepIndex = 0;
    // Physics continuity
    g_stepStartTemperature = g_currentTemperature; 
    g_stepStartTime = millis();
    
    if (g_activeProfileSteps.size() > 0) {
        g_currentState = STATE_RUNNING_RAMP;
    }
}

void Process_NextStep() {
    g_currentStepIndex++;
    if (g_currentStepIndex >= g_activeProfileSteps.size()) {
        g_currentState = STATE_MAIN_MENU; 
    } else {
        g_stepStartTemperature = g_currentTemperature;
        g_stepStartTime = millis();
        g_currentState = STATE_RUNNING_RAMP;
    }
}

void Process_Update() {
    static unsigned long lastTickTime = 0;
    unsigned long now = millis();

    // --- Physics Simulation ---
    if (now - lastTickTime >= SIM_UPDATE_RATE_MS) {
        float deltaTimeSec = (now - lastTickTime) / 1000.0f;
        
        if (g_heaterState) {
            g_currentTemperature += HEATER_POWER_RATE * deltaTimeSec; 
        } else {
            if (g_currentTemperature > TEMP_AMBIENT) {
                float dynamicCooling = (g_currentTemperature - TEMP_AMBIENT) * 0.02f;
                g_currentTemperature -= (COOLING_RATE_NAT + dynamicCooling) * deltaTimeSec;
            }
        }
        lastTickTime = now;
    }

    // --- Profile Logic ---
    if (g_currentState == STATE_RUNNING_RAMP) {
        ReflowStep step = g_activeProfileSteps[g_currentStepIndex];
        unsigned long elapsedMs = millis() - g_stepStartTime;
        float elapsedSec = elapsedMs / 1000.0f;

        float tempDiff = step.targetTemperature - g_stepStartTemperature;
        int direction = (tempDiff > 0) ? 1 : -1;
        
        float rampedTemp = g_stepStartTemperature + (step.rampRate * elapsedSec * direction);

        bool isTargetReached = false;
        if ((direction == 1 && rampedTemp >= step.targetTemperature) || 
            (direction == -1 && rampedTemp <= step.targetTemperature)) {
            rampedTemp = step.targetTemperature;
            isTargetReached = true;
        }

        g_currentSetpoint = rampedTemp;

        // Completion Check
        float error = abs(g_currentTemperature - step.targetTemperature);
        if (isTargetReached && error < 2.0f) {
            // Simplified Logic: If Auto Advance is OFF, we wait. If ON, we proceed.
            if (g_autoAdvanceSteps) {
                Process_NextStep();
            } else {
                g_currentState = STATE_RUNNING_WAIT;
            }
        }
    } 
    else if (g_currentState == STATE_RUNNING_WAIT) {
        g_currentSetpoint = g_activeProfileSteps[g_currentStepIndex].targetTemperature;
    }
    else {
        g_heaterState = false;
        g_currentSetpoint = 0;
    }

    // --- Bang-Bang Controller ---
    if (g_currentState == STATE_RUNNING_RAMP || g_currentState == STATE_RUNNING_WAIT) {
        if (g_currentTemperature < g_currentSetpoint - 1.0f) {
            g_heaterState = true;
        } else if (g_currentTemperature > g_currentSetpoint + 0.5f) {
            g_heaterState = false;
        }
    }
    
    digitalWrite(PIN_HEATER_LED, g_heaterState ? HIGH : LOW);
}