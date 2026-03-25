#pragma once

// --- Hardware Pin Definitions ---

// Rotary Encoder
#define PIN_ENCODER_CLK 5
#define PIN_ENCODER_DT  18
#define PIN_ENCODER_SW  19

// Thermocouple (MAX6675)
// These are the pins from GlobalContext.h which we assume are the correct ones
// Replace thermocouple with NTC thermistor (100k @ 25C, B=3950)
#define PIN_THERMISTOR  34
// Divider resistor value (Ohms)
#define THERMISTOR_R_DIV 100000.0
// Thermistor parameters
#define THERMISTOR_BETA 3950.0
#define THERMISTOR_R25  100000.0

// Output / Actuators
#define PIN_HEATER_LED  2     // SSR Control (and onboard LED for monitoring)

// I2C OLED Display
#define PIN_I2C_SDA     21
#define PIN_I2C_SCL     22

// --- System Constants ---
#define SERIAL_BAUD_RATE 115200
#define DISPLAY_I2C_ADDR 0x3C
#define DISPLAY_WIDTH    128
#define DISPLAY_HEIGHT   64

#define SENSOR_READ_DELAY_MS 250 
#define TEMP_AMBIENT         25.0f

// Reflow Limits
#define MAX_TARGET_TEMP      300.0f
#define MIN_TARGET_TEMP      20.0f
#define MAX_RAMP_RATE        10.0f
#define MIN_RAMP_RATE        0.1f
