# xDuinoRails_MM

A model train decoder firmware for the Seeed Xiao RP2040, using the Märklin Motorola protocol.

## Features

- Supports the Märklin Motorola protocol.
- Configuration via CVs (Configuration Variables).


## Hardware

- Seeed Xiao RP2040


## Getting Started

To build and upload the firmware, you will need to have [PlatformIO](https://platformio.org/) installed.

1. Clone this repository.
2. Open the repository in Visual Studio Code with the PlatformIO IDE extension.
3. To build the firmware, run: `pio run -e seeed_xiao_rp2040`
4. To upload the firmware, connect the Seeed Xiao RP2040 to your computer and run: `pio run -e seeed_xiao_rp2040 -t upload`


## Configuration

For detailed information on supported CVs and how to program them, please see the following documents:

- [CV Support and Configuration](./documentation/CV_SUPPORT.md)
- [Märklin CV Programming (Step-by-Step)](./documentation/MM_CV_PROGRAMMING.md)


## Development

This project includes a suite of native tests that can be run without needing the target hardware.

To run the tests, execute the following command:

```
pio test -e native
```


## License

License TBD.
