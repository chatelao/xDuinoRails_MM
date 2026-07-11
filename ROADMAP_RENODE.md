# Renode Simulation Roadmap

This document outlines the integration of Renode simulation into the xDuinoRails_MM project for automated verification.

## Current Integration (Phase 1)
- [x] Basic Seeed Studio XIAO RP2040 platform description (`.repl`).
- [x] Functional simulation script (`.resc`) for xDuinoRails_MM firmware.
- [x] Automated test execution with Robot Framework.
- [x] CI/CD integration via GitHub Actions.
- [x] Portable local execution script (`run_renode_tests.sh`).

## Future Improvements (Phase 2)
- [ ] **Protocol Simulation:** Implement a mechanism to inject real DCC or MM2 bitstreams into the simulated GPIO pin to test the `ProtocolHandler` in a closed-loop.
- [ ] **NeoPixel Simulation:** Add a Renode model for the internal NeoPixel to verify visual status feedback.
- [ ] **BEMF Loop Verification:** Fine-tune the `MotorModel` to more accurately reflect the electrical characteristics of model train motors for BEMF PI loop testing.
- [ ] **ESP32-C6 Support:** Expand Renode platform descriptions to include the ESP32-C6 target once stable Renode models are available.

## Usage
To run the simulation locally, use the provided script:
```bash
./run_renode_tests.sh
```
This will download a portable Renode instance, build the firmware, and execute the Robot Framework tests.
