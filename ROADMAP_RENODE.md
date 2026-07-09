# Renode Simulation Roadmap

This roadmap outlines the phased integration of Renode simulation for the xDuinoRails_MM project to enhance CI/CD verification.

## Phase 1: Basic Integration (Current)
- [x] Establish Renode directory structure (`test/renode/`).
- [x] Integrate custom RP2040 peripheral models (ADC, PWM, MotorModel).
- [x] Implement SerialConsole verification via UART in Robot Framework.
- [x] Automated setup script `run_renode_tests.sh` for local and CI/CD use.
- [x] GitHub Actions integration (`ci.yml`).

## Phase 2: Telemetry and Control Loop
- [ ] Verify BEMF PI controller stability using Renode high-speed logs.
- [ ] Implement Robot Framework tests for speed curve mapping (Steps 1-14 to PWM).
- [ ] Simulate motor stalls and verify kickstart re-triggering.

## Phase 3: Protocol Stimulation
- [ ] Develop a Renode "Stimulus" model to inject MM2 packets into GPIO 7.
- [ ] Verify `ProtocolHandler` decoding accuracy under simulated clock jitter.
- [ ] Test CV programming (MM mode) via simulated track polarity reversals.

## Phase 4: Hardware Parity & Multi-Arch
- [ ] Port simulation models to support Seeed XIAO ESP32-C6.
- [ ] Add "Virtual Oscilloscope" output for PWM and BEMF signals.
- [ ] Integrate Power-Profiler to estimate DCC-ACK efficacy.
