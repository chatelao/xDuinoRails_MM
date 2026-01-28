#include "Arduino.h"

void attachInterrupt(uint8_t pin, void (*isr)(), int mode) {
  // Mock implementation
}

uint8_t digitalPinToInterrupt(uint8_t pin) {
  // Mock implementation
  return pin;
}

void pinMode(uint8_t pin, uint8_t mode) {
  // Mock implementation
}

void digitalWrite(uint8_t pin, uint8_t val) {
  // Mock implementation
}

int analogRead(uint8_t pin) {
  // Mock implementation
  return 0;
}

void analogWrite(uint8_t pin, int val) {
  // Mock implementation
}

void analogWriteFreq(int freq) {
  // Mock implementation
}

void analogWriteRange(int range) {
  // Mock implementation
}

unsigned long mock_millis = 0;

unsigned long millis() {
  return mock_millis;
}

void delayMicroseconds(unsigned int us) {
  // Mock implementation
}
