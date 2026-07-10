*** Settings ***
Suite Setup     Setup
Suite Teardown  Teardown
Resource        ${RENODEKEYWORDS}

*** Variables ***
${RESC}         ${CURDIR}/../xiao_rp2040.resc
${UART}         sysbus.uart0

*** Test Cases ***
Should Boot And Show Welcome Message
    Wait For Line On Uart     xDuinoRails_MM starting...    timeout=10

Should Respond To Help Command
    Write Line To Uart        ?
    Wait For Line On Uart     Available commands:           timeout=5

Should Read All CVs
    Write Line To Uart        cv
    Wait For Line On Uart     CV 1:                         timeout=5
    Wait For Line On Uart     CV 250:                       timeout=5

*** Keywords ***
Setup
    Start Emulation
    Execute Script          ${RESC}

Teardown
    Reset Emulation
