# Renode Simulation Roadmap

This roadmap outlines the plan for integrating and enhancing Renode simulation for the xDuinoRails_MM project.

## Phase 1: Basic Integration (Current)
- [x] Setup basic Renode infrastructure (REPL, RESC).
- [x] Integrate UART CLI verification using Robot Framework.
- [x] Basic CI/CD integration in GitHub Actions.
- [ ] Automated firmware build and test execution script.

## Phase 2: Peripheral & Logic Verification
- [ ] **Motor Control:** Verify PWM output and direction logic in simulation.
- [ ] **ADC & BEMF:** Simulate BEMF voltage feedback and verify the PI control loop response.
- [ ] **CV Management:** Verify CV read/write persistence and effects in simulation.
- [ ] **Lights Control:** Verify function-driven light switching.

## Phase 3: Protocol Simulation
- [ ] **DCC/MM Signal Injection:** Implement a way to inject DCC/MM signals into the simulation to test the `ProtocolHandler`.
- [ ] **Stress Testing:** Simulate various signal conditions and noise to test decoder robustness.

## Phase 4: Hardware-in-the-Loop (HIL) & Advanced Physics
- [ ] **Realistic Motor Model:** Integrate more accurate motor physics (friction, inertia).
- [ ] **Power Management:** Simulate power interruptions and brown-out conditions.
- [ ] **Multi-Core Verification:** Ensure thread-safety and performance on RP2040's dual cores.
