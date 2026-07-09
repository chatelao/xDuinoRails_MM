# Renode Simulation Gaps

This document identifies the current limitations and gaps in the Renode simulation for the xDuinoRails_MM decoder project.

## 1. Protocol Signal Injection
*   **Gap:** The simulation currently lacks a mechanism to inject real-time DCC or Märklin Motorola (MM) signals into the `DCC_MM_SIGNAL` (D5/GPIO 7) pin.
*   **Impact:** We cannot verify the `ProtocolHandler` and `MaerklinMotorola` library integration in a live simulation. Tests currently rely on the `SerialConsole` to command the decoder.
*   **Mitigation:** Use a custom Renode peripheral or a Python script to toggle the GPIO pin at the protocol's timing, or mock the protocol handler in the firmware for simulation builds.

## 2. NeoPixel (WS2812) Simulation
*   **Gap:** Standard Renode does not have a built-in model for the WS2812 protocol used by `DebugLeds`.
*   **Impact:** The `DebugLeds` module cannot be visually or logically verified in the simulation.
*   **Mitigation:** A custom peripheral could be written to decode the WS2812 stream, but for now, it's a minor gap.

## 3. ESP32-C6 Support
*   **Gap:** While Renode supports many RISC-V cores, a complete machine model for the Seeed Studio XIAO ESP32-C6 is not yet as robust or well-documented as the RP2040 model.
*   **Impact:** Simulation is currently focused on the RP2040 target.
*   **Mitigation:** Continue focus on RP2040 simulation as the primary verification platform.

## 4. BEMF and Motor Physics
*   **Gap:** The `MotorModel` in Renode is a simplified physics model. It may not perfectly capture the inductive effects and back-EMF transients of every real-world model locomotive motor.
*   **Impact:** PID tuning (CV 54/55) done in simulation may not directly translate to real hardware.
*   **Mitigation:** Use simulation for logic verification (e.g., "does the PWM increase when load is applied?") rather than precise tuning.

## 5. Hardware-Specific PWM Characteristics
*   **Gap:** Renode's PWM model verifies duty cycles but may not capture fine-grained hardware artifacts or the exact behavior of the RP2040 PWM slices (like phase alignment) as seen on silicon.
*   **Impact:** High-frequency PWM effects on the motor are simplified.

## 6. RP2040 Specific Hardware Blocks
Based on the reference implementation status:
*   **Bootrom:** ❌ Not implemented. We bypass this by setting `VectorTableOffset` and loading ELF directly.
*   **Resets:** ❌ Not implemented. Hardware reset signals are not simulated.
*   **Clocks:** ⚠️ Partial. Basic clock structures defined, but fixed frequencies are used instead of dynamic PLL configuration.
*   **USB:** ❌ Not implemented.
*   **SSI:** ❌ Not implemented.
