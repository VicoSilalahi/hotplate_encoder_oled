#include <Arduino.h>
#include "Config.h"
#include "GlobalContext.h"
#include "DisplayManager.h"
#include "InputHandler.h"
#include "ReflowProcess.h"

void setup() {
    Serial.begin(SERIAL_BAUD_RATE);
    
    // Initialize global state
    Global_Init();
    
    // Initialize modules
    Input_Init();
    Display_Init();
    Process_Init();
    
    Serial.println(F("Reflow Controller System Ready"));
    Serial.printf("Thermistor Pin: %d, Rdiv=%.0f, R25=%.0f, Beta=%.0f\n", PIN_THERMISTOR, THERMISTOR_R_DIV, THERMISTOR_R25, THERMISTOR_BETA);
}

void loop() {
    Input_Process();
    Process_Update();
    Display_Update();
    
    // Small delay to prevent watchdog issues or excessive loop speed
    delay(10);
}