# Renode Simulation Gaps

This document tracks the current gaps and limitations of the Renode simulation for xDuinoRails_MM.

## Infrastructure Gaps

* **DCC/MM Signal Injection**: There is currently no mechanism in the simulation to inject real or simulated Märklin Motorola (MM) or DCC pulse trains into the `DCC_MM_SIGNAL` (GPIO 7) pin. Verification relies on the `SerialConsole` to bypass the protocol handler.
* **NeoPixel Simulation**: While the pin for the NeoPixel is defined, Renode does not currently simulate the WS2812 protocol or provide a visual buffer for the RGB LED.
* **Motor Physics**: The current simulation uses simple LEDs to represent motor outputs. It does not simulate back-EMF (BEMF) generation, motor load, or inertia.
* **ADC Feedback**: Related to motor physics, the BEMF sensing pins (A0, A1) do not receive any realistic analog feedback during simulation.
* **EEPROM Persistence**: Renode's simulation of the RP2040 internal flash and its use for EEPROM emulation (via the Arduino core) might not fully persist across resets in the same way physical hardware does.

## Verification Gaps

* **Protocol Handler**: Since signals cannot be injected, the `ProtocolHandler` logic (MM1/MM2 decoding) is not exercised in the Renode simulation.
* **BEMF PI Controller**: The load compensation logic in `MotorControl` is not verified because of the lack of BEMF feedback.
* **Power Management**: Motor and Function shutdown pins are not verified.
