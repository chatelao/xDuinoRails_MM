# Renode Simulation Gaps

While the Renode simulation provides a powerful environment for testing the decoder's logic, there are several known gaps compared to real hardware.

## 1. MM/DCC Protocol Injection
- **Gap:** Currently, there is no realistic simulation of the high-frequency Märklin Motorola or DCC signal injection.
- **Impact:** Testing the `ProtocolHandler` relies on manual UART command injection instead of real track signals.
- **Status:** Investigating custom peripheral models for bitstream injection.

## 2. NeoPixel Protocol
- **Gap:** The standard Renode `LED` model does not support the WS2812B (NeoPixel) protocol.
- **Impact:** The status LED behavior is not fully verifiable in simulation.
- **Status:** Requires a custom model for the PIO-based NeoPixel driver.

## 3. Real-world Motor Physics
- **Gap:** The `MotorModel` uses a simplified physics approach (back-EMF proportional to velocity).
- **Impact:** BEMF PI loop tuning in simulation may not perfectly translate to specific physical locomotive motors.
- **Status:** Basic proportional feedback is functional; inductive transients and friction non-linearities are simplified.

## 4. Hardware Shutdown Pins
- **Gap:** While GPIOs are defined, the effect of the `MOTOR_SHUT_PIN` and `FUNC_SHUT_PIN` on the driver chips is not electrically modeled.
- **Impact:** The firmware could theoretically "work" in simulation even if the shutdown logic is inverted or broken.
- **Status:** Documented as a known limitation.

## 5. ESP32-C6 Simulation
- **Gap:** Renode support for the ESP32-C6 is currently less mature than for the RP2040.
- **Impact:** Simulation tests are primarily focused on the RP2040 target.
- **Status:** Roadmap item for future expansion.
