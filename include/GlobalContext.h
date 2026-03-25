#pragma once
#include <Arduino.h>
#include <Adafruit_SSD1306.h>
#include <ESP32Encoder.h>
#include <vector>
#include <cmath>
#include "SystemTypes.h"
#include "Config.h"

// --- Global Objects ---
extern Adafruit_SSD1306 g_oledDisplay;
extern ESP32Encoder g_rotaryEncoder;
// Thermistor read function (returns Celsius)
double Read_TemperatureC();

// --- System State Structure ---
struct SystemContext {
    SystemState state;
    bool isManualSource;
    bool autoAdvance;
    
    float currentTemperature;
    float currentSetpoint;
    bool heaterActive;
    
    int currentStepIndex;
    unsigned long stepStartTime;
    float stepStartTemperature;
    
    // UI Navigation
    int menuCursor;
    int scrollIndex;
    int editStep;
    int editParameter;
    
    // Input
    long lastEncoderPos;
    unsigned long lastButtonTime;
    
    // Profiles
    std::vector<ReflowStep> manualSteps;
    std::vector<ReflowStep> activeSteps;
};

extern SystemContext ctx;

// --- Helper Functions ---
void Global_AddManualStep();
void Global_RemoveManualStep();
void Global_Init();