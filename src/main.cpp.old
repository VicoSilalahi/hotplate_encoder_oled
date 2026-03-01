#include <Arduino.h>
#include "GlobalContext.h"
#include "DisplayManager.h"
#include "InputHandler.h"
#include "ReflowProcess.h"

void setup() {
    Serial.begin(115200);
    
    Input_Init();
    Display_Init();
    Process_Init();
    
    Serial.println(F("Reflow Controller System Ready"));
}

void loop() {
    Input_Process();
    Process_Update();
    Display_Update();
}