#include "MotorControl.h"

const int PWM_RANGE = 1023;

MotorControl::MotorControl(CvManager &cvManager, int pinA, int pinB, int bemfA,
                           int bemfB)
    : cvManager(cvManager) {
  pinA_priv           = pinA;
  pinB_priv           = pinB;
  bemfA_priv          = bemfA;
  bemfB_priv          = bemfB;
  targetPwm           = 0;
  currDirection       = MM2DirectionState_Forward;
  targetDirection     = MM2DirectionState_Forward;
  isKickstarting_priv = false;
  kickstartBegin      = 0;
  lastBemfMeasure     = 0;
  lastSpeed           = 0;
  previousPwm         = 0;

  int motorType = cvManager.getCv(CV_MOTOR_TYPE);

  switch (motorType) {
  case 1: // Faulhaber
    PWM_FREQ = 400;
    // PWM_MIN_MOVING is set from CV
    KICK_PWM        = 1023;
    KICK_MAX_TIME   = 150;
    BEMF_THRESHOLD  = 120;
    BEMF_SAMPLE_INT = 15;
    break;
  case 2: // Maxon
    PWM_FREQ = 20000;
    // PWM_MIN_MOVING is set from CV
    KICK_PWM        = 600;
    KICK_MAX_TIME   = 80;
    BEMF_THRESHOLD  = 80;
    BEMF_SAMPLE_INT = 10;
    break;
  default: // Standard DC
    PWM_FREQ = 20000;
    // PWM_MIN_MOVING is set from CV
    KICK_PWM        = 800;
    KICK_MAX_TIME   = 100;
    BEMF_THRESHOLD  = 100;
    BEMF_SAMPLE_INT = 12;
    break;
  }
}

void MotorControl::setup() {
  pinMode(pinA_priv, OUTPUT);
  pinMode(pinB_priv, OUTPUT);
  pinMode(bemfA_priv, INPUT);
  pinMode(bemfB_priv, INPUT);

#ifdef ARDUINO_ARCH_RP2040
  analogWriteFreq(PWM_FREQ);
  analogWriteRange(PWM_RANGE);
#elif defined(ARDUINO_ARCH_ESP32)
  analogWriteFrequency(pinA_priv, PWM_FREQ);
  analogWriteFrequency(pinB_priv, PWM_FREQ);
  analogWriteResolution(pinA_priv, 10); // 10 bits = 1023
  analogWriteResolution(pinB_priv, 10); // 10 bits = 1023
#endif

  writeMotorHardware(0, MM2DirectionState_Forward);
}

const int PWM_MAX = 1023;

void MotorControl::setSpeed(int step, MM2DirectionState dir) {
  if (step == 0) {
    update(0, dir);
    return;
  }

  // Map CVs (0-255) to PWM (0-1023)
  int vStart = map(cvManager.getCv(CV_START_VOLTAGE), 0, 255, 0, PWM_MAX);
  int vHigh  = map(cvManager.getCv(CV_MAXIMUM_SPEED), 0, 255, 0, PWM_MAX);
  int vMid   = map(cvManager.getCv(CV_MEDIUM_SPEED), 0, 255, 0, PWM_MAX);

  // Defaults if CVs are 0
  if (vHigh == 0)
    vHigh = PWM_MAX;
  if (vStart == 0)
    vStart = 1;

  // Clamp vStart to vHigh to avoid negative slopes
  if (vStart > vHigh)
    vStart = vHigh;

  if (vMid == 0) {
    vMid = (vStart + vHigh) / 2;
  } else {
    // Clamp vMid between vStart and vHigh
    if (vMid < vStart)
      vMid = vStart;
    if (vMid > vHigh)
      vMid = vHigh;
  }

  int pwm;
  if (step <= 7) {
    pwm = map(step, 1, 7, vStart, vMid);
  } else {
    pwm = map(step, 7, 14, vMid, vHigh);
  }

  update(pwm, dir);
}

void MotorControl::update(int pwm, MM2DirectionState dir) {
  targetPwm         = pwm;
  targetDirection   = dir;
  unsigned long now = millis();

  // Bei Stillstand Richtung sofort übernehmen
  if (targetPwm == 0)
    currDirection = targetDirection;

  if (previousPwm == 0 && targetPwm > 0 && KICK_MAX_TIME > 0) {
    isKickstarting_priv = true;
    kickstartBegin      = now;
    lastBemfMeasure     = 0;
  }
  if (targetPwm == 0) {
    isKickstarting_priv = false;
  }

  if (isKickstarting_priv) {
    if (now - kickstartBegin >= KICK_MAX_TIME) {
      isKickstarting_priv = false;
    } else {
      if (now - lastBemfMeasure > BEMF_SAMPLE_INT) {
        int currentBEMF = readBEMF();
        lastBemfMeasure = now;
        if (currentBEMF > BEMF_THRESHOLD) {
          isKickstarting_priv = false;
        }
      }
      if (isKickstarting_priv) {
        writeMotorHardware(KICK_PWM, currDirection);
      }
    }
  }

  if (!isKickstarting_priv) {
    if (currDirection != targetDirection) {
      writeMotorHardware(0, currDirection);
      currDirection = targetDirection;
    } else {
      writeMotorHardware(targetPwm, currDirection);
    }
  }
  previousPwm = targetPwm;
}

void MotorControl::stop() {
  targetPwm = 0;
  writeMotorHardware(0, currDirection);
}

bool MotorControl::isKickstarting() { return isKickstarting_priv; }

MM2DirectionState MotorControl::getCurrentDirection() { return currDirection; }

void MotorControl::writeMotorHardware(int pwm, MM2DirectionState dir) {
  if (pwm > PWM_RANGE)
    pwm = PWM_RANGE;
  if (pwm < 0)
    pwm = 0;

  if (dir == MM2DirectionState_Forward) {
    digitalWrite(pinB_priv, LOW);
    analogWrite(pinA_priv, pwm);
  } else {
    digitalWrite(pinA_priv, LOW);
    analogWrite(pinB_priv, pwm);
  }
}

int MotorControl::readBEMF() {
  digitalWrite(pinA_priv, LOW);
  digitalWrite(pinB_priv, LOW);
  delayMicroseconds(500);
  int valA = analogRead(bemfA_priv);
  int valB = analogRead(bemfB_priv);
  return (valA > valB) ? (valA - valB) : (valB - valA);
}
