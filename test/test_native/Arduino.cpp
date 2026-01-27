#include "Arduino.h"
#include <iostream>

void          pinMode(int pin, int mode) {}
void          digitalWrite(int pin, int value) {}
void          analogWrite(int pin, int value) {}
void          delayMicroseconds(int us) {}
int           analogRead(int pin) { return 0; }
unsigned long millis() { return 0; }

void analogWriteFreq(int freq) {}
void analogWriteRange(int range) {}
