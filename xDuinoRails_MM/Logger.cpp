#include "Logger.h"
#include <stdarg.h>
#include <stdio.h>

Logger logger;

Logger::Logger()
    : cvManager(nullptr), cachedEnabled(false), protocolEnabled(true),
      pwmEnabled(true), cvEnabled(true), bemfEnabled(true),
      isInitialized(false) {}

void Logger::begin(CvManager *cvManager, unsigned long baudRate) {
  this->cvManager = cvManager;
  cachedEnabled   = isEnabled();
  isInitialized   = true;
  DEBUG_SERIAL.begin(baudRate);
}

void Logger::toggleLogging() {
  cachedEnabled = !cachedEnabled;
  if (cvManager) {
    cvManager->setCv(CV_DEBUG_ENABLE, cachedEnabled ? 1 : 0);
  }
}

void Logger::toggleCategory(LogCategory category) {
  switch (category) {
  case LogCategory::Protocol:
    protocolEnabled = !protocolEnabled;
    break;
  case LogCategory::PWM:
    pwmEnabled = !pwmEnabled;
    break;
  case LogCategory::CV:
    cvEnabled = !cvEnabled;
    break;
  case LogCategory::BEMF:
    bemfEnabled = !bemfEnabled;
    break;
  default:
    break;
  }
}

bool Logger::isLoggingEnabled() { return cachedEnabled; }

bool Logger::isCategoryEnabled(LogCategory category) {
  switch (category) {
  case LogCategory::Protocol:
    return protocolEnabled;
  case LogCategory::PWM:
    return pwmEnabled;
  case LogCategory::CV:
    return cvEnabled;
  case LogCategory::BEMF:
    return bemfEnabled;
  default:
    return true;
  }
}

bool Logger::isEnabled() {
  if (cvManager == nullptr)
    return false;
  return cvManager->getCv(CV_DEBUG_ENABLE) != 0;
}

void Logger::print(const char *message, LogCategory category) {
  if (isInitialized && cachedEnabled && isCategoryEnabled(category)) {
    DEBUG_SERIAL.print(message);
  }
}

void Logger::println(const char *message, LogCategory category) {
  if (isInitialized && cachedEnabled && isCategoryEnabled(category)) {
    DEBUG_SERIAL.println(message);
  }
}

void Logger::printf(LogCategory category, const char *format, ...) {
  if (isInitialized && cachedEnabled && isCategoryEnabled(category)) {
    char    buf[128];
    va_list args;
    va_start(args, format);
    vsnprintf(buf, sizeof(buf), format, args);
    va_end(args);
    DEBUG_SERIAL.print(buf);
  }
}

void Logger::printf(const char *format, ...) {
  if (isInitialized && cachedEnabled && isCategoryEnabled(LogCategory::General)) {
    char    buf[128];
    va_list args;
    va_start(args, format);
    vsnprintf(buf, sizeof(buf), format, args);
    va_end(args);
    DEBUG_SERIAL.print(buf);
  }
}
