*** Settings ***
Suite Setup                   Setup
Suite Teardown                Teardown
Test Teardown                 Test Teardown
Resource                      ${RENODEKEYWORDS}

*** Variables ***
${UART}                       sysbus.uart0
${RESC}                       ${CURDIR}/decoder.resc

*** Test Cases ***
Should Show Help Menu
    [Documentation]           Verifies that the help command 'h' returns the command summary.
    [Timeout]                 30 seconds
    Create Machine
    Start Emulation

    Wait For Line On Uart     xDuinoRails_MM starting...

    Write Char On Uart        h
    Wait For Line On Uart     --- xDuinoRails CLI Help ---
    Wait For Line On Uart     cv <num> <val> : Set CV value
    Wait For Line On Uart     s <speed>      : Set speed (0-14)
    Wait For Line On Uart     d f/b          : Set direction

Should Set Speed and Change PWM
    [Documentation]           Verifies that setting speed via CLI updates the motor state.
    [Timeout]                 30 seconds
    Create Machine
    Start Emulation

    Wait For Line On Uart     xDuinoRails_MM starting...

    # Set speed to 7
    Write Line On Uart        s 7
    Wait For Line On Uart     Serial: Speed set to 7
    Wait For Line On Uart     Motor: Step 7 -> PWM

*** Keywords ***
Create Machine
    Execute Script            ${RESC}
    Create Terminal Tester    ${UART}
