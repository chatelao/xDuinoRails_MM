#include "Logger.h"
#include <stdarg.h>
#include <stdio.h>

Logger logger;

Logger::Logger()
    : cvManager(nullptr), cachedEnabled(false), isInitialized(false) {}

void Logger::begin(CvManager *cvManager, unsigned long baudRate) {
  this->cvManager = cvManager;
  cachedEnabled   = isEnabled();
  isInitialized   = true;
  Serial.begin(baudRate);
}

void Logger::toggleLogging() {
  cachedEnabled = !cachedEnabled;
  if (cvManager) {
    cvManager->setCv(CV_DEBUG_ENABLE, cachedEnabled ? 1 : 0);
  }
}

bool Logger::isLoggingEnabled() { return cachedEnabled; }

bool Logger::isEnabled() {
  if (cvManager == nullptr)
    return false;
  return cvManager->getCv(CV_DEBUG_ENABLE) != 0;
}

void Logger::print(const char *message) {
  if (isInitialized && cachedEnabled) {
    Serial.print(message);
  }
}

void Logger::println(const char *message) {
  if (isInitialized && cachedEnabled) {
    Serial.println(message);
  }
}

void Logger::printf(const char *format, ...) {
  if (isInitialized && cachedEnabled) {
    char    buf[128];
    va_list args;
    va_start(args, format);
    vsnprintf(buf, sizeof(buf), format, args);
    va_end(args);
    Serial.print(buf);
  }
}
