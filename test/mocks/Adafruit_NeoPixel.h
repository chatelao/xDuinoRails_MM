#ifndef ADAFRUIT_NEOPIXEL_H
#define ADAFRUIT_NEOPIXEL_H

#include <cstdint>
#include <vector>

#define NEO_GRB 0x01
#define NEO_KHZ800 0x02

class Adafruit_NeoPixel {
public:
  Adafruit_NeoPixel(uint16_t n, int16_t p, uint8_t t) : numPixels(n) {
    colors.resize(n, 0);
    showCount = 0;
  }
  void begin() {}
  void show() { showCount++; }
  void setPixelColor(uint16_t n, uint32_t c) {
    if (n < colors.size()) {
      colors[n] = c;
    }
  }
  void clear() {
    for (auto &c : colors)
      c = 0;
  }
  void setBrightness(uint8_t b) { brightness = b; }

  static uint32_t Color(uint8_t r, uint8_t g, uint8_t b) {
    return ((uint32_t)r << 16) | ((uint32_t)g << 8) | b;
  }

  uint32_t getPixelColor(uint16_t n) const {
    if (n < colors.size())
      return colors[n];
    return 0;
  }

  int getShowCount() const { return showCount; }

private:
  uint16_t              numPixels;
  std::vector<uint32_t> colors;
  uint8_t               brightness = 255;
  int                   showCount  = 0;
};

#endif // ADAFRUIT_NEOPIXEL_H
