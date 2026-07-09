*** Settings ***
Suite Setup                   Setup
Suite Teardown                Teardown
Test Setup                    Reset Emulation
Resource                      ${RENODEKEYWORDS}

*** Variables ***
${UART}                       sysbus.uart0
${RESC}                       ${CURDIR}/../xiao_rp2040.resc

*** Keywords ***
Setup
    Execute Command           include @${RESC}
    Create Terminal Tester    ${UART}

Teardown
    Execute Command           clear

*** Test Cases ***
Should Print Help
    Start Emulation
    Wait For Line On Uart     xDuinoRails_MM starting...
    Write Line To Uart        h
    Wait For Line On Uart     Available commands:
    Wait For Line On Uart     cv <num> <val> - Set CV
    Wait For Line On Uart     cv - Read all CVs
    Wait For Line On Uart     s <speed> - Set speed (0-14)
    Wait For Line On Uart     d f/b - Set direction

Should Read CVs
    Start Emulation
    Wait For Line On Uart     xDuinoRails_MM starting...
    Write Line To Uart        cv
    Wait For Line On Uart     CV 1: 3
    Wait For Line On Uart     CV 8: 8
    Wait For Line On Uart     CV 250: 1
