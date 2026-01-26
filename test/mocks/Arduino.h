#ifndef ARDUINO_H
#define ARDUINO_H

#include <cstdint>
#include <cmath>
#include <chrono>
#include <thread>

#define HIGH 0x1
#define LOW  0x0

#define INPUT 0x0
#define OUTPUT 0x1

// Mock functions
void pinMode(uint8_t pin, uint8_t mode);
void digitalWrite(uint8_t pin, uint8_t val);
long map(long x, long in_min, long in_max, long out_min, long out_max);
void delay(unsigned long ms);
unsigned long millis();

#endif // ARDUINO_H
