# RENODE_GAPS.md

This document identifies known gaps and limitations in the current Renode simulation environment for xDuinoRails_MM.

## Hardware Support
- **PIO (Programmable I/O):** The RP2040 PIO block is not fully modeled in the standard Renode release. Complex timing-dependent protocols (like NeoPixel or high-speed DCC decoding) may require custom C#/Python extensions.
- **USB Device:** Serial over USB is not simulated. All debug output and CLI interaction are redirected to Hardware UART0 (Serial1).
- **Dual-Core:** While Renode supports multicore RP2040, the current simulation is focused on Core 0.

## Protocol & Signal
- **DCC/MM Injection:** There is no built-in "MM Signal Generator" in Renode. Bitstreams must be manually injected via GPIO state changes or Python scripts.
- **BEMF Feedback:** Motor BEMF is not automatically calculated based on PWM duty cycle. ADC values for BEMF must be predefined or updated via scripts during simulation.
- **RailCom:** High-speed RailCom feedback (cutouts and transmission) is not currently simulated.

## Testing
- **Robot Framework:** Timing in Robot Framework tests can be sensitive to the host machine's performance. `Wait For Line On Uart` timeouts should be generous.
- **Graphic Output:** Visual representation of LEDs and NeoPixels is limited to log messages or analyzer windows.
