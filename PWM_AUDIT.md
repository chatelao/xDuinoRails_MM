# PWM Audit: Speed to PWM Mapping

This document describes how the Märklin Motorola (MM) speed steps are currently mapped to PWM values in the `MotorControl` class.

## Overview

- **Input:** MM speed steps (0 to 14).
- **Output:** PWM duty cycle (0 to 1023).
- **PWM Frequency:** Depends on motor type (400 Hz or 20 kHz).

## Mapping Logic

The mapping is implemented in `MotorControl::setSpeed(int step, MM2DirectionState dir)`.

### Special Cases

- **Step 0:** Always results in **PWM 0** (Stop).
- **Step 14:** Always results in **PWM 1023** (Full Speed).

### Standard Mapping (Steps 1 to 14)

For steps between 1 and 14, the PWM value is calculated using the Arduino `map` function based on the `CV_START_VOLTAGE` (CV 2).

**Formula:**
```cpp
int pwmMinMoving = CV_START_VOLTAGE * 40;
if (pwmMinMoving == 0) pwmMinMoving = 1;
int pwm = map(step, 1, 14, pwmMinMoving, 1023);
```

### Table of Sample Values (with CV 2 = 1)

With `CV_START_VOLTAGE = 1`, `pwmMinMoving = 40`.

| MM Speed Step | PWM Value (Approx.) |
| :--- | :--- |
| 0 | 0 |
| 1 | 40 |
| 2 | 115 |
| 3 | 191 |
| 4 | 266 |
| 5 | 342 |
| 6 | 417 |
| 7 | 493 |
| 8 | 569 |
| 9 | 644 |
| 10 | 720 |
| 11 | 795 |
| 12 | 871 |
| 13 | 946 |
| 14 | 1023 |

## Observed Issues / Notes

1.  **CV 5 (Maximum Speed) is ignored:** Currently, the upper bound of the mapping is always fixed at 1023. `CV_MAXIMUM_SPEED` is defined in `CvManager` but not used in the `MotorControl` scaling logic.
2.  **CV 2 Multiplier:** The `CV_START_VOLTAGE` is multiplied by 40 to determine the minimum PWM. Since the max CV value is 255, this could lead to values far exceeding the PWM range (255 * 40 = 10200). However, the `map` function will still work, but it might produce unexpected results if `pwmMinMoving >= 1023`.
3.  **Kickstart:** When starting from a standstill (PWM 0 -> PWM > 0), a "Kickstart" pulse is applied. The PWM value during kickstart depends on the motor type:
    - **Faulhaber:** PWM 1023 for up to 150ms.
    - **Maxon:** PWM 600 for up to 80ms.
    - **Standard DC:** PWM 800 for up to 100ms.
    Kickstart ends early if the BEMF threshold is reached.
