#include "Arduino.h"

unsigned long mock_millis = 0;

void analogWriteFreq(int freq) {}
void analogWriteRange(int range) {}

unsigned long millis() {
    return mock_millis++;
}

void attachInterrupt(uint8_t pin, void (*isr)(void), int mode) {}
int digitalPinToInterrupt(uint8_t pin) {
    return pin;
}
