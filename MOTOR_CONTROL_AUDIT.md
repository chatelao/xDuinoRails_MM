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

## Analysis of Motor Behavior (Case Study)

Reported strange behavior:
- **Speed 5:** Barely moves
- **Speed 6:** Moves fast
- **Speed 7:** Moves at maximal speed

### Calculated PWM Mapping (New Standard Defaults: CV 2=1, 5=255, 6=127)

| MM Step | PWM Value | Duty Cycle | Notes |
| :--- | :--- | :--- | :--- |
| 1 | 4 | 0.4% | Vstart |
| 4 | 256 | 25.0% | |
| 7 | 509 | 49.8% | Vmid |
| 11 | 802 | 78.4% | |
| 14 | 1023 | 100.0% | Vhigh |

### Analytic Findings

The previously observed behavior where the motor maxed out at step 7 was due to suboptimal default CV settings (`CV 2=85`, `CV 5=140`, `CV 6=105`). These settings restricted the speed range and resolution.

The new standard defaults (`CV 2=1`, `CV 5=255`, `CV 6=127`) provide:
1.  **Full Range:** The motor can be driven from minimum to maximum power.
2.  **Linear Response:** The default curve is linear, providing consistent speed increments across all 14 steps.
3.  **Maximum Resolution:** Users can now utilize all 14 Märklin Motorola speed steps effectively.

For motors with high starting friction or early saturation, users can still calibrate the decoder by adjusting CVs 2, 5, and 6.
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
