# CV Support and Configuration

This document specifies the minimal supported Configuration Variables (CVs) for this project.

## CV Table

| Emoji | CV | Name | Type | Default/Fix | Description |
|---|---|---|---|---|---|
| 🏷️ | 1 | Base Address | Mandatory | 3 | The short address (1–127). |
| ⚡ | 2 | Start Voltage | Standard | 1 - 5 | Minimum PWM for the motor to start at speed step 1. |
| 📈 | 3 | Acceleration | Standard | 5 | "Chart Up": Time factor for smooth acceleration to maximum speed. |
| 📉 | 4 | Braking Time | Standard | 5 | "Chart Down": Time factor for coasting to a standstill. |
| 💨 | 5 | Maximum Speed | Standard | 0 or 255 | Limits the maximum voltage at full speed (scaling). 0 = deactivated (full throttle). |
| ℹ️ | 7 | Version | Mandatory | 10 | Read-Only. E.g. 10 for version 1.0. |
| 🏭 | 8 | Manufacturer ID | Mandatory | 13 | Important: NMRA ID for DIY/Public Domain. Writing to 8 triggers a reset. |
| 🔢 | 17 | Long Addr. (High) | Standard | 192 | Upper byte of the long address (default 192). |
| 🔢 | 18 | Long Addr. (Low) | Standard | 100 | Lower byte (Default 192+100 is often address 100 or 3). |
| ⚙️ | 29 | Configuration | Mandatory | 6 | Default 6 = 28/128 speed steps (2) + analog allowed (4). |
| 🔦 | 33 | Front Light (F0f) | Standard | 1 | Bit 0 on: Switches physical output A. |
| 🔴 | 34 | Rear Light (F0r) | Standard | 2 | Bit 1 on: Switches physical output B. |
| 🆔 | 107 | Ext. ID (High) | Meta | 1 | Identifier for DecoderDB (part 1 of ID 266). |
| 🆔 | 108 | Ext. ID (Low) | Meta | 10 | Identifier for DecoderDB (part 2 of ID 266). |
