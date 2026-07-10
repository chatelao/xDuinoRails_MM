# Renode Simulation Gaps

This document tracks the current limitations and missing features in the Renode simulation for the xDuinoRails_MM decoder.

## Protocol Simulation
- **DCC/MM Signal Injection:** Renode currently lacks a built-in model for high-frequency DCC or Märklin Motorola signal generation on GPIO pins. The `ProtocolHandler` cannot be fully tested with real protocol streams in the current simulation.
- **RailCom:** No simulation of RailCom feedback signals.

## Peripherals
- **NeoPixel (WS2812B):** The timing-sensitive NeoPixel protocol is not simulated. `DebugLeds` functionality is not visible in simulation results.
- **EEPROM/Flash Persistence:** While Renode supports flash, the specific wear-leveling or persistence behavior of the Arduino `EEPROM` library on RP2040 might differ from real hardware.

## Physics & Hardware
- **Motor Physics:** The current motor model is a simplified approximation. Real-world behaviors like back-EMF variations due to mechanical load, friction, and inductive spikes are only partially captured.
- **Power Consumption:** No simulation of current draw or power management features.

## Platform Specifics
- **ESP32-C6 Support:** Current Renode setup focuses on RP2040. ESP32-C6 simulation is not yet established in this project.
- **Dual-Core:** While Renode supports dual Cortex-M0+, complex race conditions or core-to-core communication might not be perfectly reproduced.
