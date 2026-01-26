#ifndef ARDUINO_MOCK_H
#define ARDUINO_MOCK_H

#include <cstdint>

#define CHANGE 1

void analogWriteFreq(int freq);
void analogWriteRange(int range);
unsigned long millis();
void attachInterrupt(uint8_t pin, void (*isr)(void), int mode);
int digitalPinToInterrupt(uint8_t pin);

extern unsigned long mock_millis;

#endif // ARDUINO_MOCK_H
