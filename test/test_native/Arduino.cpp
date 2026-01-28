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

unsigned long mock_millis_value = 0;

unsigned long millis() {
  return mock_millis_value;
}

void delay(unsigned long ms) {
  mock_millis_value += ms;
}

void delayMicroseconds(unsigned int us) {
  // Mock implementation
}

int digitalPinToInterrupt(int pin) {
  // Mock implementation, just return the pin
  return pin;
}

void attachInterrupt(int interrupt, void (*isr)(), int mode) {
  // Mock implementation, do nothing
}
