# xDuinoRails_MM User Manual

Welcome to the xDuinoRails_MM firmware for the Seeed Xiao RP2040. This firmware allows you to use the RP2040 as a model train decoder supporting the Märklin Motorola (MM) protocol.

## Hardware Connections

The following pins are used for the decoder functions on the Seeed Xiao RP2040:

| Function | Pin | Description |
|---|---|---|
| **DCC/MM/SX Input**| D5 | Connect to the track signal via optocoupler. |
| **DCC-ACK** | D4 | Output for DCC acknowledgment. |
| **RailCom** | D6 | Output for RailCom signal. |
| **Motor A** | D7 | Motor output A. |
| **Motor B** | D8 | Motor output B. |
| **BEMF A** | A0 | Back-EMF sensing input A. |
| **BEMF B** | A1 | Back-EMF sensing input B. |
| **Motor Shut** | A2 (D2) | Shutdown signal for motor driver. |
| **Front Light** | D9 | Forward direction light output (F0f). |
| **Rear Light** | D10 | Backward direction light output (F0b). |
| **Functions Shut**| A3 (D3) | Shutdown signal for functions. |

## Status Indicators

The Seeed Xiao RP2040's built-in LEDs and NeoPixel provide visual feedback on the decoder's state:

### Internal LEDs (RP2040)
- **Red LED (D15):** Lights up when the decoder has locked onto an MM2 signal.
- **Green LED (D16):** Lights up when Function F1 is active.

### NeoPixel LED
- **Pulsing Blue:** Decoder is idle (Speed 0) and receiving signal.
- **Color Gradient (Forward):** Bright Green (slow) to White (fast).
- **Color Gradient (Backward):** Bright Orange (slow) to Black/Off (fast).
- **Flashing Red:** Timeout - No valid signal detected for more than 1 second.
- **Solid White:** Motor kickstart is active.

## Configuration and Programming

The decoder can be configured using Configuration Variables (CVs).

### CV Table

This table specifies the minimal supported Configuration Variables (CVs) for this project.

| Emoji | CV | Name | Type | Default/Fix | Description |
|---|---|---|---|:---:|---|
| 🏷️ | 1 | Base Address | Mandatory | 3 | The short address (1–127). |
| ⚡ | 2 | Start Voltage | Standard | 10 | Minimum PWM for the motor to start at speed step 1. |
| 📈 | 3 | Acceleration | Standard | 5 | Time factor for smooth acceleration (Not yet implemented). |
| 📉 | 4 | Braking Time | Standard | 5 | Time factor for coasting to a standstill (Not yet implemented). |
| 💨 | 5 | Maximum Speed | Standard | 0 | Limits the maximum voltage at full speed (0 = 100%). |
| ℹ️ | 6 | Medium Speed | Standard | 0 | Mid-point voltage for the speed curve (0 = auto). |
| ℹ️ | 7 | Version | Mandatory | 10 | Read-Only. E.g. 10 for version 1.0. |
| 🏭 | 8 | Manufacturer ID | Mandatory | 13 | Important: NMRA ID for DIY/Public Domain. Writing to 8 triggers a reset. |
| 🐕 | 11 | Watchdog Timeout | Standard | 5 | Timeout in 100ms steps (Default 5 = 500ms). Shuts down motor if no signal. |
| 🔒 | 15 | Programming Lock | Standard | 0 | Set to 7 to allow programming via direction changes. |
| 🔢 | 17 | Long Addr. (High) | Standard | 192 | Upper byte of the long address (default 192). |
| 🔢 | 18 | Long Addr. (Low) | Standard | 100 | Lower byte (Default 192+100 is often address 100 or 3). |
| ⚙️ | 29 | Configuration | Mandatory | 6 | Default 6 = 28/128 speed steps (2) + analog allowed (4). |
| 🔦 | 33 | Front Light (F0f) | Standard | 1 | Bit 0 on: Switches physical output A. |
| 🔴 | 34 | Rear Light (F0r) | Standard | 2 | Bit 1 on: Switches physical output B. |
| 🔄 | 49 | BEMF Config | Standard | 1 | Bit 0: Enable BEMF sensing and Load Compensation. |
| 🏎️ | 52 | Motor Type | Standard | 0 | Selects the motor characteristics curve. 0 = Standard DC, 1 = Faulhaber, 2 = Maxon. |
| 🎛️ | 54 | BEMF K | Standard | 32 | Proportional gain for the BEMF PI controller. |
| 🎛️ | 55 | BEMF I | Standard | 24 | Integral gain for the BEMF PI controller. |
| 🆔 | 107 | Ext. ID (High) | Meta | 1 | Identifier for DecoderDB (part 1 of ID 266). |
| 🆔 | 108 | Ext. ID (Low) | Meta | 10 | Identifier for DecoderDB (part 2 of ID 266). |
| 🪵 | 250 | Debug Enable | Standard | 1 | 0 = Disabled, 1 = Enabled (default). Outputs debug info to Serial USB. |

### Programming Guide

For step-by-step instructions on how to program these CVs using a Märklin Central Station or 6021, see [Märklin CV Programming](documentation/MM_CV_PROGRAMMING.md).

## Safety Instructions
- Ensure all connections are secure and insulated to prevent short circuits.
- Do not exceed the voltage and current ratings of the Seeed Xiao RP2040 or your motor driver circuit.

## Command Line Interface (CLI)

The decoder provides a simple command line interface via the USB Serial port (115200 baud). This allows for direct control and configuration without a digital station.

### Available Commands

| Command | Description | Example |
|---|---|---|
| `cv <num> <val>` | Sets Configuration Variable `<num>` to value `<val>`. | `cv 1 3` (Sets address to 3) |
| `cv` | Reads out all current Configuration Variable values. | `cv` |
| `s <speed>` | Sets the target speed (0–14). | `s 7` (Half speed) |
| `d f` | Sets the direction to **Forward**. | `d f` |
| `d b` | Sets the direction to **Backward**. | `d b` |
| `f <0/1>` | Sets Function 1 (F1) to Off (0) or On (1). | `f 1` (Turn on F1) |
| `L` or `l` | Toggles USB Serial logging (On/Off). | `L` |

> **Note:** Commands are executed upon pressing Enter.
