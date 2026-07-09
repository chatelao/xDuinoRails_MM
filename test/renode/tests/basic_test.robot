*** Settings ***
Resource                      ${RENODEKEYWORDS}
Suite Setup                   Run Keywords    Create Machine    Start Emulation
Test Teardown                 Test Teardown

*** Variables ***
${UART}                       sysbus.uart0
${RESC}                       ${CURDIR}/../seeed_xiao_rp2040.resc

*** Test Cases ***
Should Boot and Respond to Help
    [Documentation]           Verifies that the firmware boots and the serial console is interactive.
    [Timeout]                 30 seconds
    Wait For Line On Uart     xDuinoRails_MM starting...

    # Send help command
    Write Char On Uart        ?
    Wait For Line On Uart     --- Serial Console Help ---
    Wait For Line On Uart     cv <num> <val> : Set CV value

Should Set and Read CV
    [Documentation]           Verifies that CVs can be set and read via serial console.
    [Timeout]                 30 seconds
    Wait For Line On Uart     xDuinoRails_MM starting...

    # Set CV 1 (Address) to 42
    Write Line On Uart        cv 1 42
    Wait For Line On Uart     CV 1 set to 42

    # Read all CVs
    Write Line On Uart        cv
    Wait For Line On Uart     CV 1: 42

*** Keywords ***
Create Machine
    Execute Command           include @${RESC}
    Create Terminal Tester    ${UART}

Test Teardown
    Execute Command           mach clear
