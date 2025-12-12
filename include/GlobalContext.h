#pragma once
#include <Arduino.h>
#include <Adafruit_SSD1306.h>
#include <ESP32Encoder.h> // Hardware Pulse Counter Lib
#include <vector>
#include "SystemTypes.h"

// --- Hardware Pin Definitions ---
#define PIN_ENCODER_CLK 5
#define PIN_ENCODER_DT  18
#define PIN_ENCODER_SW  19
#define PIN_HEATER_LED  2     
#define PIN_I2C_SDA     21
#define PIN_I2C_SCL     22

// --- Physics & Simulation Constants ---
#define SIM_UPDATE_RATE_MS 100
#define TEMP_AMBIENT       25.0f
#define HEATER_POWER_RATE  6.0f 
#define COOLING_RATE_NAT   0.5f

// --- Global Objects ---
extern Adafruit_SSD1306 g_oledDisplay;
extern ESP32Encoder g_rotaryEncoder; // Replaces manual counters

// --- UI Navigation State ---
extern SystemState g_currentState;
extern int g_menuCursorIndex;
extern int g_listScrollIndex;
extern int g_editStepIndex;
extern int g_editParameterIndex; 

// --- Hardware Input State ---
extern long g_lastEncoderPosition;
extern unsigned long g_lastButtonPressTime;

// --- Reflow Profile Data ---
extern std::vector<ReflowStep> g_manualProfileSteps;
extern std::vector<ReflowStep> g_activeProfileSteps;

// --- Process Logic State ---
extern bool g_isManualSource;
extern bool g_autoAdvanceSteps;
extern int g_currentStepIndex;
extern unsigned long g_stepStartTime;
extern float g_stepStartTemperature;
extern float g_currentSetpoint;
extern float g_currentTemperature;
extern bool g_heaterState;

// --- Helper Functions ---
void Global_AddManualStep();
void Global_RemoveManualStep();