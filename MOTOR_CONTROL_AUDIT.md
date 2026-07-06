# Motor Control Audit

This document provides a technical audit of the motor control algorithm implementation in the xDuinoRails_MM firmware. It covers speed mapping, motor profiles, hardware interaction, and safety mechanisms.

## Speed Curve Mapping

The firmware implements a 3-point piecewise linear speed curve for the 14 Märklin Motorola (MM) speed steps.

### CV Configuration
* **CV 2 (Start Voltage):** Defines the PWM value for speed step 1.
* **CV 5 (Maximum Speed):** Defines the PWM value for speed step 14.
* **CV 6 (Medium Speed):** Defines the PWM value for speed step 7.

### Implementation Details
* **Scaling:** CV values (0-255) are mapped to 10-bit PWM values (0-1023).
* **Step 0:** Always results in PWM 0.
* **Interpolation:**
  * Steps 1 to 7: Linear interpolation between `Vstart` and `Vmid`.
  * Steps 7 to 14: Linear interpolation between `Vmid` and `Vhigh`.
* **Defaults and Clamping:**
  * If `Vhigh` is 0, it defaults to 1023.
  * If `Vstart` is 0, it defaults to 1.
  * `Vstart` is clamped to `Vhigh`.
  * If `Vmid` is 0, it is calculated as `(Vstart + Vhigh) / 2`.
  * `Vmid` is clamped between `Vstart` and `Vhigh`.

## Motor Profiles (CV 52)

The firmware supports different motor types through hardcoded profiles selectable via **CV 52**.

| CV 52 Value | Motor Type  | PWM Frequency | Kick PWM | Kick Max Time | BEMF Threshold | BEMF Sample Int |
| :---        | :---        | :---          | :---     | :---          | :---           | :---            |
| 0 (Default) | Standard DC | 20 kHz        | 800      | 100 ms        | 100            | 12 ms           |
| 1           | Faulhaber   | 400 Hz        | 1023     | 150 ms        | 120            | 15 ms           |
| 2           | Maxon       | 20 kHz        | 600      | 80 ms         | 80             | 10 ms           |

### Hardware Setup
PWM frequency and resolution (10-bit) are configured during `MotorControl::setup()` based on the selected profile. The implementation supports both RP2040 and ESP32 architectures.

## Kickstart and BEMF Logic

To overcome initial motor friction, the firmware implements a kickstart phase when the motor starts from a standstill.

### Kickstart Phase
* **Trigger:** Triggered when the previous PWM was 0 and the new target PWM is greater than 0.
* **Operation:** The motor is driven at a high PWM value (`KICK_PWM`) defined in the motor profile.
* **Visual Feedback:** The NeoPixel flashes white during kickstart (logic in `DebugLeds`).

### BEMF Measurement
* **Function:** `MotorControl::readBEMF()`
* **Mechanism:**
  1. Both motor pins are set to `LOW`.
  2. A blanking delay of **500 microseconds** (`delayMicroseconds(500)`) is applied to allow inductive spikes to decay.
  3. Analog values are sampled from both BEMF pins.
  4. The absolute difference between the samples is returned.

### Termination Conditions
The kickstart phase ends immediately when either of these conditions is met:
1. **Timeout:** The elapsed time exceeds `KICK_MAX_TIME` (80-150 ms depending on profile).
2. **BEMF Threshold:** The measured BEMF value exceeds `BEMF_THRESHOLD` (80-120 depending on profile), indicating the motor has started turning.
   - **Note:** BEMF-based termination can be disabled via **CV 49 (Bit 0)**. If bit 0 is cleared, only the timeout condition will terminate the kickstart.

## Load Compensation (PI Controller)

Active speed regulation is implemented using a PI controller when **CV 49 (Bit 0)** is enabled.

### Algorithm
1. **Target BEMF:** The target PWM (0-1023) is scaled to the 12-bit ADC range (`targetBEMF = targetPwm * 4`).
2. **Measured BEMF:** Obtained via `readBEMF()` at every `BEMF_SAMPLE_INT` interval.
3. **Error Calculation:** `error = targetBEMF - measuredBEMF`.
4. **PI Calculation:**
   - **P-Term:** `error * (CV_54 / 32.0)`
   - **I-Term:** `integral * (CV_55 / 128.0)`
   - **Adjustment:** `P-Term + I-Term`
5. **Output:** `finalPwm = targetPwm + Adjustment` (clamped to 0-1023).

### Parameters
* **CV 54 (K):** Proportional component (Range 0-63, Default 32).
* **CV 55 (I):** Integral component (Range 0-63, Default 24).
* **Anti-Windup:** The integral term is clamped to +/- 2000 to prevent runaway effects.

## Signal Loss Watchdog

The firmware includes a safety watchdog to stop the locomotive if the command signal is lost.

### Operation
* **Watchdog Timeout:** Configured via **CV 11** (default 5, value in 100ms units, i.e., 500ms).
* **Signal Monitoring:** The `ProtocolHandler` updates the `lastSignalTime` whenever a valid packet is parsed.

### Ramp-Down Implementation
The logic is implemented in the main `loop()` of `xDuinoRails_MM.ino`:
1. If `protocol.isSignalTimeout()` is true:
   - The time elapsed since the watchdog timeout is calculated.
   - If the elapsed time is **>= 500ms**, the motor is stopped immediately (`motor.stop()`).
   - If the elapsed time is **< 500ms**, a linear ramp-down is applied:
     - `rampSpeed = map(elapsed, 0, 500, targetSpeed, 0)`
     - This results in a smooth, forced deceleration to zero over a fixed 500ms window after the watchdog period expires.

## Unimplemented Features

The following features are currently documented in the CV table but have no logic in the motor control implementation:

* **CV 3 (Acceleration Rate):** The speed mapping logic calculates the target PWM immediately. There is no ramp logic for acceleration in `MotorControl.cpp`.
* **CV 4 (Braking Time):** There is no ramp logic for deceleration during normal operation. Deceleration is only implemented during signal loss (as a fixed 500ms ramp).
