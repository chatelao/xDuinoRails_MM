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
   - **Note:** BEMF-based termination and load compensation are controlled by **CV 49 (Bit 0)**.

## Load Compensation (PI Controller)

When BEMF is enabled, the firmware uses a PI controller to maintain the target speed under load.

### Algorithm
1. **Target BEMF:** Calculated as `targetPwm * 4` (mapping 10-bit PWM to 12-bit BEMF).
2. **Sampling:** BEMF is measured every `BEMF_SAMPLE_INT` (10-15ms).
3. **Control Formula:** `adjustment = (error * K) / 16 + (errorSum * I) / 64`
   - **K (CV 54):** Proportional gain.
   - **I (CV 55):** Integral gain.
4. **Windup Protection:** `errorSum` is clamped to ±10000.

## Analysis of Motor Behavior (Case Study)

Reported strange behavior:
- **Speed 5:** Barely moves
- **Speed 6:** Moves fast
- **Speed 7:** Moves at maximal speed

### Calculated PWM Mapping (Default CVs: 2=10, 5=0, 6=0)

| MM Step | PWM Value | Duty Cycle | Notes |
| :--- | :--- | :--- | :--- |
| 1 | 40 | 3.9% | Vstart |
| 2 | 121 | 11.8% | |
| 3 | 203 | 19.8% | |
| 4 | 285 | 27.8% | |
| 5 | 367 | 35.9% | Reported: Barely moves |
| 6 | 449 | 43.9% | Reported: Fast |
| 7 | 531 | 51.9% | Vmid (Midpoint between 1 and 14) - Reported: Maximal speed |
| 8 | 601 | 58.7% | |
| 14 | 1023 | 100.0% | Vhigh |

### Analytic Findings

The observed behavior is a result of the physical motor characteristics interacting with the default 3-point speed curve:

1.  **High Starting Friction:** The motor requires approx. 35-40% PWM (Step 5) to overcome internal friction. This explains why it "barely moves" at Step 5.
2.  **Early Saturation:** Many model railroad motors reach their perceived maximum physical speed (or the limit of the gear ratio) at around 50-60% duty cycle. In the default configuration, **Step 7** already provides **52% PWM**. To the user, there is no noticeable speed increase between Step 7 and Step 14, leading to the perception that Step 7 is already "maximal speed".
3.  **Non-Linear Response:** The jump from 36% (Step 5) to 44% (Step 6) and 52% (Step 7) is relatively small in terms of PWM, but happens to be in the most sensitive region of this specific motor's RPM curve.

### Recommendations for Calibration

To fix this for this specific locomotive, the user should:
1.  **Increase CV 2 (Start Voltage):** Set to approx. 80-90 so that Step 1 already overcomes friction.
2.  **Decrease CV 5 (Maximum Speed):** Set to approx. 120-150 to map Step 14 to the physical maximum speed, providing better resolution over the lower speed range.
3.  **Adjust CV 6 (Medium Speed):** Use a value lower than the default midpoint to create a "logarithmic" curve, giving more steps in the slow speed range.

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
