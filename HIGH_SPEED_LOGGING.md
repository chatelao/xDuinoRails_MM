# High-Speed Logging Analysis

This document provides an analysis of raw motor control telemetry captured using the high-speed logging mode (`l h`).

## Captured Data
```
CSV | 239180 | 285 | 1254 | -114 | 1427 | 307 | 592 | 32 | 24
CSV | 239193 | 285 | 2170 | -1030 | 397 | -1912 | -1627 | 32 | 24
CSV | 239206 | 285 | 1480 | -340 | 57 | -659 | -374 | 32 | 24
CSV | 239219 | 285 | 860 | 280 | 337 | 686 | 971 | 32 | 24
CSV | 239232 | 285 | 1130 | 10 | 347 | 150 | 435 | 32 | 24
CSV | 239245 | 285 | 1216 | -76 | 271 | -51 | 234 | 32 | 24
CSV | 239258 | 285 | 785 | 355 | 626 | 944 | 1229 | 32 | 24
CSV | 239271 | 285 | 1484 | -344 | 282 | -583 | -298 | 32 | 24
CSV | 239284 | 285 | 857 | 283 | 565 | 777 | 1062 | 32 | 24
CSV | 239297 | 285 | 596 | 544 | 1109 | 1503 | 1788 | 32 | 24
CSV | 239310 | 285 | 1084 | 56 | 1165 | 548 | 833 | 32 | 24
CSV | 239323 | 285 | 2307 | -1167 | -2 | -2334 | -2049 | 32 | 24
CSV | 239336 | 285 | 1142 | -2 | -4 | -5 | 280 | 32 | 24
CSV | 239349 | 285 | 797 | 343 | 339 | 813 | 1098 | 32 | 24
CSV | 239362 | 285 | 1067 | 73 | 412 | 300 | 585 | 32 | 24
CSV | 239375 | 285 | 604 | 536 | 948 | 1427 | 1712 | 32 | 24
CSV | 239388 | 285 | 683 | 457 | 1405 | 1440 | 1725 | 32 | 24
```

## Analysis Summary

### Key Metrics
*   **Target PWM:** 285
*   **Target BEMF:** 1140 (calculated as `targetPwm * 4`)
*   **BEMF Statistics:**
    *   Minimum: 596
    *   Maximum: 2307
    *   Average: 1148.00
*   **Adjustment Statistics:**
    *   Minimum: -2334
    *   Maximum: 1503
    *   Average: 197.12

### Gain Verification
Based on the relationship `adjustment = (error * K / 16) + (integral * I / 64)`, the gains for this run are confirmed as:
*   **CV 54 (K): 32**
*   **CV 55 (I): 24**

## Observations and Diagnosis

1.  **Severe Oscillations:** The BEMF value fluctuates violently between 596 and 2307 around the target of 1140. This is a classic symptom of an over-tuned control loop.
2.  **High Correction Values:** The adjustment values (e.g., -2334, +1503) far exceed the target PWM (285), indicating that the controller is reacting too aggressively to errors.
3.  **Instability:** The system fails to settle. Even when the error is small (e.g., error 10 at timestamp 239232), subsequent measurements show immediate divergence.

## Recommendations

The current PI gains are too high for the connected motor, causing jerky movement and audible "hunting" behavior.

1.  **Reduce Proportional Gain (CV 54):** Lower K from 32 to **16** to reduce the immediate reaction to BEMF spikes.
2.  **Reduce Integral Gain (CV 55):** Lower I from 24 to **12** to slow down the accumulation of correction over time.
3.  **Re-evaluate:** After adjusting CVs, capture a new high-speed log to ensure the BEMF readings stay within a tighter band around the target (e.g., ±200).
