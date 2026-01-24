# Refactoring Proposal for xDuinoRails_MM

## 1. Introduction

This document proposes a refactoring of the `xDuinoRails_MM.ino` sketch to improve its structure, maintainability, and extensibility. The current implementation, while functional, is contained within a single file, leading to tightly coupled code and mixed concerns.

The proposed refactoring will separate the code into three distinct modules, each with a clear responsibility:

1.  **Protocol Handler**: For low-level to high-level protocol handling.
2.  **Motor Control**: For all motor-related operations.
3.  **Lights Control**: For managing all lighting and visual feedback.

## 2. Current Architecture Issues

The current single-file `xDuinoRails_MM.ino` has several drawbacks:

*   **Lack of Modularity**: All logic for protocol decoding, motor physics, and visual feedback is intertwined in the main `loop()`.
*   **Global State**: The sketch relies heavily on global variables, making it difficult to track state changes and dependencies.
*   **Poor Readability**: The mix of different concerns in one place makes the code harder to understand and follow.
*   **Difficult to Test and Debug**: Isolating and testing specific functionalities is challenging.
*   **Hard to Extend**: Adding new features or changing existing ones can have unforeseen side effects on other parts of the code.

## 3. Proposed Architecture

We will refactor the codebase into a modular architecture using C++ classes. Each module will have its own header (`.h`) and source (`.cpp`) file.

The new file structure will be:

```
.
├── xDuinoRails_MM.ino      // Main sketch file (coordinator)
├── ProtocolHandler.h       // Interface for the Protocol Handler
├── ProtocolHandler.cpp     // Implementation of the Protocol Handler
├── MotorControl.h          // Interface for the Motor Control
├── MotorControl.cpp        // Implementation of the Motor Control
├── LightsControl.h         // Interface for the Lights Control
└── LightsControl.cpp       // Implementation of the Lights Control
```

### 3.1. Module Responsibilities

#### a) `ProtocolHandler`

This module will be responsible for decoding the Maerklin-Motorola (MM) digital signal and translating it into high-level commands.

*   **Responsibilities**:
    *   Initialize and manage the `MaerklinMotorola` library instance.
    *   Handle the DCC signal interrupt (`isr`).
    *   Parse the incoming data stream (`mm.Parse()`).
    *   Decode address, speed, direction, and function states.
    *   Manage protocol-specific logic (e.g., MM1/MM2 differences, command timeout).
*   **Public Interface (Conceptual)**:
    ```cpp
    class ProtocolHandler {
    public:
        void setup();
        void loop();
        bool isTimeout();
        int getTargetSpeed();
        MM2DirectionState getTargetDirection();
        bool getFunctionState(int f);
    };
    ```

#### b) `MotorControl`

This module will encapsulate all the logic related to controlling the physical motor.

*   **Responsibilities**:
    *   Configure motor-specific hardware (pins, PWM frequency).
    *   Translate speed steps into PWM values.
    *   Implement the "kickstart" logic for low-speed startup.
    *   Handle BEMF (Back-EMF) measurements to detect motor movement.
    *   Provide a safe interface to set motor speed and direction.
*   **Public Interface (Conceptual)**:
    ```cpp
    class MotorControl {
    public:
        void setup(int motorType);
        void update(int targetPwm, MM2DirectionState targetDir);
        void stop();
        bool isKickstarting();
    };
    ```

#### c) `LightsControl`

This module will manage all visual outputs of the decoder.

*   **Responsibilities**:
    *   Control the main function lights (front and rear).
    *   Manage the NeoPixel for status visualization (speed, kickstart, etc.).
    *   Control the onboard LEDs for debugging and status information.
*   **Public Interface (Conceptual)**:
    ```cpp
    class LightsControl {
    public:
        void setup();
        void update(int speedStep, MM2DirectionState direction, bool f0, bool f1, bool isMm2Locked, bool isKickstarting, bool isTimeout);
    };
    ```

## 4. Refactored Main Sketch Example

After refactoring, the main `xDuinoRails_MM.ino` file will act as a coordinator, connecting the modules.

### Before (`loop()` in `xDuinoRails_MM.ino`):
```cpp
void loop() {
  // A mix of protocol parsing, logic, and hardware control...
  mm.Parse();
  MaerklinMotorolaData* Data = mm.GetData();
  unsigned long now = millis();

  if (Data && !Data->IsMagnet && Data->Address == MM_ADDRESS) {
      // ... lots of code for direction, speed, lights ...
  }

  // ... kickstart logic ...
  if (isKickstarting) {
      // ...
  }

  // ... failsafe logic ...
}
```

### After (`loop()` in `xDuinoRails_MM.ino`):
```cpp
#include "ProtocolHandler.h"
#include "MotorControl.h"
#include "LightsControl.h"

ProtocolHandler protocol;
MotorControl motor;
LightsControl lights;

void setup() {
    protocol.setup();
    motor.setup(MOTOR_TYPE);
    lights.setup();
}

void loop() {
    protocol.loop();

    if (protocol.isTimeout()) {
        motor.stop();
    } else {
        int targetPwm = getLinSpeed(protocol.getTargetSpeed()); // Mapping remains for now
        motor.update(targetPwm, protocol.getTargetDirection());
    }

    lights.update(
        protocol.getTargetSpeed(),
        motor.getCurrentDirection(), // Or from protocol, depends on implementation
        protocol.getFunctionState(0),
        protocol.getFunctionState(1),
        protocol.isMm2Locked(),
        motor.isKickstarting(),
        protocol.isTimeout()
    );
}
```
This new structure will make the system much cleaner, easier to maintain, and ready for future enhancements.
