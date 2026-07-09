# Proposal: CV 9 for PWM Frequency Control

Currently, the PWM frequency is tied to the motor profile selected in **CV 52**. While this simplifies setup for many users, it limits the ability to fine-tune the motor's performance, especially for older or less efficient motors that might benefit from a lower frequency (for better torque) or high-efficiency motors that benefit from high frequency (for silent operation).

## Proposed Configuration Variable

| CV | Name | Type | Default | Range | Description |
|---|---|---|:---:|:---:|---|
| 9 | PWM Frequency | Optional | 0 | 0–255 | Sets the PWM frequency for the motor driver. |

### Value Mapping

*   **0 (Default):** Use the default frequency defined by the motor profile in **CV 52**.
*   **1–255:** Set the PWM frequency explicitly.
    *   **Formula:** $Frequency = Value \times 100\text{ Hz}$
    *   **Example 1:** Value 4 = 400 Hz (Good for low-speed torque, typical for older motors).
    *   **Example 2:** Value 200 = 20,000 Hz (Silent operation, typical for modern motors).

## Motivation

1.  **Low-Speed Performance:** As analyzed in `PWM_AUDIT_2.md`, high-frequency PWM (20kHz) can lead to stalling at low speed steps because the pulses are too short to overcome inductive reactance and static friction. Being able to lower the frequency to e.g. 400Hz or 1kHz can significantly improve crawling performance without changing the motor profile.
2.  **Silent Operation:** Some users prefer 20kHz or higher to eliminate audible PWM "hum" or "whine."
3.  **Standardization:** Using CV 9 for PWM characteristics aligns with industry trends where CV 9 is often used for PWM period or frequency control.

## Implementation Impact

*   **MotorControl::setup():** The `analogWriteFreq` (RP2040) or `analogWriteFrequency` (ESP32) call will need to be updated to check CV 9.
*   **CvManager:** CV 9 must be added to the default CV list and the `printAllCvs` readout.
*   **Runtime Changes:** Since PWM frequency initialization often happens once, changing CV 9 should trigger a decoder reset (similar to CV 52) to take effect reliably on all platforms.
