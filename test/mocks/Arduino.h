#ifndef ARDUINO_H
#define ARDUINO_H

#include <cstdint>

#define HIGH 0x1
#define LOW  0x0

#define INPUT 0x0
#define OUTPUT 0x1
#define INPUT_PULLUP 0x2
#define CHANGE 1

void attachInterrupt(uint8_t pin, void (*isr)(), int mode);
uint8_t digitalPinToInterrupt(uint8_t pin);
void pinMode(uint8_t pin, uint8_t mode);
void digitalWrite(uint8_t pin, uint8_t val);
int analogRead(uint8_t pin);
void analogWrite(uint8_t pin, int val);
void analogWriteFreq(int freq);
void analogWriteRange(int range);

unsigned long millis();
void delayMicroseconds(unsigned int us);

extern unsigned long mock_millis;

#endif // ARDUINO_H
