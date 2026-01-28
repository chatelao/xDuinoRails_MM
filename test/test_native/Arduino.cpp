#include "Arduino.h"

std::map<uint8_t, uint8_t> mock_digital_write_state;
std::map<uint8_t, int>     mock_analog_write_state;
unsigned long              mock_millis_time = 0;

void reset_arduino_mocks() {
	mock_digital_write_state.clear();
	mock_analog_write_state.clear();
	mock_millis_time = 0;
}

void pinMode(uint8_t pin, uint8_t mode) {
  // Mock implementation
}

void digitalWrite(uint8_t pin, uint8_t val) {
	mock_digital_write_state[pin] = val;
}

int analogRead(uint8_t pin) {
  // Mock implementation
  return 0;
}

void analogWrite(uint8_t pin, int val) {
	mock_analog_write_state[pin] = val;
}

void analogWriteFreq(int freq) {
  // Mock implementation
}

void analogWriteRange(int range) {
  // Mock implementation
}

unsigned long millis() { return mock_millis_time; }

void advance_millis(unsigned long ms) { mock_millis_time += ms; }

void delayMicroseconds(unsigned int us) {
  // Mock implementation
}

long map(long x, long in_min, long in_max, long out_min, long out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}
