# Märklin 6021 CV Programming Protocol (Assumed)

**Disclaimer:** The official documentation for the Märklin 6021 CV programming protocol is not publicly available. The protocol described here is based on common practices for similar systems of the same era and is not guaranteed to be accurate.

## Entering Programming Mode

To enter programming mode, the following sequence of events must occur:

1.  The locomotive's address must be selected on the command station.
2.  The direction of the locomotive must be changed four times in rapid succession (within 2 seconds).

This will toggle the programming mode.

## Writing a CV

Once in programming mode, the decoder will interpret the next two speed commands as CV programming data:

1.  The first speed command sent by the command station will be interpreted as the CV address to be written.
2.  The second speed command sent by the command station will be interpreted as the value to be written to the selected CV.

After the second speed command is received, the decoder will automatically exit programming mode.

## CVs

### CV 15 - Programming Lock

To prevent accidental changes to CVs, CV 15 is used as a lock. Before programming can be initiated, CV 15 must be set to the value `7`.

### CV 8 - Decoder Reset

Writing the value `8` to CV 8 will reset all CVs to their factory default values.

## Limitations

Since the speed commands are used to transmit the CV address and value, the range of writable CVs and values is limited by the number of speed steps supported by the command station. For example, a command station with 14 speed steps can only write to CVs 1-14, and can only write values 1-14.
