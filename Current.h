// CurrentSensor.h
#ifndef CURRENT_SENSOR_H
#define CURRENT_SENSOR_H

#include <Arduino.h>
#include <Filters.h>

class CurrentSensor {
private:
    uint8_t pin;
    float slope;
    float intercept;
    float offset;
    float window;
    float adcRef;
    int adcMax;
    float rms_v1;
    float rmsCurrent;
    RunningStatistics stats;
    bool calibrated;

public:
    // Constructor
    CurrentSensor(uint8_t _pin, float _slope, float _intercept) {
        pin = _pin;
        slope = _slope;
        intercept = _intercept;
        offset = 0;
        window = 40.0 / 50.0;  // 50Hz frequency
        adcRef = 3300.0;       // ESP32 ADC reference voltage in mV
        adcMax = 4095;         // 12-bit ADC resolution
        rms_v1 = 0;
        rmsCurrent = 0;
        calibrated = false;
    }

    // Initialize the sensor
    void begin() {
        pinMode(pin, INPUT);
        analogReadResolution(12);
        stats.setWindowSecs(window);
    }

    // Calibrate zero offset (call with no load connected)
    void calibrate(int samples = 1250) {
        Serial.print("Calibrating sensor on pin ");
        Serial.print(pin);
        Serial.println("...");
        
        float sum = 0;

        for (int i = 0; i < samples; i++) {
            sum += (analogRead(pin) * adcRef) / adcMax;
            delay(1);
        }

        offset = sum / samples;
        calibrated = true;
        
        Serial.print("Zero offset: ");
        Serial.print(offset, 2);
        Serial.println(" mV");
    }

    // Collect samples (call this continuously in loop)
    void update() {
        if (!calibrated) return;
        
        float mv = (analogRead(pin) * adcRef) / adcMax;
        float corrected_mv = mv - offset;
        stats.input(corrected_mv);  // Feed the corrected (centered) AC signal
    }

    // Compute RMS → Amps
    float getCurrent(int sen_num) {
        if (!calibrated) return 0;
        
        // sigma() gives the standard deviation, which equals RMS for a zero-mean AC signal
        rms_v1 = stats.sigma();                 // RMS mV
        rmsCurrent = intercept + slope * rms_v1;
        if (rmsCurrent < 0.002) {
            rmsCurrent = 0;
        }
        
        Serial.print("Corrected RMS: ");
        Serial.print(rms_v1, 3);
        Serial.print(" mV  |  TRMS_");
        Serial.print(sen_num);
        Serial.print(": ");
        Serial.print(rmsCurrent, 3);
        Serial.println(" A");

        return rmsCurrent;
        
    }

    // Optional: Get current offset value
    float getOffset() {
        return offset;
    }

    // Optional: Set custom window length
    void setWindow(float freq) {
        window = 40.0 / freq;
        stats.setWindowSecs(window);
    }
    
    // Check if calibrated
    bool isCalibrated() {
        return calibrated;
    }
};

#endif
