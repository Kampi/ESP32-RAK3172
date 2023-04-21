# CHANGELOG

## [4.1.1] - 21.04.2023

**Fixed:**

- Fix wrong error handling in 'RAK3172_LoRaWAN_isJoined'
- Remove 'RAK3172_PrepareSleep' and replace it with 'RAK3172_Deinit'
- Fix wrong baudrate handling when reinitializing the driver after sleep

**Changed:**
- Update CHANGELOG
- Change version number to 4.1.1

## [4.1.0] - 01.04.2023

**Fixed:**

- Fixed broken LoRaWAN attempt handling for RUI3 based firmware, because the newest RAK3172 firmwares will use the attempt count correctly
- `RAK3172_SetBaud` has not set the baudrate to the device structure
- Fixed bug where RUI3 based firmware needs other parameter values for the `AT+PBW` command
- Fix wrong enum value for `RAK3172_ERR_NOT_CONNECTED`
- Fix possible stack size issues with the UART receive task
- Fix wrong device driver object attribute in sleep example
- Fix downlink event to work with module firmware 1.0.4 and below
- Fix join event to work with module firmware 1.0.4 and below
- Fix missing check for a transmission confirmation in `RAK3172_LoRaWAN_Transmit`
- Fix wrong RX1 and RX2 delay times whn using module firmware 1.0.4 and below

**Added:**

- Add SF5 when using RUI3 and P2P mode
- Add additional option for the UART receive buffer size
- Add overflow and buffer error handling to `RAK3172_UART_EventTask`
- Add additional error handling in `RAK3172_UART_EventTask`
- Add `RAK3172_GetBaud` as inline function
- Add `Kconfig.projbuild` files for the examples
- Add additional RUI3 bandwith values
- Add support for long payloads (>500 bytes)
- Add LoRaWAN class enum
- Add `RAK3172_ERR_INVALID_MODE` error to all LoRaWAN and P2P functions
- Add support for esp-idf v5.0.0
- Add class B device support for LoRaWAN
- Add multicast support for LoRaWAN
- Add example for class C multicast
- Add error code for duty cycle violation in LoRaWAN mode
- Add multicast application for Chirpstack
- Add hardware dependend driver code
- Add support for the `AT+RX2DR` command
- Add support for the `AT+RX2FQ` command

**Added (EXPERIMENTAL):**

- Add tools to update the RAK3172 firmware by the driver when using the RUI3 option

**Changed:**

- Rename code rate enums for P2P mode
- Change minimal preamble for P2P mode to 5 when using RUI3
- Change `RAK3172_P2P_isListening` to inline
- Change type casting to explicit casting
- Change the LoRaWAN based join behaviour, because RUI3 based firmware is now using the attempt counter in LoRaWAN mode correctly
- Change LoRaWAN mode as disabled by default
- `RAK3172_TASK_BUFFER_SIZE` is affecting the task buffer size only now
- Rename `RAK3172_TASK_QUEUE_LENGTH` in `RAK3172_UART__QUEUE_LENGTH` and move it to the `UART` menu
- Rename `RAK3172_GetBaud` in `RAK3172_GetBaudFromDevice`
- Remove `RAK3172_UART_RX`, `RAK3172_UART_TX`, `RAK3172_UART_BAUD` and `RAK3172_UART_PORT` from the config, because they weren´t used in the driver
- Move the `Retries` parameter from `RAK3172_LoRaWAN_Init` to `RAK3172_LoRaWAN_Transmit`
- Clean up code
- Remove RF support from `Kconfig` (will come back later)
- Update examples
- Update documentation
- Update CHANGELOG
- Rework CHANGELOG layout
- Change version number to 4.1.0

## [4.0.0] - 09.09.2022

**Fixed:**

- Fix error in splash screen handling in function "RAK3172_SetMode"
- Rework event handling

**Added:**

- Add RAK3172 device structure initialization macro
- Add firmware version check
- Add factory reset
- Add sleep command for the RAK module
- Add "RAK3172_ERR_NOT_CONNECTED" in LoRaWAN mode
- Add error check for "RAK3172_LoRaWAN_SetRX1Delay" and "RAK3172_LoRaWAN_SetRX2Delay"
- Add function to set / get the LoRaWAN join mode (ABP or OTAA)
- Add functions to get / set the join (1/2) and Rx (1/2) delays
- Add function to get the RSSI value from all channels in LoRaWAN mode
- Add option to enable / disable a hardware reset
- Add support for core affinity
- Add support for RUI3 based firmware
- Add option for UART to support ESP32 power management modes
- Add option for non-blocking "RAK3172_LoRaWAN_StartJoin" function
- Add error output to "RAK3172_ERROR_CHECK"

**Changed:**

- Rename error codes
- Improve parameter checking in different functions
- Improve sleep handling of the driver
- Change license to MIT
- Remove LoRaWAN transmit timeout
- Code optimizations
- Update Kconfig with new configuration settings
- Update documentation
- Update examples
- Update CHANGELOG
- Change version number to 4.0.0

## [3.0.0] - 22.05.2022

**Added:**

- Add option to enable / disable LoRaWAN or P2P mode
- Add ADR support to LoRaWAN driver
- Add power down options in LoRaWAN mode

**Changed:**

- Fix missing path in CMakeLists.txt
- Replace Arduino code with esp-idf
- Code refactoring
- Update documentation
- Update CHANGELOG
- Change version number to 3.0.0

## [2.0.0] - 22.03.2022

**Added:**

- Add key handling for ABP mode
- Add functions to read the RSSI and SNR value of the last packet
- Add P2P mode

**Changed:**

- Remove "Status" from "RAK3172_Transmit"
- Code refactoring
- Update documentation
- Update CHANGELOG
- Change version number to 2.0.0

## [1.0.0] - 19.03.2022

**Added:**

Initial release
Update CHANGELOG
Change version number to 1.0.0
