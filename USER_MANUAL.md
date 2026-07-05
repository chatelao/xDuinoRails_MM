# xDuinoRails_MM User Manual

Welcome to the xDuinoRails_MM firmware for the Seeed Xiao RP2040. This firmware allows you to use the RP2040 as a model train decoder supporting the Märklin Motorola (MM) protocol.

## Hardware Connections

The following pins are used for the decoder functions on the Seeed Xiao RP2040:

| Function | Pin | Description |
|---|---|---|
| **DCC/MM Input** | D2 | Connect to the track signal via a suitable optocoupler/schmitt-trigger circuit. |
| **Motor A** | D7 | Motor output A. |
| **Motor B** | D8 | Motor output B. |
| **BEMF A** | A0 | Back-EMF sensing input A. |
| **BEMF B** | A1 | Back-EMF sensing input B. |
| **Front Light** | D10 | Forward direction light output (F0f). |
| **Rear Light** | D9 | Backward direction light output (F0b). |

## Status Indicators

The Seeed Xiao RP2040's built-in LEDs and NeoPixel provide visual feedback on the decoder's state:

### Internal LEDs (RP2040)
- **Red LED:** Lights up when the decoder has locked onto an MM2 signal.
- **Blue LED:** Lights up when Function F1 is active.

### NeoPixel LED
- **Pulsing Blue:** Decoder is idle (Speed 0) and receiving signal.
- **Color Gradient (Green to Red):** Indicates the current speed. Green is slow, Red is fast.
- **Flashing Red:** Timeout - No valid signal detected for more than 1 second.
- **Solid White:** Motor kickstart is active.

## Configuration and Programming

The decoder can be configured using Configuration Variables (CVs).

- **CV Table:** For a list of supported CVs and their default values, see [CV Support and Configuration](documentation/CV_SUPPORT.md).
- **Programming Guide:** For step-by-step instructions on how to program these CVs using a Märklin Central Station or 6021, see [Märklin CV Programming](documentation/MM_CV_PROGRAMMING.md).

## Safety Instructions
- Ensure all connections are secure and insulated to prevent short circuits.
- Do not exceed the voltage and current ratings of the Seeed Xiao RP2040 or your motor driver circuit.
