# Health Guardian: ESP32 Real-Time Health Monitoring System

Health Guardian is a high-performance, RTOS-based embedded system designed for medical-grade data acquisition. Unlike standard hobbyist projects, this system utilizes a **Preemptive Multitasking Architecture** to ensure deterministic sampling of vital signs (ECG, SpO2, Heart Rate) while simultaneously hosting a web-based visualization dashboard.

## Engineering Features
* **Deterministic Sampling:** 360Hz ECG acquisition using Hardware Timer Interrupts (ISR) to eliminate software-induced jitter.
* **Multithreaded Execution:** Leverages Dual-Core ESP32 processing using FreeRTOS Tasks and Mutex-guarded shared memory.
* **Digital Signal Processing (DSP):** Real-time 1st-order IIR filtering for EMG noise attenuation in ECG signals.
* **Fault Tolerance:** Integrated Task Watchdog Timer (WDT) and Power-On Self-Test (POST) for sensor health validation.
* **Power Optimization:** Deep Sleep implementation with motion-triggered wake-up (MPU6050 Interrupts).

##  Tech Stack
- **SoC:** ESP32 (Xtensa Dual-Core)
- **Language/Framework:** C++ / FreeRTOS / ESP-IDF
- **Protocols:** I2C, SPI, UART, HTTP, JSON
- **Sensors:** MAX30102 (SpO2), AD8232 (ECG), MPU6050 (IMU), DS18B20 (Temp)

##  System Architecture
The firmware is split into three priority levels:
1. **ISR (Priority: Critical):** Hardware timer-driven ECG sampling.
2. **Task A (Priority: High):** Bio-sensor data processing (I2C).
3. **Task B (Priority: Low):** WiFi stack and JSON API handling.
