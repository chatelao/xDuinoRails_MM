#ifndef ARDUINO_H
#define ARDUINO_H

#include <cstdint>
#include <map>
#include <string>

extern std::map<uint8_t, int> analog_write_values;
extern std::map<uint8_t, int> digital_write_values;

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

unsigned long millis();
void advance_millis(unsigned long ms);
void delayMicroseconds(unsigned int us);
void reset_arduino_mock();
long map(long, long, long, long, long);

#endif // ARDUINO_H
