#include <Arduino.h>
#include "PinConfig.h"
#include "IRHandler.h"
#include "Current.h"
#include "Voltage.h"
#include "display.h"

// Sensor calibration constants
const float slope_1 = 0.002907;    
const float intercept_1 = -0.0659;
const float slope_2 = 0.003176;     
const float intercept_2 = -0.07196;
const float slope_3 = 0.003176;     
const float intercept_3 = -0.07196;

// Voltage sensor configuration
const uint8_t VOLTAGE_PIN = 19;
const float Vref = 3.3;
const float VOLTAGE_CALIBRATION = 1185.0;  // Adjust this value

// Create instances of our control classes
PinConfig pinConfig;
CurrentSensor sensor1(34, slope_1, intercept_1);
CurrentSensor sensor2(35, slope_2, intercept_2);
CurrentSensor sensor3(33, slope_3, intercept_3);
VoltageSensor voltageSensor(VOLTAGE_PIN, Vref, VOLTAGE_CALIBRATION);
IRHandler irHandler(pinConfig);
Display display;

// Timing variables
unsigned long printPeriod = 1500;
unsigned long previousMillis = 0;

void setup() {
    Serial.begin(115200);
    
    Serial.println("Starting Dual ACS712 Current Sensors + ZMPT101B Voltage Sensor");
    Serial.println("Ensure NO LOAD is connected for current sensor calibration...");
    delay(2000);
    
    // Initialize current sensors
    sensor1.begin();
    sensor2.begin();
    sensor3.begin();

    // Calibrate current sensors (no load connected!)
    Serial.println("\nCalibrating current sensors...");
    sensor1.calibrate(1000);
    delay(500);
    sensor2.calibrate(1000);
    delay(500);
    sensor3.calibrate(1000);
    
    // Initialize voltage sensor
    Serial.println("\nInitializing voltage sensor...");
    voltageSensor.begin();
    
    Serial.println("\nCalibration complete. Starting measurements...");
    
    // Initialize hardware
    pinConfig.begin();
    irHandler.begin();
    display.begin();
    delay(1000);
}

void loop() {
    // Process IR commands
    irHandler.update();
     // Update current sensors
        sensor1.update();
        sensor2.update();
        sensor3.update();
        

    // Print readings at regular intervals
    if ((unsigned long)(millis() - previousMillis) >= printPeriod) {
        previousMillis = millis();
        
        Serial.println("\n========== Readings ==========");
        float current1 = sensor1.getCurrent(1);
        float current2 = sensor2.getCurrent(2);
        float current3 = sensor3.getCurrent(3);

        // Get and display current from both sensors
        
        // Get voltage reading (directly calls getRmsVoltage() like original code)
        float voltage = voltageSensor.getRmsVoltage();
        Serial.print("AC RMS Voltage: ");
        Serial.print(voltage);
        Serial.println(" V");
        
        

        // Calculate total current
        float totalCurrent = current1 + current2;
        Serial.print("Total Current: ");
        Serial.print(totalCurrent, 3);
        Serial.println(" A");
        Serial.print("senT:");
        Serial.print(current3, 3);
        Serial.println("A");

        // Calculate power for each circuit
        float power1 = voltage * current1;
        float power2 = voltage * current2;
        float totalPower = power1 + power2;
        
        Serial.print("Power 1: ");
        Serial.print(power1, 2);
        Serial.println(" W");
        
        Serial.print("Power 2: ");
        Serial.print(power2, 2);
        Serial.println(" W");
        
        Serial.print("Total Power: ");
        Serial.print(totalPower, 2);
        Serial.println(" W");
        
        Serial.println("==============================\n");

        // Update LCD display
        display.showCurrents(current1, current2, totalCurrent);
    }
}
