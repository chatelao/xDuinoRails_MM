# PWM Audit: Speed to PWM Mapping

This document describes how the Märklin Motorola (MM) speed steps are currently mapped to PWM values in the `MotorControl` class.

## Overview

- **Input:** MM speed steps (0 to 14).
- **Output:** PWM duty cycle (0 to 1023).
- **PWM Frequency:** Depends on motor type (400 Hz or 20 kHz).
- **Directional Lights:** PWM brightness control (0 to 1023) via CV 63 (0-7).

## Mapping Logic

The mapping is implemented in `MotorControl::setSpeed(int step, MM2DirectionState dir)`.

### Special Cases

- **Step 0:** Always results in **PWM 0** (Stop).
- **Step 14:** Always results in **PWM 1023** (Full Speed).

### Standard Mapping (Steps 1 to 14)

For steps between 1 and 14, the PWM value is calculated using a 3-point speed curve based on `CV_START_VOLTAGE` (CV 2), `CV_MEDIUM_SPEED` (CV 6), and `CV_MAXIMUM_SPEED` (CV 5).

**Formula Logic:**
1.  **CV to PWM Mapping:** CV values (0-255) are mapped to PWM (0-1023).
2.  **Vstart:** `map(CV_2, 0, 255, 0, 1023)`. Default (if 0) = 1.
3.  **Vhigh:** `map(CV_5, 0, 255, 0, 1023)`. Default (if 0) = 1023.
4.  **Vmid:** `map(CV_6, 0, 255, 0, 1023)`. Default (if 0) = `(Vstart + Vhigh) / 2`.
5.  **Interpolation:**
    - Steps 1-7: `map(step, 1, 7, Vstart, Vmid)`
    - Steps 7-14: `map(step, 7, 14, Vmid, Vhigh)`

### Table of Sample Values (Defaults: CV 2=10, CV 5=0, CV 6=0)

With `CV 2 = 10`, `Vstart ≈ 40`. `CV 5 = 0` (Vhigh = 1023). `CV 6 = 0` (Vmid ≈ 531).

| MM Speed Step | PWM Value (Approx.) |
| :--- | :--- |
| 0 | 0 |
| 1 | 40 |
| 2 | 121 |
| 3 | 203 |
| 4 | 285 |
| 5 | 367 |
| 6 | 449 |
| 7 | 531 |
| 8 | 601 |
| 9 | 671 |
| 10 | 741 |
| 11 | 812 |
| 12 | 882 |
| 13 | 952 |
| 14 | 1023 |

## Observed Issues / Notes

1.  **3-Point Curve:** Provides better control over non-linear motor responses.
2.  **Standardized Scaling:** All speed-related CVs now use the same 0-255 scaling factor.
3.  **Kickstart:** When starting from a standstill (PWM 0 -> PWM > 0), a "Kickstart" pulse is applied. The PWM value during kickstart depends on the motor type:
    - **Faulhaber:** PWM 1023 for up to 150ms.
    - **Maxon:** PWM 600 for up to 80ms.
    - **Standard DC:** PWM 800 for up to 100ms.
    Kickstart ends early if the BEMF threshold is reached.
