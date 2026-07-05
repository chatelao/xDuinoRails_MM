# Audit: CV Implementation Progress

This document tracks the implementation status of Configuration Variables (CVs) in the xDuinoRails_MM firmware, with a focus on motor control parameters.

## Overview of CV Usage

| CV | Name | Status | Usage in Code |
|:---:|---|:---:|---|
| 1 | Base Address | ✅ Fully Implemented | Used by `ProtocolHandler` to filter packets. |
| 2 | Start Voltage | ✅ Implemented | Used in `MotorControl::setSpeed` to define minimum PWM. |
| 3 | Acceleration | ❌ Not Implemented | Initialized in `CvManager`, but ignored by `MotorControl`. |
| 4 | Braking Time | ❌ Not Implemented | Initialized in `CvManager`, but ignored by `MotorControl`. |
| 5 | Maximum Speed | ❌ Not Implemented | Initialized in `CvManager`, but ignored by `MotorControl`. |
| 7 | Version | ✅ Read-Only | Hardcoded to 10 (v1.0). |
| 8 | Manufacturer ID | ✅ Implemented | Writing 8 triggers a factory reset and reboot. |
| 15 | Programming Lock | ✅ Implemented | Must be set to 7 to enter programming mode. |
| 17/18| Long Address | ⚠️ Initialized Only | Values are stored but not used by the protocol handler. |
| 29 | Configuration | ⚠️ Initialized Only | Values are stored but not used for logic (e.g., speed steps). |
| 33 | Front Light (F0f) | ✅ Implemented | Bit 0 enables front light output. |
| 34 | Rear Light (F0r) | ✅ Implemented | Bit 1 enables rear light output. |
| 52 | Motor Type | ✅ Implemented | Selects motor characteristics profile in `MotorControl`. |
| 107/108| Ext. ID | ⚠️ Initialized Only | Metadata for DecoderDB identification. |

## Motor Parameters Detail (CV 52)

The `MotorControl` class uses `CV_52` to select between different hardware control profiles.

### Profiles defined in `MotorControl.cpp`:

| Parameter | Profile 0 (Standard DC) | Profile 1 (Faulhaber) | Profile 2 (Maxon) |
|---|---|---|---|
| **PWM Frequency** | 20 kHz | 400 Hz | 20 kHz |
| **Kickstart PWM** | 800 / 1023 | 1023 / 1023 | 600 / 1023 |
| **Max Kickstart Time**| 100 ms | 150 ms | 80 ms |
| **BEMF Threshold** | 100 | 120 | 80 |
| **BEMF Sample Int.** | 12 ms | 15 ms | 10 ms |

## Implementation Gaps & Observations

### 1. Acceleration & Braking (CV 3 & 4)
While these CVs exist in the manager, the `MotorControl::update` method currently applies the target speed almost instantaneously (modulated only by the kickstart logic). A momentum ramp is missing.

### 2. Maximum Speed (CV 5)
The `setSpeed` logic maps steps 1-14 to `pwmMinMoving` - `PWM_MAX`. It does not currently scale the upper bound based on CV 5.

### 3. Start Voltage Scaling (CV 2)
In `MotorControl.cpp`, the start voltage is calculated as:
`int pwmMinMoving = cvManager.getCv(CV_START_VOLTAGE) * 40;`
Since the total PWM range is 0-1023, a CV value higher than 25 will effectively set the minimum speed to the maximum possible PWM, leaving no room for speed steps. This scaling factor may need review.

### 4. BEMF Integration
BEMF sensing is currently only used to terminate the kickstart pulse early if rotation is detected. It is not used for active speed regulation (Load Compensation).
