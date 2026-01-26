#ifndef ADAFRUIT_NEOPIXEL_H
#define ADAFRUIT_NEOPIXEL_H

#include <cstdint>

class Adafruit_NeoPixel {
public:
    Adafruit_NeoPixel(uint16_t n, int16_t p, uint8_t t = 0);
    void begin();
    void show();
    void setPixelColor(uint16_t n, uint32_t c);
    void clear();
};

#endif // ADAFRUIT_NEOPIXEL_H
