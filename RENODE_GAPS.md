# Renode Simulation Gaps

This document tracks the current limitations and missing features in the Renode simulation environment for xDuinoRails_MM.

## Protocol Simulation
- **No Signal Injection:** Currently, there is no mechanism to feed real or simulated DCC/MM bitstreams into the `ProtocolHandler` via GPIO D5. Tests rely on manual CLI commands to simulate protocol effects.
- **Timing Sensitivity:** Renode's timing for bit-banged protocols may not be 100% cycle-accurate compared to real hardware.

## Peripheral Support
- **NeoPixel (WS2812B):** The Seeed XIAO's built-in NeoPixel is not currently simulated. Visual feedback from `DebugLeds` cannot be verified in Renode.
- **ESP32-C6 Support:** The simulation environment is restricted to the RP2040 platform. There is no plan to support the ESP32-C6 in Renode at this time.
- **EEPROM/Flash Persistence:** CV storage via EEPROM emulation is not yet verified in the simulation.

## Motor & BEMF
- **Physics Model:** The current motor model is a simplified DC motor. Real-world inductive spikes and complex BEMF characteristics are not fully modeled.
- **ADC Noise:** ADC readings in Renode are perfect; they lack the noise and jitter present in real-world BEMF sensing.

## CI/CD Environment
- **GUI Limitations:** Renode runs in headless mode in GitHub Actions; any feature requiring a GUI (like the Renode monitor window) must be automated via Robot Framework or CLI scripts.
- **Resource Constraints:** Complex simulations involving multiple cores and peripherals may be slow in the CI environment.
