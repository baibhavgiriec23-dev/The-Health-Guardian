#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>
#include <esp_task_wdt.h>
#include "Sensors.h"

WebServer server(80);
SemaphoreHandle_t xVitalsMutex;
hw_timer_t * timer = NULL;
const int DEBUG_PIN = 2; // For Logic Analyzer

// 1. Hardware Interrupt Service Routine (ISR)
void IRAM_ATTR onTimer() {
    digitalWrite(DEBUG_PIN, HIGH); // Start timing check
    int raw = analogRead(34);
    latestVitals.ecgValue = applyIIRFilter((float)raw - 2048.0); // DC offset 
    digitalWrite(DEBUG_PIN, LOW); // End timing check
}

void setup() {
    Serial.begin(115200);
    pinMode(DEBUG_PIN, OUTPUT);
    xVitalsMutex = xSemaphoreCreateMutex();

    // FEATURE: Power-On Self-Test
    initSensors();
    latestVitals.systemHealthy = runSelfTest();

    // FEATURE: Watchdog Timer (5s timeout)
    esp_task_wdt_init(5, true); 
    esp_task_wdt_add(NULL);

    // FEATURE: Hardware Timer for 360Hz ECG
    timer = timerBegin(0, 80, true); 
    timerAttachInterrupt(timer, &onTimer, true);
    timerAlarmWrite(timer, 2777, true); // 1,000,000 / 360 = 2777us
    timerAlarmEnable(timer);

    WiFi.begin("SSID", "PASS");

    // Start RTOS Tasks
    xTaskCreatePinnedToCore(TaskBio, "Bio", 4096, NULL, 2, NULL, 1);
    xTaskCreatePinnedToCore(TaskWeb, "Web", 8192, NULL, 1, NULL, 0);
}

void TaskBio(void *pvParameters) {
    for (;;) {
        esp_task_wdt_reset(); // Kick the Dog
        float t = getTemperature();
        if (xSemaphoreTake(xVitalsMutex, portMAX_DELAY)) {
            latestVitals.temperature = t;
            xSemaphoreGive(xVitalsMutex);
        }
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void TaskWeb(void *pvParameters) {
    server.on("/data", []() {
        StaticJsonDocument<256> doc;
        if (xSemaphoreTake(xVitalsMutex, pdMS_TO_TICKS(10))) {
            doc["temp"] = latestVitals.temperature;
            doc["ecg"] = latestVitals.ecgValue;
            doc["status"] = latestVitals.systemHealthy ? "OK" : "ERROR";
            xSemaphoreGive(xVitalsMutex);
        }
        String response;
        serializeJson(doc, response);
        server.send(200, "application/json", response);
    });
    server.begin();
    for (;;) {
        server.handleClient();
        vTaskDelay(pdMS_TO_TICKS(20));
    }
}

void loop() { vTaskDelete(NULL); }