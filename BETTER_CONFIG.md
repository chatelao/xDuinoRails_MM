# Better Motor Configuration and Tuning

This document describes how to use the high-speed logging features to analyze and optimize the motor control performance of the xDuinoRails_MM firmware.

## Motor Control Weaknesses

Common issues in model railroad motor control include:
1.  **Oscillations:** The locomotive jerks back and forth at a constant speed, indicating the PI gains (CV 54/55) are too high.
2.  **Sluggish Response:** The locomotive takes too long to reach the target speed or slows down significantly under load, indicating gains are too low.
3.  **Low-Speed Stalling:** The motor doesn't have enough torque to overcome friction at speed step 1.
4.  **Non-Linearity:** The transition between kickstart and regulated speed is not smooth.

## High-Speed CSV Logging

To diagnose these issues, use the High-Speed Logging mode. This mode outputs raw control loop data at every sampling interval (typically every 10-15ms).

### Enabling High-Speed Logging
In the Serial Console (115200 baud), type:
```
l h
```
This toggles the high-speed mode. Note that this generates a lot of serial traffic.

### CSV Format
The output is prefixed with `CSV,` and follows this structure:
`CSV,timestamp,targetPwm,currentBEMF,error,integral,adjustment`

*   **timestamp:** Milliseconds since boot.
*   **targetPwm:** The PWM value calculated from the speed curve (0-1023).
*   **currentBEMF:** The raw 12-bit ADC value read from the motor (0-4095).
*   **error:** The difference between `targetBEMF` (targetPwm * 4) and `currentBEMF`.
*   **integral:** The accumulated error sum (clamped to ±10000).
*   **adjustment:** The final correction value added to `targetPwm`.

## Tuning Procedure

1.  **Baseline:** Set CV 54 (K) and CV 55 (I) to default values (e.g., K=32, I=24).
2.  **Capture Data:** Enable high-speed logging (`l h`) and start the locomotive at a medium speed step (e.g., `s 7`).
3.  **Analyze Step Response:**
    *   Observe how quickly `currentBEMF` reaches the target.
    *   If `currentBEMF` overshoots and oscillates, **reduce CV 54 (K)**.
    *   If `currentBEMF` stays below target for a long time, **increase CV 55 (I)**.
4.  **Load Test:** Apply physical resistance to the locomotive and observe the `adjustment` and `integral` values. They should increase to compensate for the load.
5.  **Iterate:** Adjust CVs and repeat until the locomotive runs smoothly at all speeds.

## Suggested Values per Motor Type

| Motor Type | CV 52 | CV 54 (K) | CV 55 (I) |
| :--- | :--- | :--- | :--- |
| Standard DC | 0 | 32 | 24 |
| Faulhaber | 1 | 16 | 12 |
| Maxon | 2 | 48 | 32 |

*Note: These are starting points; every physical motor is unique.*
