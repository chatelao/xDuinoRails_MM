*** Settings ***
Suite Setup                   Setup
Suite Teardown                Teardown
Test Teardown                 Test Teardown
Resource                      ${RENODEKEYWORDS}

*** Variables ***
${UART}                       sysbus.uart0
${RESC}                       ${CURDIR}/xDuinoRails.resc

*** Test Cases ***
Should Boot And Show Welcome Message
    [Documentation]           Verifies that the firmware boots and prints the welcome message.
    [Timeout]                 30 seconds
    Create Machine
    Start Emulation

    Wait For Line On Uart     xDuinoRails_MM starting...
    Wait For Line On Uart     Motor: Type=0, PWM Freq=20000 Hz

Should Respond To CV Command
    [Documentation]           Verifies that the 'cv' command returns the list of CVs.
    [Timeout]                 10 seconds
    Create Machine
    Start Emulation

    Wait For Line On Uart     xDuinoRails_MM starting...

    Write Line To Uart        cv
    Wait For Line On Uart     CV 1: 3
    Wait For Line On Uart     CV 2: 1
    Wait For Line On Uart     CV 8: 131

Should Set Speed Via Serial
    [Documentation]           Verifies that setting speed via serial is acknowledged.
    [Timeout]                 10 seconds
    Create Machine
    Start Emulation

    Wait For Line On Uart     xDuinoRails_MM starting...

    Write Line To Uart        s 10
    Wait For Line On Uart     Serial: Speed set to 10
    Wait For Line On Uart     Motor: Step 10 -> PWM 766, Dir 1

*** Keywords ***
Create Machine
    Execute Command           include @${RESC}
    Create Terminal Tester    ${UART}
