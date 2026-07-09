*** Settings ***
Suite Setup                   Setup
Suite Teardown                Teardown
Test Teardown                 Test Teardown
Resource                      ${RENODEKEYWORDS}

*** Variables ***
${UART}                       sysbus.uart0
${FIRMWARE}                   ${CURDIR}/../../../.pio/build/seeed_xiao_rp2040/firmware.elf
${RESC}                       ${CURDIR}/../renode-config/tests/prepare.resc

*** Test Cases ***
Should Print Help Message
    [Documentation]           Verifies that the SerialConsole prints the help message when 'h' is entered.
    [Timeout]                 60 seconds
    Create Machine
    Start Emulation

    Wait For Line On Uart     xDuinoRails_MM starting...

    # Enter 'h' for help
    Write Char On Uart        h

    Wait For Line On Uart     Available Commands:
    Wait For Line On Uart     cv <num> <val> - Set CV
    Wait For Line On Uart     s <speed>     - Set speed (0-14)
    Wait For Line On Uart     d f|b         - Set direction
    Wait For Line On Uart     l <cat>       - Toggle log category

*** Keywords ***
Create Machine
    Execute Command           $global.TEST_FILE = @${FIRMWARE}
    Execute Script            ${RESC}
    Create Terminal Tester    ${UART}
