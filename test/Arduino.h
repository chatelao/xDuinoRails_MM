#ifndef ARDUINO_H
#define ARDUINO_H

#include <cstdint>

#define CHANGE 1

// Mock implementations of Arduino functions
void delay(uint32_t ms);
unsigned long millis();
void attachInterrupt(uint8_t pin, void (*isr)(), int mode);
int digitalPinToInterrupt(int pin);

#endif // ARDUINO_H
