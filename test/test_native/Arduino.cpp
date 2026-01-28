#include "Arduino.h"

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

unsigned long current_millis = 0;
unsigned long millis() { return current_millis; }
void          advance_millis(unsigned long ms) { current_millis += ms; }

void delayMicroseconds(unsigned int us) {
  // Mock implementation
}
