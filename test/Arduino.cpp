#include "Arduino.h"
#include <chrono>
#include <thread>

static auto start_time = std::chrono::high_resolution_clock::now();

void delay(uint32_t ms) {
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

unsigned long millis() {
    auto now = std::chrono::high_resolution_clock::now();
    return std::chrono::duration_cast<std::chrono::milliseconds>(now - start_time).count();
}

void attachInterrupt(uint8_t pin, void (*isr)(), int mode) {
    // Do nothing in mock
}

int digitalPinToInterrupt(int pin) {
    return pin;
}
