*** Settings ***
Suite Setup                   Setup
Suite Teardown                Teardown
Test Setup                    Reset Emulation
Resource                      ${RENODEKEYWORDS}

*** Variables ***
${UART}                       sysbus.uart0
${ELF_FILE}                   ${CURDIR}/../../../.pio/build/seeed_xiao_rp2040/firmware.elf
${RESC_FILE}                  ${CURDIR}/../xduino_decoder.resc

*** Keywords ***
Setup
    Execute Command           $ELF_FILE = @${ELF_FILE}
    Execute Command           include @${RESC_FILE}
    Create Terminal Tester    ${UART}

Teardown
    # No specific teardown needed

*** Test Cases ***
Should Initialize And Print Help
    Wait For Line On Uart     xDuinoRails_MM starting...
    Write Line To Uart        h
    Wait For Line On Uart     Available commands:

Should Set Speed Via Serial
    # Toggle PWM logging to see motor updates
    Write Line To Uart        l w
    Wait For Line On Uart     PWM logging enabled

    # Set speed to 7
    Write Line To Uart        s 7
    Wait For Line On Uart     Motor: Step 7 -> PWM

    # Verify kickstart
    Wait For Line On Uart     Motor: Kickstart started

Should Read CVs
    Write Line To Uart        cv 1
    Wait For Line On Uart     CV 1:
