#include "GlobalContext.h"
#include <Wire.h>

// --- Object Instantiation ---
Adafruit_SSD1306 g_oledDisplay(DISPLAY_WIDTH, DISPLAY_HEIGHT, &Wire, -1);
ESP32Encoder g_rotaryEncoder;
MAX6675 g_thermocouple(PIN_THERMO_CLK, PIN_THERMO_CS, PIN_THERMO_DO);

// --- Global State ---
SystemContext ctx;

void Global_Init() {
    ctx.state = STATE_MAIN_MENU;
    ctx.isManualSource = true;
    ctx.autoAdvance = true;
    
    ctx.currentTemperature = TEMP_AMBIENT;
    ctx.currentSetpoint = 0.0f;
    ctx.heaterActive = false;
    
    ctx.currentStepIndex = 0;
    ctx.stepStartTime = 0;
    ctx.stepStartTemperature = TEMP_AMBIENT;
    
    ctx.menuCursor = 0;
    ctx.scrollIndex = 0;
    ctx.editStep = 0;
    ctx.editParameter = 0;
    
    ctx.lastEncoderPos = 0;
    ctx.lastButtonTime = 0;
    
    ctx.manualSteps.clear();
    ctx.activeSteps.clear();
}

void Global_AddManualStep() {
    if (ctx.manualSteps.size() < 8) {
        ctx.manualSteps.push_back({150.0f, 1.0f});
    }
}

void Global_RemoveManualStep() {
    if (ctx.manualSteps.size() > 1) {
        ctx.manualSteps.pop_back();
    }
}