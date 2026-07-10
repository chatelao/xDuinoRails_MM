# ROADMAP_RENODE.md

This document outlines the phased integration of Renode simulation for the xDuinoRails_MM project.

## Phase 1: Basic UART and CLI (Completed)
- [x] Initial Renode setup for RP2040.
- [x] Mapping XIAO RP2040 UART0 for Serial Console access.
- [x] Integration with Robot Framework for automated CLI testing.
- [x] CI/CD pipeline integration.

## Phase 2: Peripherals and Motor Feedback (Ongoing)
- [ ] Implement virtual Motor feedback (PWM to Speed/BEMF).
- [ ] Verify Light outputs (Front/Rear/Internal) via GPIO states.
- [ ] Basic ADC simulation for BEMF readings.
- [ ] Stub DCC signal input via GPIO buttons/scripts.

## Phase 3: Protocol Injection (Planned)
- [ ] Inject Märklin Motorola (MM) bitstreams into the simulation via GPIO.
- [ ] Verify `ProtocolHandler` timing and bit detection in a virtual environment.
- [ ] End-to-end testing: DCC Command -> Protocol Handler -> Motor Speed change.

## Phase 4: Full System Verification (Future)
- [ ] Multi-node simulation (Command Station + Decoders).
- [ ] RailCom feedback simulation.
- [ ] NeoPixel protocol simulation (if supported by Renode extensions).
