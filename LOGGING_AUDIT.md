# Logging Audit: Current State of Logging

This document describes the current architecture and implementation of logging in the xDuinoRails_MM firmware.

## Architecture

The logging system is centralized in the `Logger` class, providing a unified way to output debug information over the Serial USB connection.

### Configuration
Logging is controlled by **CV 250 (CV_DEBUG_ENABLE)**.
- **Value 0:** Logging disabled.
- **Value > 0:** Logging enabled (default = 1).

The `Logger` class caches the state of CV 250 during its initialization in `begin()` to minimize EEPROM/flash reads and ensure performance. A change to CV 250 typically requires a reboot to take effect (or re-initialization of the logger).

### API
- `void begin(CvManager *cvManager, unsigned long baudRate = 115200)`: Initializes the logger and the Serial port if enabled.
- `void print(const char *message)`: Basic string output.
- `void println(const char *message)`: String output with a newline.
- `void printf(const char *format, ...)`: Formatted output.

### Constraints
- **Buffer Size:** The `printf` method uses a fixed-size internal buffer of **128 bytes**. Messages exceeding this length will be truncated or cause undefined behavior (though `vsnprintf` is used for safety).
- **Initialization:** The logger must be initialized with a valid `CvManager` instance to check the configuration CV.

## Current Logging Points

The following areas of the codebase currently implement logging:

| Component | Log Message / Information |
| :--- | :--- |
| **Main** (`xDuinoRails_MM.ino`) | "xDuinoRails_MM starting..." |
| **CV Manager** | "CV [num] set to [value]" (Logged whenever a CV is modified via `setCv`) |
| **CV Programmer** | Programming mode entry/exit status. |
| | Progress of direction change sequences (for entering programming mode). |
| | Received CV addresses and values during programming. |
| **Protocol Handler** | Decoded MM packet details: Address, Speed, Direction, F0 state, and MM2 status. |

## Implementation Details

The `Logger` class is designed to be safe for both the target hardware (RP2040, ESP32) and native tests. In native tests, it interacts with a `Serial` mock.

```cpp
// Example usage in CvManager.cpp
logger.printf("CV %d set to %d\n", cv, value);
```

## Observed Issues / Improvements
1. **Dynamic Enable:** Currently, `cachedEnabled` is only set at `begin()`. If CV 250 is changed during runtime, logging won't enable/disable until the next reboot.
2. **Buffer Safety:** The 128-byte buffer in `printf` is sufficient for current messages but should be monitored if more complex logging is added.
