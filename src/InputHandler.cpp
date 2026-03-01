#include "InputHandler.h"
#include "GlobalContext.h"
#include "ReflowProcess.h"
#include <ESP32Encoder.h>

void Input_Init() {
    ESP32Encoder::useInternalWeakPullResistors = puType::up;
    g_rotaryEncoder.attachHalfQuad(PIN_ENCODER_DT, PIN_ENCODER_CLK);
    g_rotaryEncoder.setCount(0);
    ctx.lastEncoderPos = 0;
    pinMode(PIN_ENCODER_SW, INPUT_PULLUP);
}

void Input_Process() {
    // 1. Read Hardware Counter
    long rawCount = (long)g_rotaryEncoder.getCount();
    long logicalPosition = rawCount / 2; 
    
    int direction = 0;
    if (logicalPosition != ctx.lastEncoderPos) {
        direction = (logicalPosition > ctx.lastEncoderPos) ? 1 : -1;
        ctx.lastEncoderPos = logicalPosition;
    }

    // 2. Read Button (Debounced)
    bool isClicked = false;
    if (digitalRead(PIN_ENCODER_SW) == LOW) {
        if (millis() - ctx.lastButtonTime > 250) {
            isClicked = true;
            ctx.lastButtonTime = millis();
        }
    }

    // --- State Machine Navigation ---
    switch (ctx.state) {
        case STATE_MAIN_MENU:
            if (direction) ctx.menuCursor = constrain(ctx.menuCursor + direction, 0, 2);
            if (isClicked) {
                if (ctx.menuCursor == 0) { 
                    ctx.isManualSource = true;
                    ctx.menuCursor = 0; 
                    ctx.state = STATE_SELECT_RUN_MODE;
                }
                else if (ctx.menuCursor == 1) { 
                    ctx.state = STATE_MANUAL_CONFIG; 
                    ctx.scrollIndex = 0; 
                }
                else if (ctx.menuCursor == 2) {
                    ctx.isManualSource = false;
                    ctx.menuCursor = 0; 
                    ctx.state = STATE_SELECT_RUN_MODE;
                }
            }
            break;

        case STATE_SELECT_RUN_MODE:
            if (direction) ctx.menuCursor = constrain(ctx.menuCursor + direction, 0, 1);
            if (isClicked) {
                bool autoAdvance = (ctx.menuCursor == 0);
                Process_Start(ctx.isManualSource, autoAdvance);
            }
            break;

        case STATE_MANUAL_CONFIG:
            {
                int totalItems = ctx.manualSteps.size() + 3;
                if (direction) ctx.scrollIndex = constrain(ctx.scrollIndex + direction, 0, totalItems - 1);
                
                if (isClicked) {
                    if (ctx.scrollIndex < (int)ctx.manualSteps.size()) {
                        ctx.editStep = ctx.scrollIndex;
                        ctx.editParameter = 0;
                        ctx.state = STATE_EDIT_STEP;
                    } else if (ctx.scrollIndex == ctx.manualSteps.size()) {
                        Global_AddManualStep();
                    } else if (ctx.scrollIndex == (int)ctx.manualSteps.size() + 1) {
                        Global_RemoveManualStep();
                        if(ctx.scrollIndex > 0) ctx.scrollIndex--;
                    } else {
                        ctx.state = STATE_MAIN_MENU;
                    }
                }
            }
            break;

        case STATE_EDIT_STEP:
            if (ctx.editParameter == 0) { // Target
                if (direction) {
                    ctx.manualSteps[ctx.editStep].targetTemperature = 
                        constrain(ctx.manualSteps[ctx.editStep].targetTemperature + (direction * 5), MIN_TARGET_TEMP, MAX_TARGET_TEMP);
                }
                if (isClicked) ctx.editParameter++; 
            } else if (ctx.editParameter == 1) { // Rate
                if (direction) {
                    ctx.manualSteps[ctx.editStep].rampRate = 
                        constrain(ctx.manualSteps[ctx.editStep].rampRate + (direction * 0.1), MIN_RAMP_RATE, MAX_RAMP_RATE);
                }
                if (isClicked) ctx.editParameter++; 
            } else { // Hold
                if (direction) {
                    ctx.manualSteps[ctx.editStep].holdSeconds = 
                        constrain(ctx.manualSteps[ctx.editStep].holdSeconds + (direction * 5), 0, 300);
                }
                if (isClicked) ctx.state = STATE_MANUAL_CONFIG; 
            }
            break;

        case STATE_RUNNING_RAMP:
        case STATE_RUNNING_WAIT:
            if (isClicked) {
                if (ctx.state == STATE_RUNNING_WAIT) {
                    Process_NextStep();
                } else {
                    ctx.state = STATE_MAIN_MENU;
                    ctx.heaterActive = false;
                }
            }
            break;
            
        default:
            break;
    }
}