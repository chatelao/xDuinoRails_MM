#include "Arduino.h"

void pinMode(uint8_t pin, uint8_t mode) {}
void digitalWrite(uint8_t pin, uint8_t val) {}
int analogRead(uint8_t pin) { return 0; }
void analogReadResolution(int res) {}
void delay(unsigned long ms) {}
unsigned long millis() { return 0; }

MockStream Serial;
