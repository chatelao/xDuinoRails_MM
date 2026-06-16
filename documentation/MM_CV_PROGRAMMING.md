# Märklin CV Programming (Step-by-Step)

This guide provides a step-by-step process for programming CVs (Configuration Variables) on Märklin digital decoders.

| Phase | Step | [STOP] | [GO] | Keypad | Direction Change | Result |
|---|---|:---:|:---:|:---:|:---:|---|
| **Entering Programming Mode** | 1. Reset Central Station | Hold | Hold | | | "99" flashes on display |
| | 2. Turn Off Track Power | Press | | | | |
| | 3. Enter Address 80 | | | 80 | | |
| | 4. Initiate Programming Mode | | Press | | Hold | |
| | 5. Confirmation | | | | | Lights flashing slowly |
| **Programming CVs** | 1. Enter CV Number | | | CV No. | Press | Lights flash rapidly |
| | 2. Enter CV Value | | | Value | Press | Lights flash slowly |
| **Exiting Programming Mode** | Exit | Press | | | | Normal operation |

## Detailed Instructions

### Entering Programming Mode
1.  **Reset the Central Station:** Press and hold **[STOP]** and **[GO]** simultaneously until "99" flashes on the display.
2.  **Turn Off Track Power:** Press **[STOP]**.
3.  **Enter Address 80:** Use the keypad to enter the address **"80"**.
4.  **Initiate Programming Mode:** Press and hold the **Direction Change** button (the left control knob beyond the resistance point). While holding it, briefly press the **[GO]** button, then release the direction change button.
5.  **Confirmation:** The locomotive's lights should now be flashing, indicating that the decoder is ready for programming.

### Programming CVs
1.  **Enter the CV Number:** Enter the two-digit CV number (e.g., "01" for address). Briefly press the **Direction Change** button. Lights will flash more rapidly.
2.  **Enter the CV Value:** Enter the two-digit value (e.g., "20"). Briefly press the **Direction Change** button again. Lights return to slow flashing.
*Repeat these steps for additional CVs.*

### Exiting Programming Mode
*   To exit programming mode, press the **[STOP]** button.
