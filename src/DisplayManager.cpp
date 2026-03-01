#include "DisplayManager.h"
#include "GlobalContext.h"
#include <Wire.h>

void Display_Init() {
    Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL);
    if(!g_oledDisplay.begin(SSD1306_SWITCHCAPVCC, DISPLAY_I2C_ADDR)) {
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
  
    if (ctx.state == STATE_MAIN_MENU) {
        g_oledDisplay.println(F("== REFLOW MATE =="));
        
        g_oledDisplay.setCursor(10, 20); 
        g_oledDisplay.print(ctx.menuCursor == 0 ? ">" : " "); g_oledDisplay.println("Run Manual");
        
        g_oledDisplay.setCursor(10, 32); 
        g_oledDisplay.print(ctx.menuCursor == 1 ? ">" : " "); g_oledDisplay.println("Config Manual");
        
        g_oledDisplay.setCursor(10, 44); 
        g_oledDisplay.print(ctx.menuCursor == 2 ? ">" : " "); g_oledDisplay.println("Load Presets");
        
        g_oledDisplay.setCursor(0, 56);
        g_oledDisplay.print("Temp: "); g_oledDisplay.print(ctx.currentTemperature, 1); g_oledDisplay.print("C");
    }
    else if (ctx.state == STATE_SELECT_RUN_MODE) {
        g_oledDisplay.println(F("== SELECT MODE =="));
        
        g_oledDisplay.setCursor(0, 16);
        g_oledDisplay.print("Src: "); 
        g_oledDisplay.println(ctx.isManualSource ? "Manual Cfg" : "Preset");

        g_oledDisplay.setCursor(10, 35);
        g_oledDisplay.print(ctx.menuCursor == 0 ? ">" : " "); g_oledDisplay.println("Unattended (Auto)");

        g_oledDisplay.setCursor(10, 48);
        g_oledDisplay.print(ctx.menuCursor == 1 ? ">" : " "); g_oledDisplay.println("Attended (Step)");
    }
    else if (ctx.state == STATE_MANUAL_CONFIG) {
        g_oledDisplay.println(F("[CONFIG MANUAL]"));
        
        int startIdx = (ctx.scrollIndex > 2) ? ctx.scrollIndex - 2 : 0;
        
        for (int i = 0; i < 4; i++) {
            int itemIdx = startIdx + i;
            int yPos = 16 + (i * 12);
            
            if (itemIdx < (int)ctx.manualSteps.size()) {
                g_oledDisplay.setCursor(0, yPos);
                g_oledDisplay.print(ctx.scrollIndex == itemIdx ? ">" : " ");
                g_oledDisplay.print("P"); g_oledDisplay.print(itemIdx + 1);
                g_oledDisplay.print(": "); g_oledDisplay.print(ctx.manualSteps[itemIdx].targetTemperature, 0);
                g_oledDisplay.print("C @ "); g_oledDisplay.print(ctx.manualSteps[itemIdx].rampRate, 1);
            } 
            else if (itemIdx == (int)ctx.manualSteps.size()) {
                g_oledDisplay.setCursor(0, yPos);
                g_oledDisplay.print(ctx.scrollIndex == itemIdx ? ">" : " "); g_oledDisplay.println("Add Step (+)");
            }
            else if (itemIdx == (int)ctx.manualSteps.size() + 1) {
                g_oledDisplay.setCursor(0, yPos);
                g_oledDisplay.print(ctx.scrollIndex == itemIdx ? ">" : " "); g_oledDisplay.println("Del Step (-)");
            }
            else if (itemIdx == (int)ctx.manualSteps.size() + 2) {
                g_oledDisplay.setCursor(0, yPos);
                g_oledDisplay.print(ctx.scrollIndex == itemIdx ? ">" : " "); g_oledDisplay.println("Back");
            }
        }
    }
    else if (ctx.state == STATE_EDIT_STEP) {
        g_oledDisplay.print(F("EDIT PHASE ")); g_oledDisplay.println(ctx.editStep + 1);
        
        float prevTemp = TEMP_AMBIENT; 
        if (ctx.editStep > 0) {
            prevTemp = ctx.manualSteps[ctx.editStep - 1].targetTemperature;
        }

        g_oledDisplay.setCursor(10, 16);
        g_oledDisplay.print("From: "); g_oledDisplay.print(prevTemp, 0); g_oledDisplay.println(" C");
        
        g_oledDisplay.setCursor(10, 28);
        g_oledDisplay.print(ctx.editParameter == 0 ? ">" : " "); 
        g_oledDisplay.print("Tgt:  "); g_oledDisplay.print(ctx.manualSteps[ctx.editStep].targetTemperature, 0); g_oledDisplay.println(" C");
        
        g_oledDisplay.setCursor(10, 40);
        g_oledDisplay.print(ctx.editParameter == 1 ? ">" : " "); 
        g_oledDisplay.print("Rate: "); g_oledDisplay.print(ctx.manualSteps[ctx.editStep].rampRate, 1); g_oledDisplay.println(" C/s");

        g_oledDisplay.setCursor(10, 52);
        g_oledDisplay.print(ctx.editParameter == 2 ? ">" : " "); 
        g_oledDisplay.print("Hold: "); g_oledDisplay.print(ctx.manualSteps[ctx.editStep].holdSeconds); g_oledDisplay.println(" s");
    }
    else if (ctx.state == STATE_RUNNING_RAMP || ctx.state == STATE_RUNNING_HOLD || ctx.state == STATE_RUNNING_WAIT) {
        g_oledDisplay.setCursor(0,0);
        if (ctx.state == STATE_RUNNING_WAIT) {
            g_oledDisplay.setTextColor(SSD1306_BLACK, SSD1306_WHITE);
            g_oledDisplay.print(" WAIT INPUT ");
            g_oledDisplay.setTextColor(SSD1306_WHITE);
        } else if (ctx.state == STATE_RUNNING_HOLD) {
            g_oledDisplay.setTextColor(SSD1306_BLACK, SSD1306_WHITE);
            g_oledDisplay.print(" HOLDING... ");
            g_oledDisplay.setTextColor(SSD1306_WHITE);
        } else {
            g_oledDisplay.print(" RAMPING... ");
        }
        
        g_oledDisplay.print(" P"); g_oledDisplay.print(ctx.currentStepIndex + 1);
        g_oledDisplay.print("/"); g_oledDisplay.println(ctx.activeSteps.size());

        g_oledDisplay.setTextSize(2);
        g_oledDisplay.setCursor(0, 20);
        g_oledDisplay.print(ctx.currentTemperature, 1); 
        g_oledDisplay.setTextSize(1); g_oledDisplay.print("c");
        
        // Target / Setpoint box
        g_oledDisplay.drawRect(75, 18, 50, 25, SSD1306_WHITE);
        g_oledDisplay.setCursor(80, 22);
        g_oledDisplay.print("Set:"); 
        g_oledDisplay.setCursor(80, 32);
        g_oledDisplay.print(ctx.currentSetpoint, 0);

        // Heater Status Indicator
        if (ctx.heaterActive) {
            g_oledDisplay.fillCircle(120, 5, 3, SSD1306_WHITE);
        }

        g_oledDisplay.setCursor(0, 50);
        if (ctx.state == STATE_RUNNING_WAIT) {
            g_oledDisplay.print("CLICK FOR NEXT >");
        } else if (ctx.state == STATE_RUNNING_HOLD) {
            unsigned long elapsed = (millis() - ctx.stepStartTime) / 1000;
            int remaining = ctx.activeSteps[ctx.currentStepIndex].holdSeconds - elapsed;
            g_oledDisplay.print("Hold: "); g_oledDisplay.print(remaining); g_oledDisplay.print("s left");
        } else {
            g_oledDisplay.print("Tgt: "); g_oledDisplay.print(ctx.activeSteps[ctx.currentStepIndex].targetTemperature, 0);
            g_oledDisplay.print(" @ "); g_oledDisplay.print(ctx.activeSteps[ctx.currentStepIndex].rampRate, 1); 
        }
    }

    g_oledDisplay.display();
}