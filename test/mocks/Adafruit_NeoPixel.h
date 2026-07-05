#ifndef ADAFRUIT_NEOPIXEL_H
#define ADAFRUIT_NEOPIXEL_H

#include <cstdint>
#include <vector>

#define NEO_GRB 0x01
#define NEO_KHZ800 0x02

class Adafruit_NeoPixel {
public:
  Adafruit_NeoPixel(uint16_t n, int16_t p, uint8_t t) : _n(n) {
    _colors.resize(n, 0);
    _showCount   = 0;
    lastInstance = this;
  }
  void begin() {}
  void show() { _showCount++; }
  void setPixelColor(uint16_t n, uint32_t c) {
    if (n < _n)
      _colors[n] = c;
  }
  void setBrightness(uint8_t b) {}
  void clear() {
    for (auto &c : _colors)
      c = 0;
  }

  static uint32_t Color(uint8_t r, uint8_t g, uint8_t b) {
    return ((uint32_t)r << 16) | ((uint32_t)g << 8) | b;
  }

  // Test helpers
  uint32_t getPixelColor(uint16_t n) const { return (n < _n) ? _colors[n] : 0; }
  int      getShowCount() const { return _showCount; }

  static Adafruit_NeoPixel *lastInstance;

private:
  uint16_t              _n;
  std::vector<uint32_t> _colors;
  int                   _showCount;
};

#endif // ADAFRUIT_NEOPIXEL_H
