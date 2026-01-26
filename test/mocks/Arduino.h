#ifndef ARDUINO_H
#define ARDUINO_H

#include <cstdint>
#include <iostream>
#include <string>

#define HIGH 0x1
#define LOW  0x0

#define INPUT 0x0
#define OUTPUT 0x1
#define INPUT_PULLUP 0x2

void pinMode(uint8_t pin, uint8_t mode);
void digitalWrite(uint8_t pin, uint8_t val);
int analogRead(uint8_t pin);
void analogReadResolution(int res);
void delay(unsigned long ms);
unsigned long millis();

class MockStream {
public:
    void print(const std::string& s) { std::cout << s; }
    void println(const std::string& s) { std::cout << s << std::endl; }
};

extern MockStream Serial;

#endif // ARDUINO_H
