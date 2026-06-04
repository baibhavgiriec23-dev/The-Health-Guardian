#ifndef SENSORS_H
#define SENSORS_H

#include <Arduino.h>

struct HealthData {
    float heartRate;
    float spo2;
    float temperature;
    float roll;
    float pitch;
    float ecgValue;
    bool ecgLeadOK;
    bool systemHealthy;
};

// Functions
void initSensors();
bool runSelfTest();
void enterDeepSleep();
float applyIIRFilter(float input);
void handleHighSpeedSampling(); // To be called by Hardware Timer ISR

extern HealthData latestVitals; // Global access with Mutex safety
#endif