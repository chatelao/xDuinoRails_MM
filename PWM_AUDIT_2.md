# PWM Audit 2: Motor Stall Analysis with BEMF Disabled

This document analyzes why motors may fail to move (stalling after the kickstart phase) when Back-EMF (BEMF) load compensation is disabled (CV 49, bit 0 = 0).

## 1. Speed to PWM Mapping (Default Settings)

When BEMF is disabled, the motor speed depends entirely on the open-loop PWM duty cycle. With the default factory settings (CV 2=10, CV 5=0, CV 6=0), the resulting PWM values for the 14 MM speed steps are:

| MM Step | PWM (10-bit) | Duty Cycle | Result |
| :--- | :--- | :--- | :--- |
| **0** | 0 | 0.0% | Stop |
| **Kickstart** | 800 | 78.2% | **High torque pulse** |
| **1 (Vstart)** | 40 | 3.9% | Stall likelihood: High |
| **2** | 121 | 11.8% | Stall likelihood: Moderate |
| **7 (Vmid)** | 531 | 51.9% | Crawl / Slow movement |
| **14 (Vhigh)** | 1023 | 100.0% | Full Speed |

### The "Drop-off" Effect
During the kickstart phase, the motor receives a **78.2% duty cycle** pulse. This is usually enough to overcome static friction and get the motor spinning. However, as soon as the kickstart timeout (default 100ms) expires, the system drops the duty cycle to the target speed value (e.g., **3.9%** for Step 1).

## 2. Technical Impact of PWM Frequency (20kHz)

The default PWM frequency for "Standard DC" motors is **20kHz**. While this provides silent operation, it has a significant impact on low-speed performance in open-loop mode:

1.  **Inductive Reactance:** At 20kHz, the high frequency increases the effective impedance of the motor windings.
2.  **Short Pulse Width:** At a 4% duty cycle and 20kHz, the "ON" time is only **2 microseconds**.
3.  **Current Rise Time:** Standard DC motors have significant inductance. A 2µs pulse is often too short for the current to rise to a level that produces sufficient magnetic flux to overcome the internal friction of the locomotive's gearbox.
4.  **BEMF Requirement:** Closed-loop control (BEMF enabled) normally compensates for this by automatically increasing the PWM (using the PI controller) until the motor actually turns. In open-loop mode, this compensation is missing.

## 3. Logic Flow in `MotorControl::update`

The software logic for BEMF-disabled operation is as follows:

1.  **Kickstart Trigger:** If `previousPwm == 0` and `targetPwm > 0`, `isKickstarting_priv` becomes `true`.
2.  **Kickstart Execution:** `writeMotorHardware(KICK_PWM, currDirection)` is called.
3.  **Termination:**
    - Since `bemfEnabled` is `false` (CV 49 bit 0 = 0), the BEMF threshold check is skipped.
    - Termination happens **exclusively** via the timeout: `now - kickstartBegin >= KICK_MAX_TIME`.
4.  **Steady State:** Once `isKickstarting_priv` is `false`, the logic enters the standard drive block:
    - `finalPwm = targetPwm;`
    - `lastAdjustment` and `bemfErrorSum` are reset to 0 because `bemfEnabled` is false.
    - `writeMotorHardware(finalPwm, currDirection)` is called.

## 4. Conclusion and Recommendations

The "only kickstarting" behavior is **not a software bug**, but a physical limitation of running high-frequency PWM at very low duty cycles without load compensation.

### Recommended Solutions:
1.  **Enable BEMF:** Set CV 49 to 1 (default). This allows the PI controller to overcome friction automatically.
2.  **Increase CV 2 (Start Voltage):** If BEMF must remain disabled, increase CV 2 to a value where the motor continues to spin after the kickstart (typically 40-80, resulting in 15-30% duty cycle).
3.  **Change Motor Type:** If the motor is inefficient, try setting CV 52 to 1 (Faulhaber), which uses a **400Hz** PWM frequency. The longer pulses at 400Hz provide much better low-speed torque in open-loop mode.
