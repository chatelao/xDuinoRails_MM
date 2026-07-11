#include "Arduino.h"

std::map<uint8_t, int> analog_write_values;
std::map<uint8_t, int> digital_write_values;
std::map<uint8_t, int> analog_read_values;
std::map<uint8_t, std::deque<int>> analog_read_sequences;
int                    last_pwm_freq = 0;
std::map<uint8_t, int> last_esp32_pwm_freq;

static unsigned long current_millis = 0;

void pinMode(uint8_t pin, uint8_t mode) {
  // Mock implementation
}

void digitalWrite(uint8_t pin, uint8_t val) { digital_write_values[pin] = val; }

int analogRead(uint8_t pin) {
  if (analog_read_sequences.count(pin) && !analog_read_sequences[pin].empty()) {
    int val = analog_read_sequences[pin].front();
    analog_read_sequences[pin].pop_front();
    return val;
  }
  if (analog_read_values.count(pin)) {
    return analog_read_values[pin];
  }
  return 0;
}

void analogWrite(uint8_t pin, int val) { analog_write_values[pin] = val; }

void analogWriteFreq(int freq) { last_pwm_freq = freq; }

void analogWriteRange(int range) {
  // Mock implementation
}

void analogWriteFrequency(uint8_t pin, int freq) {
  last_esp32_pwm_freq[pin] = freq;
}

void analogWriteResolution(uint8_t pin, int res) {
  // Mock implementation
}

unsigned long millis() { return current_millis; }

void advance_millis(unsigned long ms) { current_millis += ms; }

void delay(unsigned long ms) { advance_millis(ms); }

void delayMicroseconds(unsigned int us) {
  // Mock implementation
}

void reset_arduino_mock() {
  analog_write_values.clear();
  digital_write_values.clear();
  analog_read_values.clear();
  analog_read_sequences.clear();
  last_pwm_freq = 0;
  last_esp32_pwm_freq.clear();
  current_millis = 0;
}

long map(long x, long in_min, long in_max, long out_min, long out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

MockSerial Serial;

void MockSerial::begin(unsigned long baud) {}
void MockSerial::print(const char *s) {
  printf("%s", s);
  logLines.push_back(std::string(s));
}
void MockSerial::print(int n) {
  printf("%d", n);
  logLines.push_back(std::to_string(n));
}
void MockSerial::println(const char *s) {
  printf("%s\n", s);
  logLines.push_back(std::string(s));
}
void MockSerial::println(int n) {
  printf("%d\n", n);
  logLines.push_back(std::to_string(n));
}
void MockSerial::clearLog() { logLines.clear(); }

int MockSerial::available() { return inputBuffer.size(); }

int MockSerial::read() {
  if (inputBuffer.empty())
    return -1;
  char c = inputBuffer.front();
  inputBuffer.pop_front();
  return (uint8_t)c;
}

size_t MockSerial::write(uint8_t c) {
  char s[2] = {(char)c, 0};
  logLines.push_back(std::string(s));
  return 1;
}

void MockSerial::pushInput(const std::string &s) {
  for (char c : s) {
    inputBuffer.push_back(c);
  }
}
