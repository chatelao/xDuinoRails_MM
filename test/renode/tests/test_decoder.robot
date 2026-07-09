*** Settings ***
Suite Setup                   Custom Setup
Suite Teardown                Teardown
Test Setup                    Custom Test Setup
Resource                      ${RENODEKEYWORDS}

*** Variables ***
${UART}                       sysbus.uart0
${ELF_FILE}                   ${ELF_FILE_VAR}
${RESC_FILE}                  ${CURDIR}/../xduino_decoder.resc

*** Keywords ***
Custom Setup
    Setup                     # Start Renode remote server

Custom Test Setup
    Reset Emulation
    Execute Command           $global.ELF_FILE = @${ELF_FILE}
    Execute Command           include @${RESC_FILE}
    Create Terminal Tester    ${UART}
    Execute Command           start

*** Test Cases ***
Should Initialize And Print Help
    Wait For Line On Uart     xDuinoRails_MM starting...
    Write Line To Uart        h
    Wait For Line On Uart     Available commands:

Should Set Speed Via Serial
    Wait For Line On Uart     xDuinoRails_MM starting...
    # Toggle PWM logging to see motor updates
    Write Line To Uart        l w
    Wait For Line On Uart     PWM logging enabled

    # Set speed to 7
    Write Line To Uart        s 7
    Wait For Line On Uart     Motor: Step 7 -> PWM

    # Verify kickstart
    Wait For Line On Uart     Motor: Kickstart started

Should Read CVs
    Wait For Line On Uart     xDuinoRails_MM starting...
    Write Line To Uart        cv 1
    Wait For Line On Uart     CV 1:
