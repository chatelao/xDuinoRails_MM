#include "Arduino.h"
#include <Adafruit_NeoPixel.h>

Adafruit_NeoPixel *Adafruit_NeoPixel::lastInstance = nullptr;

std::map<uint8_t, int> analog_write_values;
std::map<uint8_t, int> digital_write_values;
std::map<uint8_t, int> analog_read_values;

static unsigned long current_millis = 0;

void pinMode(uint8_t pin, uint8_t mode) {
  // Mock implementation
}

void digitalWrite(uint8_t pin, uint8_t val) { digital_write_values[pin] = val; }

int analogRead(uint8_t pin) {
  if (analog_read_values.count(pin))
    return analog_read_values[pin];
  return 0;
}

void analogWrite(uint8_t pin, int val) { analog_write_values[pin] = val; }

void analogWriteFreq(int freq) {
  // Mock implementation
}

void analogWriteRange(int range) {
  // Mock implementation
}

unsigned long millis() { return current_millis; }

void delay(unsigned long ms) { current_millis += ms; }

void advance_millis(unsigned long ms) { current_millis += ms; }

void delayMicroseconds(unsigned int us) {
  // Mock implementation
}

void reset_arduino_mock() {
  analog_write_values.clear();
  digital_write_values.clear();
  analog_read_values.clear();
  current_millis = 0;
}

long map(long x, long in_min, long in_max, long out_min, long out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

MockSerial Serial;

void MockSerial::begin(unsigned long baud) {}
void MockSerial::print(const char *s) { printf("%s", s); }
void MockSerial::print(int n) { printf("%d", n); }
void MockSerial::println(const char *s) { printf("%s\n", s); }
void MockSerial::println(int n) { printf("%d\n", n); }
