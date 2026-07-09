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
Should Show BEMF Telemetry
    [Documentation]           Verifies that BEMF telemetry is logged when the 'b' category is enabled.
    [Timeout]                 90 seconds
    Create Machine
    Start Emulation

    Wait For Line On Uart     xDuinoRails_MM starting...

    # Enable BEMF logging: 'l b'
    Write Line On Uart        l b

    # Set speed to trigger some activity
    Write Line On Uart        s 7

    # Wait for telemetry output (occurs every 1000ms)
    # Expected format: PWM: avg=..., last=... | BEMF: avg=..., last=...
    Wait For Line On Uart     PWM: avg=.*  treatAsRegex=true
    Wait For Line On Uart     BEMF: avg=.*  treatAsRegex=true

*** Keywords ***
Create Machine
    Execute Command           $global.TEST_FILE = @${FIRMWARE}
    # Override board initialization to use motor model
    Execute Command           $board_initialization_file = @${CURDIR}/../boards/initialize_seeed_xiao_rp2040_motor.resc
    Execute Script            ${RESC}
    Create Terminal Tester    ${UART}
