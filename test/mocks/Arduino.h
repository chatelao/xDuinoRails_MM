#ifndef ARDUINO_H
#define ARDUINO_H

#include <cstdint>

#define HIGH 0x1
#define LOW  0x0

#define INPUT 0x0
#define OUTPUT 0x1
#define INPUT_PULLUP 0x2

void pinMode(uint8_t pin, uint8_t mode);
void digitalWrite(uint8_t pin, uint8_t val);
int analogRead(uint8_t pin);
void analogWrite(uint8_t pin, int val);
void analogWriteFreq(int freq);
void analogWriteRange(int range);

#define CHANGE 1

extern unsigned long mock_millis_value;
unsigned long millis();
void delay(unsigned long ms);
void delayMicroseconds(unsigned int us);
int digitalPinToInterrupt(int pin);
void attachInterrupt(int interrupt, void (*isr)(), int mode);

#endif // ARDUINO_H
