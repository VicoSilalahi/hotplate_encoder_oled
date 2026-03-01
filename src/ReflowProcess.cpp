#include "ReflowProcess.h"
#include "GlobalContext.h"

void Process_Init() {
    pinMode(PIN_HEATER_LED, OUTPUT);
    digitalWrite(PIN_HEATER_LED, LOW);
    
    // Initialize Default Manual Profile if empty
    if (ctx.manualSteps.empty()) {
        ctx.manualSteps.push_back({100.0f, 1.0f, 30}); // 100C, 1C/s, 30s soak
        ctx.manualSteps.push_back({180.0f, 2.0f, 60}); // 180C, 2C/s, 60s reflow
    }
    
    // Give sensor time to stabilize
    delay(500);
}

void Process_Start(bool useManualSource, bool autoAdvance) {
    ctx.isManualSource = useManualSource;
    ctx.autoAdvance = autoAdvance;

    if (ctx.isManualSource) {
        ctx.activeSteps = ctx.manualSteps;
    } else {
        // Load professional reflow preset
        ctx.activeSteps.clear();
        ctx.activeSteps.push_back({150.0f, 1.0f, 60}); // Preheat/Soak
        ctx.activeSteps.push_back({230.0f, 2.0f, 30}); // Reflow
        ctx.activeSteps.push_back({25.0f, 2.0f, 0});   // Cool
    }
    
    ctx.currentStepIndex = 0;
    
    // Read actual temp for start point
    double readTemp = g_thermocouple.readCelsius();
    if (!isnan(readTemp)) {
        ctx.currentTemperature = (float)readTemp;
    } else {
        Serial.println(F("[PROCESS] Warning: Initial temperature read failed (NaN)"));
    }

    ctx.stepStartTemperature = ctx.currentTemperature; 
    ctx.stepStartTime = millis();
    
    if (!ctx.activeSteps.empty()) {
        ctx.state = STATE_RUNNING_RAMP;
    }
}

void Process_NextStep() {
    ctx.currentStepIndex++;
    if (ctx.currentStepIndex >= (int)ctx.activeSteps.size()) {
        ctx.state = STATE_MAIN_MENU; 
        ctx.heaterActive = false;
    } else {
        ctx.stepStartTemperature = ctx.currentTemperature;
        ctx.stepStartTime = millis();
        ctx.state = STATE_RUNNING_RAMP;
    }
}

void Process_Update() {
    static unsigned long lastTickTime = 0;
    unsigned long now = millis();

    // --- HARDWARE SENSOR UPDATE ---
    if (now - lastTickTime >= SENSOR_READ_DELAY_MS) {
        double rawTemp = g_thermocouple.readCelsius();
        
        if (!isnan(rawTemp)) {
            ctx.currentTemperature = (float)rawTemp;
            // Diagnostic output if needed
            // Serial.printf("Temp: %.2f\n", rawTemp);
        } else {
            // If NaN, we have a sensor error. In a real system, we should safety-stop.
            Serial.println(F("[SENSOR] Error: MAX6675 returned NaN!"));
            // ctx.heaterActive = false; // Emergency stop if desired
        }
        
        lastTickTime = now;
    }

    // --- Profile Logic ---
    if (ctx.state == STATE_RUNNING_RAMP) {
        ReflowStep step = ctx.activeSteps[ctx.currentStepIndex];
        unsigned long elapsedMs = millis() - ctx.stepStartTime;
        float elapsedSec = elapsedMs / 1000.0f;

        float tempDiff = step.targetTemperature - ctx.stepStartTemperature;
        int direction = (tempDiff > 0) ? 1 : -1;
        
        // Calculate where we SHOULD be (dynamic setpoint)
        float rampedTemp = ctx.stepStartTemperature + (step.rampRate * elapsedSec * direction);

        // Clamp target reaching
        bool isSetpointReached = false;
        if ((direction == 1 && rampedTemp >= step.targetTemperature) || 
            (direction == -1 && rampedTemp <= step.targetTemperature)) {
            rampedTemp = step.targetTemperature;
            isSetpointReached = true;
        }

        ctx.currentSetpoint = rampedTemp;

        // Completion Check (Real Temp vs Target)
        float error = abs(ctx.currentTemperature - step.targetTemperature);
        
        // Advance condition: dynamic setpoint reached AND real temp within 2C of target
        if (isSetpointReached && error < 2.0f) {
            if (step.holdSeconds > 0) {
                ctx.state = STATE_RUNNING_HOLD;
                ctx.stepStartTime = millis(); // Reset timer for hold
            } else {
                if (ctx.autoAdvance) {
                    Process_NextStep();
                } else {
                    ctx.state = STATE_RUNNING_WAIT;
                }
            }
        }
    } 
    else if (ctx.state == STATE_RUNNING_HOLD) {
        ReflowStep step = ctx.activeSteps[ctx.currentStepIndex];
        unsigned long elapsedHoldMs = millis() - ctx.stepStartTime;
        
        ctx.currentSetpoint = step.targetTemperature;

        if (elapsedHoldMs >= (unsigned long)step.holdSeconds * 1000) {
            if (ctx.autoAdvance) {
                Process_NextStep();
            } else {
                ctx.state = STATE_RUNNING_WAIT;
            }
        }
    }
    else if (ctx.state == STATE_RUNNING_WAIT) {
        ctx.currentSetpoint = ctx.activeSteps[ctx.currentStepIndex].targetTemperature;
    }
    else {
        ctx.heaterActive = false;
        ctx.currentSetpoint = 0;
    }

    // --- Bang-Bang Controller ---
    if (ctx.state == STATE_RUNNING_RAMP || ctx.state == STATE_RUNNING_HOLD || ctx.state == STATE_RUNNING_WAIT) {
        if (ctx.currentTemperature < ctx.currentSetpoint - 1.0f) {
            ctx.heaterActive = true;
        } else if (ctx.currentTemperature > ctx.currentSetpoint + 0.5f) {
            ctx.heaterActive = false;
        }
    }
    
    digitalWrite(PIN_HEATER_LED, ctx.heaterActive ? HIGH : LOW);
}