# Architecture of xDuinoRails_MM

This document describes the software architecture of the xDuinoRails_MM firmware.

## System Overview

xDuinoRails_MM is a model train decoder firmware designed for the Seeed Xiao RP2040 and ESP32-C6. It implements the Märklin Motorola (MM) protocol for controlling motor speed, direction, and auxiliary functions (lights, etc.), and supports configuration through Configuration Variables (CVs).

The system is designed with a modular architecture that separates protocol decoding, motor control, lighting, and configuration management. This separation of concerns, combined with hardware abstraction layers, enables robust native testing on non-embedded platforms.

## Architecture Diagram

![Architecture Diagram](https://www.plantuml.com/plantuml/proxy?cache=no&src=https://raw.githubusercontent.com/chatelao/xDuinoRails_MM/main/documentation/architecture.puml)

*The diagram above is rendered dynamically from the [architecture.puml](documentation/architecture.puml) file.*

## Core Components

The firmware is composed of several key modules, each responsible for a specific aspect of the decoder's functionality:

### 1. ProtocolHandler
- **Role:** Orchestrates the decoding of the Märklin Motorola signal.
- **Functionality:**
  - Interfaces with the `MaerklinMotorola` library (or its mock in tests).
  - Maintains the current target state (speed, direction, function states).
  - Handles protocol-specific logic, such as MM2 lock-in (preferring MM2 packets once detected) and direction change debouncing.
  - Monitors for signal timeouts to trigger a safety stop.

### 2. MotorControl
- **Role:** Controls the physical motor via PWM.
- **Functionality:**
  - Translates protocol speed steps into PWM values.
  - Implements motor-specific logic like "kickstart" (a brief high-power pulse when starting) and scaling based on `CV_START_VOLTAGE` and `CV_MAXIMUM_SPEED`.
  - Manages direction changes, ensuring the motor is stopped before reversing.
  - Supports Back-EMF (BEMF) sensing (hardware dependent).

### 3. LightsControl
- **Role:** Manages the locomotive's lighting.
- **Functionality:**
  - Controls front and rear lights based on the current direction and the state of function F0.
  - Configuration of light behavior can be extended via CVs.

### 4. CvManager
- **Role:** Manages Configuration Variables (CVs).
- **Functionality:**
  - Provides a centralized interface for reading and writing CV values.
  - Handles persistent storage using EEPROM (emulated or physical).
  - Implements special logic for specific CVs, such as the reset mechanism (writing 8 to CV 8).

### 5. CvProgrammer
- **Role:** Implements the MM CV programming sequence.
- **Functionality:**
  - Monitors the `ProtocolHandler` for the specific sequence of direction changes and speed packets required to enter programming mode and set CV values.
  - Directs the `CvManager` to update values upon successful programming.

### 6. DebugLeds
- **Role:** Provides visual feedback for debugging and status monitoring.
- **Functionality:**
  - Controls internal LEDs (Red, Green, Blue) and NeoPixels (if available).
  - Indicates states such as speed, function activation, MM2 lock, kickstart activity, and protocol timeout.

## Data Flow

1.  **Signal Input:** The physical MM signal is received on a digital pin.
2.  **Decoding:** The `ProtocolHandler` (via `MaerklinMotorola`) decodes the pulses into data packets.
3.  **State Update:** The `ProtocolHandler` updates the target speed, direction, and function states.
4.  **Control Execution:**
    - The `MotorControl` module reads the target speed/direction and updates the motor PWM.
    - The `LightsControl` module reads the direction and F0 state to update the light pins.
5.  **Feedback:** The `DebugLeds` module reflects the current internal state of the decoder.
6.  **Configuration:** The `CvProgrammer` may intercept packets to modify CVs via `CvManager`, which then affects the behavior of `MotorControl` and `LightsControl` in subsequent cycles.

## Hardware Abstraction and Native Testing

To enable development and testing on a PC (native environment), the firmware uses several strategies:

- **Dependency Injection:** Major modules are passed references to their dependencies (e.g., `MotorControl` receives a `CvManager` reference).
- **Mocking:** Hardware-specific libraries (Arduino core, EEPROM, MaerklinMotorola) are replaced with mock implementations in the `test/mocks` directory during native builds.
- **Conditional Compilation:** Sections of code that directly access hardware registers or features not available in the native environment are wrapped in `#ifndef PIO_UNIT_TESTING` guards.

This architecture ensures that the core logic can be verified with unit tests (located in `test/test_native`) without requiring the target microcontroller.

## Supported Architectures

The firmware currently supports:
- **RP2040:** Specifically the Seeed Xiao RP2040.
- **ESP32:** Specifically the Seeed Xiao ESP32-C6 (using the `pioarduino` platform fork).

Platform-specific differences, such as reboot mechanisms and PWM configuration, are handled via preprocessor macros (`ARDUINO_ARCH_RP2040` vs. `ARDUINO_ARCH_ESP32`).
