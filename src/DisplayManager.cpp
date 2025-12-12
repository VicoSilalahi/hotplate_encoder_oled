#include "DisplayManager.h"
#include "GlobalContext.h"
#include <Wire.h>

void Display_Init() {
    Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL);
    if(!g_oledDisplay.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
        Serial.println(F("OLED Allocation Failed"));
        for(;;);
    }
    g_oledDisplay.clearDisplay();
    g_oledDisplay.setTextColor(SSD1306_WHITE);
    g_oledDisplay.display();
}

void Display_Update() {
    g_oledDisplay.clearDisplay();

    g_oledDisplay.setTextSize(1);
    g_oledDisplay.setCursor(0, 0);
  
    if (g_currentState == STATE_MAIN_MENU) {
        g_oledDisplay.println(F("== REFLOW MATE =="));
        
        g_oledDisplay.setCursor(10, 20); 
        g_oledDisplay.print(g_menuCursorIndex == 0 ? ">" : " "); g_oledDisplay.println("Run Manual");
        
        g_oledDisplay.setCursor(10, 32); 
        g_oledDisplay.print(g_menuCursorIndex == 1 ? ">" : " "); g_oledDisplay.println("Config Manual");
        
        g_oledDisplay.setCursor(10, 44); 
        g_oledDisplay.print(g_menuCursorIndex == 2 ? ">" : " "); g_oledDisplay.println("Load Presets");
        
        g_oledDisplay.setCursor(0, 56);
        g_oledDisplay.print("Temp: "); g_oledDisplay.print(g_currentTemperature, 1); g_oledDisplay.print("C");
    }
    else if (g_currentState == STATE_SELECT_RUN_MODE) {
        g_oledDisplay.println(F("== SELECT MODE =="));
        
        // Show source
        g_oledDisplay.setCursor(0, 16);
        g_oledDisplay.print("Src: "); 
        g_oledDisplay.println(g_isManualSource ? "Manual Cfg" : "Preset");

        g_oledDisplay.setCursor(10, 35);
        g_oledDisplay.print(g_menuCursorIndex == 0 ? ">" : " "); g_oledDisplay.println("Unattended (Auto)");

        g_oledDisplay.setCursor(10, 48);
        g_oledDisplay.print(g_menuCursorIndex == 1 ? ">" : " "); g_oledDisplay.println("Attended (Step)");
    }
    else if (g_currentState == STATE_MANUAL_CONFIG) {
        g_oledDisplay.println(F("[CONFIG MANUAL]"));
        
        // Moved down to y=16 to avoid the Yellow/Blue OLED gap
        int startIdx = (g_listScrollIndex > 2) ? g_listScrollIndex - 2 : 0;
        
        for (int i = 0; i < 4; i++) {
            int itemIdx = startIdx + i;
            // Increased spacing to 12px
            int yPos = 16 + (i * 12);
            
            if (itemIdx < g_manualProfileSteps.size()) {
                g_oledDisplay.setCursor(0, yPos);
                g_oledDisplay.print(g_listScrollIndex == itemIdx ? ">" : " ");
                g_oledDisplay.print("P"); g_oledDisplay.print(itemIdx + 1);
                g_oledDisplay.print(": "); g_oledDisplay.print(g_manualProfileSteps[itemIdx].targetTemperature, 0);
                g_oledDisplay.print("C "); g_oledDisplay.print(g_manualProfileSteps[itemIdx].rampRate, 1);
            } 
            else if (itemIdx == g_manualProfileSteps.size()) {
                g_oledDisplay.setCursor(0, yPos);
                g_oledDisplay.print(g_listScrollIndex == itemIdx ? ">" : " "); g_oledDisplay.println("Add Step (+)");
            }
            else if (itemIdx == g_manualProfileSteps.size() + 1) {
                g_oledDisplay.setCursor(0, yPos);
                g_oledDisplay.print(g_listScrollIndex == itemIdx ? ">" : " "); g_oledDisplay.println("Del Step (-)");
            }
            else if (itemIdx == g_manualProfileSteps.size() + 2) {
                g_oledDisplay.setCursor(0, yPos);
                g_oledDisplay.print(g_listScrollIndex == itemIdx ? ">" : " "); g_oledDisplay.println("Back");
            }
        }
    }
    else if (g_currentState == STATE_EDIT_STEP) {
        g_oledDisplay.print(F("EDIT PHASE ")); g_oledDisplay.println(g_editStepIndex + 1);
        
        float prevTemp = TEMP_AMBIENT; 
        if (g_editStepIndex > 0) {
            prevTemp = g_manualProfileSteps[g_editStepIndex - 1].targetTemperature;
        }

        g_oledDisplay.setCursor(10, 16);
        g_oledDisplay.print("From: "); g_oledDisplay.print(prevTemp, 0); g_oledDisplay.println(" C");
        
        g_oledDisplay.setCursor(10, 28);
        g_oledDisplay.print(g_editParameterIndex == 0 ? ">" : " "); 
        g_oledDisplay.print("Tgt:  "); g_oledDisplay.print(g_manualProfileSteps[g_editStepIndex].targetTemperature, 0); g_oledDisplay.println(" C");
        
        g_oledDisplay.setCursor(10, 40);
        g_oledDisplay.print(g_editParameterIndex == 1 ? ">" : " "); 
        g_oledDisplay.print("Rate: "); g_oledDisplay.print(g_manualProfileSteps[g_editStepIndex].rampRate, 1); g_oledDisplay.println(" C/s");

        float tempDiff = abs(g_manualProfileSteps[g_editStepIndex].targetTemperature - prevTemp);
        float estSeconds = (g_manualProfileSteps[g_editStepIndex].rampRate > 0) 
                           ? (tempDiff / g_manualProfileSteps[g_editStepIndex].rampRate) 
                           : 0;

        g_oledDisplay.setCursor(10, 52);
        g_oledDisplay.print("Est: ~"); g_oledDisplay.print(estSeconds, 0); g_oledDisplay.print(" s");
    }
    else if (g_currentState == STATE_RUNNING_RAMP || g_currentState == STATE_RUNNING_WAIT) {
        g_oledDisplay.setCursor(0,0);
        if (g_currentState == STATE_RUNNING_WAIT) {
            g_oledDisplay.setTextColor(SSD1306_BLACK, SSD1306_WHITE); // Inverted
            g_oledDisplay.print(" WAIT INPUT ");
            g_oledDisplay.setTextColor(SSD1306_WHITE);
        } else {
            g_oledDisplay.print(" RAMPING... ");
        }
        
        g_oledDisplay.print(" P"); g_oledDisplay.print(g_currentStepIndex + 1);
        g_oledDisplay.print("/"); g_oledDisplay.println(g_activeProfileSteps.size());

        g_oledDisplay.setTextSize(2);
        g_oledDisplay.setCursor(0, 20);
        g_oledDisplay.print(g_currentTemperature, 1); 
        g_oledDisplay.setTextSize(1); g_oledDisplay.print("c");
        
        g_oledDisplay.setCursor(80, 20);
        g_oledDisplay.setTextSize(1);
        g_oledDisplay.print("Set:"); 
        g_oledDisplay.setCursor(80, 30);
        g_oledDisplay.print(g_currentSetpoint, 0);

        g_oledDisplay.setCursor(0, 50);
        if (g_currentState == STATE_RUNNING_WAIT) {
            g_oledDisplay.print("CLICK FOR NEXT >");
        } else {
            g_oledDisplay.print("Tgt: "); g_oledDisplay.print(g_activeProfileSteps[g_currentStepIndex].targetTemperature, 0);
            g_oledDisplay.print(" @ "); g_oledDisplay.print(g_activeProfileSteps[g_currentStepIndex].rampRate, 1); g_oledDisplay.print("/s");
        }
    }

    g_oledDisplay.display();
}