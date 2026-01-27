#ifndef ARDUINO_MOCK_H
#define ARDUINO_MOCK_H

#include <cstdint>

#define HIGH   0x1
#define LOW    0x0
#define OUTPUT 0x1
#define INPUT  0x2

void          analogWriteFreq(int freq);
void          analogWriteRange(int range);
void          pinMode(int pin, int mode);
void          digitalWrite(int pin, int value);
void          analogWrite(int pin, int value);
void          delayMicroseconds(int us);
int           analogRead(int pin);
unsigned long millis();

#endif // ARDUINO_MOCK_H
