#include "Sensors.h"
#include <Wire.h>
#include "MAX30105.h"
#include <DallasTemperature.h>
#include <OneWire.h>

MAX30105 particleSensor;
OneWire oneWire(4);
DallasTemperature sensors(&oneWire);
HealthData latestVitals;

// Filter variables
float filtered_ecg = 0;
const float IIR_ALPHA = 0.2f; 

void initSensors() {
    Wire.begin(21, 22);
    Wire.setClock(400000);
    particleSensor.begin(Wire);
    particleSensor.setup(0x1F, 4, 2, 100);
    
    // MPU6050 Wake
    Wire.beginTransmission(0x68);
    Wire.write(0x6B);
    Wire.write(0);
    Wire.endTransmission();
    
    sensors.begin();
}

bool runSelfTest() {
    Wire.beginTransmission(0x68);
    bool mpu = (Wire.endTransmission() == 0);
    Wire.beginTransmission(0x57);
    bool maxS = (Wire.endTransmission() == 0);
    return mpu && maxS;
}

float applyIIRFilter(float input) {
    // Low-pass filter to remove high-freq jitter
    filtered_ecg = (IIR_ALPHA * input) + ((1.0f - IIR_ALPHA) * filtered_ecg);
    return filtered_ecg;
}

void enterDeepSleep() {
    Serial.println("System Sleep: Wake on MPU6050 Motion");
    esp_sleep_enable_ext0_wakeup(GPIO_NUM_15, 1); // Assume MPU INT on Pin 15
    esp_deep_sleep_start();
}