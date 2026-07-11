# Renode Simulation Roadmap

This roadmap outlines the phases for integrating Renode simulation into the xDuinoRails_MM development cycle.

## Phase 1: Basic Integration (Complete)
*   [x] Basic RP2040 REPL/RESC setup.
*   [x] UART output verification.
*   [x] Serial Console interaction via Robot Framework.

## Phase 2: Peripheral Verification
*   [ ] PWM output verification: Ensure the decoder correctly sets PWM frequency and duty cycle.
*   [ ] ADC input verification: Mock BEMF feedback to test the ADC sampling logic.
*   [ ] Lights Control: Verify GPIO toggling for front and rear lights.

## Phase 3: Automated Testing & CI
*   [ ] Scripted simulation: `run_renode_tests.sh` for local execution.
*   [ ] GitHub Actions integration: Run simulation tests on every push.
*   [ ] Code coverage: Attempt to gather coverage from Renode execution.

## Phase 4: Protocol Simulation
*   [ ] Signal injection: Develop a custom peripheral or script to feed Märklin Motorola (MM) packets into the DCC/MM pin.
*   [ ] End-to-end verification: Sending a protocol packet and observing motor PWM changes.

## Phase 5: Closed-Loop Motor Simulation
*   [ ] Integrated Motor Model: Use a physics-based motor model in Renode.
*   [ ] BEMF Feedback Loop: Connect Motor Model output back to the ADC to test PI controller (CV 54/55) behavior under load.

## Phase 6: ESP32-C6 Simulation
*   [ ] Explore Seeed Studio XIAO ESP32-C6 machine model in Renode.
*   [ ] Parity testing between RP2040 and ESP32-C6 logic.
