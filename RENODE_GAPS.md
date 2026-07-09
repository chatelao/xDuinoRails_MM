# Renode Simulation Gaps

This document tracks known limitations and missing features in the Renode simulation for the xDuinoRails_MM project.

## 1. Protocol Simulation
- **No Real DCC/MM Signal Injection:** The current simulation does not inject realistic Märklin Motorola (MM) or DCC bitstreams into the `DCC_MM_SIGNAL` (D5/GPIO 7) pin. Verification depends on the SerialConsole commands or manual state manipulation in Renode.
- **Protocol Handler Timing:** High-frequency timing sensitivity of the `ProtocolHandler` (e.g., bit duration measurements) is not fully validated in the current simulation environment.

## 2. Peripheral Models
- **NeoPixel Protocol:** The WS2812B/NeoPixel protocol (Bit-banging or PIO) is not simulated. `DebugLeds` transitions cannot be visually or programmatically verified beyond GPIO state.
- **MM/DCC ACK:** The current-draw based acknowledgement (`DCC_ACK_PIN`) is not linked to a power-consumption model in Renode.

## 3. Platform Support
- **ESP32-C6 Simulation:** While the project supports the Seeed XIAO ESP32-C6, the Renode simulation environment is currently focused exclusively on the RP2040 (XIAO RP2040).
- **ADC Noise:** The `MotorModel` provides a clean back-EMF (BEMF) signal. Real-world ADC noise and electrical transients are not modeled.

## 4. Hardware/Physics
- **Realistic Motor Load:** The `MotorModel` uses a simplified physics model. Inertia, friction, and inductive reactance are approximations and may not perfectly reflect specific locomotive hardware.
- **BEMF Sampling Blanking:** The 500µs blanking delay in `MotorControl::readBEMF()` is present in firmware but its efficacy against electrical transients isn't testable in Renode due to the lack of an analog transient model.
