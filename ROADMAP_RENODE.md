# Renode Simulation Roadmap

This document outlines the phases for integrating [Renode](https://renode.io/) simulation into the xDuinoRails_MM project to enable automated testing and verification of the decoder firmware.

## Phase 1: Basic Simulation & CI/CD Integration (Current)
- [x] Set up Renode project structure in `test/renode/`.
- [x] Define XIAO RP2040 board and peripheral mapping.
- [x] Implement automated test runner script `run_renode_tests.sh`.
- [x] Verify basic Serial Console (CLI) functionality (e.g., `help` command).
- [x] Integrate Renode simulation into GitHub Actions CI pipeline.

## Phase 2: Motor Dynamics & PWM Verification
- [ ] Implement/Integrate a DC Motor model in Renode.
- [ ] Connect Motor model to PWM outputs (D7/D8) and ADC inputs (A0/A1).
- [ ] Create Robot Framework tests to verify PWM scaling and directionality.
- [ ] Simulate basic Back-EMF (BEMF) feedback to test the PI controller.

## Phase 3: Protocol Signal Simulation
- [ ] Implement a mechanism to inject Märklin Motorola (MM) or DCC bitstreams into the simulated GPIO D5.
- [ ] Verify `ProtocolHandler` state transitions (address detection, speed step decoding).
- [ ] Test signal timeout behavior and emergency stop logic in simulation.

## Phase 4: Advanced Hardware Support
- [ ] Explore NeoPixel (WS2812B) protocol simulation for `DebugLeds` verification.
- [ ] Implement complex load profiles (e.g., incline, friction) for the motor model to stress-test the BEMF logic.
- [ ] Verify CV (Configuration Variable) persistence simulation.
