#include "InputHandler.h"
#include "GlobalContext.h"
#include "ReflowProcess.h"
#include <ESP32Encoder.h> // Explicitly include to ensure enum visibility

void Input_Init() {
    // Enable internal pull-ups for the encoder pins (Critical for reliability)
    // accessing the enum value directly (UP) rather than scoped (puType::UP)
    ESP32Encoder::useInternalWeakPullResistors = puType::up;

    // Attach pins to the hardware Pulse Counter (PCNT) unit
    // attachHalfQuad usually feels best for standard encoders (1 step per detent logic)
    g_rotaryEncoder.attachHalfQuad(PIN_ENCODER_DT, PIN_ENCODER_CLK);
    
    // Reset count to 0
    g_rotaryEncoder.setCount(0);
    g_lastEncoderPosition = 0;

    // Button still needs standard GPIO setup
    pinMode(PIN_ENCODER_SW, INPUT_PULLUP);
}

void Input_Process() {
    // 1. Read Hardware Counter
    // We divide by 2 because attachHalfQuad usually generates 2 counts per detent click.
    // If your encoder moves 2 steps per click, change this to / 4.
    // If it moves 0.5 steps per click, remove the division.
    long rawCount = (long)g_rotaryEncoder.getCount();
    long logicalPosition = rawCount / 2; 
    
    int direction = 0;
    
    if (logicalPosition != g_lastEncoderPosition) {
        direction = (logicalPosition > g_lastEncoderPosition) ? 1 : -1;
        g_lastEncoderPosition = logicalPosition;
    }

    // 2. Read Button (Debounced)
    bool isClicked = false;
    if (digitalRead(PIN_ENCODER_SW) == LOW) {
        if (millis() - g_lastButtonPressTime > 250) {
            isClicked = true;
            g_lastButtonPressTime = millis();
        }
    }

    // --- State Machine Navigation ---
    switch (g_currentState) {
        case STATE_MAIN_MENU:
            if (direction) g_menuCursorIndex = constrain(g_menuCursorIndex + direction, 0, 2);
            if (isClicked) {
                if (g_menuCursorIndex == 0) { 
                    g_isManualSource = true;
                    g_menuCursorIndex = 0; 
                    g_currentState = STATE_SELECT_RUN_MODE;
                }
                else if (g_menuCursorIndex == 1) { 
                    g_currentState = STATE_MANUAL_CONFIG; 
                    g_listScrollIndex = 0; 
                }
                else if (g_menuCursorIndex == 2) {
                    g_isManualSource = false;
                    g_menuCursorIndex = 0; 
                    g_currentState = STATE_SELECT_RUN_MODE;
                }
            }
            break;

        case STATE_SELECT_RUN_MODE:
            if (direction) g_menuCursorIndex = constrain(g_menuCursorIndex + direction, 0, 1);
            if (isClicked) {
                bool autoAdvance = (g_menuCursorIndex == 0);
                Process_Start(g_isManualSource, autoAdvance);
            }
            break;

        case STATE_MANUAL_CONFIG:
            {
                int totalItems = g_manualProfileSteps.size() + 3;
                if (direction) g_listScrollIndex = constrain(g_listScrollIndex + direction, 0, totalItems - 1);
                
                if (isClicked) {
                    if (g_listScrollIndex < g_manualProfileSteps.size()) {
                        g_editStepIndex = g_listScrollIndex;
                        g_editParameterIndex = 0;
                        g_currentState = STATE_EDIT_STEP;
                    } else if (g_listScrollIndex == g_manualProfileSteps.size()) {
                        Global_AddManualStep();
                    } else if (g_listScrollIndex == g_manualProfileSteps.size() + 1) {
                        Global_RemoveManualStep();
                        if(g_listScrollIndex > 0) g_listScrollIndex--;
                    } else {
                        g_currentState = STATE_MAIN_MENU;
                    }
                }
            }
            break;

        case STATE_EDIT_STEP:
            if (g_editParameterIndex == 0) { // Target
                if (direction) {
                    g_manualProfileSteps[g_editStepIndex].targetTemperature = 
                        constrain(g_manualProfileSteps[g_editStepIndex].targetTemperature + (direction * 5), 20, 300);
                }
                if (isClicked) g_editParameterIndex++; 
            } else { // Rate
                if (direction) {
                    g_manualProfileSteps[g_editStepIndex].rampRate = 
                        constrain(g_manualProfileSteps[g_editStepIndex].rampRate + (direction * 0.1), 0.1, 10.0);
                }
                if (isClicked) g_currentState = STATE_MANUAL_CONFIG; 
            }
            break;

        case STATE_RUNNING_RAMP:
        case STATE_RUNNING_WAIT:
            if (isClicked) {
                if (g_currentState == STATE_RUNNING_WAIT) {
                    Process_NextStep();
                } else {
                    g_currentState = STATE_MAIN_MENU;
                    g_heaterState = false;
                }
            }
            break;
            
        default:
            break;
    }
}