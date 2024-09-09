# Clock Synchronization example for RAK3172

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/license/mit/)
[![Build](https://github.com/Kampi/ESP32-RAK3172/actions/workflows/release.yaml/badge.svg)](https://github.com/Kampi/ESP32-RAK3172/actions/workflows/release.yaml)

## Table of Contents

- [Clock Synchronization example for RAK3172](#clock-synchronization-example-for-rak3172)
  - [Table of Contents](#table-of-contents)
  - [About](#about)
  - [ChirpStack configuration](#chirpstack-configuration)
  - [Device firmware](#device-firmware)
  - [Example output](#example-output)
  - [Maintainer](#maintainer)

## About

This example demonstrates how to use the Clock Synchronization v1.0.0 feature via LoRaWAN with ChirpStack.

## ChirpStack configuration

You need a device profile with `Clock sync version v1.0.0` enabled in the `Application layer` menu of the profile. Also make sure that `Class-C` is supported.

## Device firmware

Make sure to enable the `RAK3172_MODE_WITH_LORAWAN_CLOCK_SYNC` configuration in your `sdkconfig`. Also set the correct LoRaWAN keys in `LoRaWAN_Default.h`.

Enter the `Device address`, `Network session key` and `Applications session key` to the `RAK3172_MC_Group_t` object in the device firmware.

## Example output

```sh
I (1304) main_task: Calling app_main()
I (1304) main: Starting application...
I (1304) RAK3172: Use library version: 4.2.0
I (1304) RAK3172: Modes enabled:
I (1304) RAK3172:      [x] LoRaWAN
I (1314) RAK3172:      [x] P2P
I (1314) RAK3172: Reset:
I (1324) RAK3172:      [ ] Hardware reset
I (1324) RAK3172:      [x] Software reset
I (1334) RAK3172_UART: UART config:
I (1334) RAK3172_UART:      Interface: 1
I (1334) RAK3172_UART:      Buffer size: 512
I (1344) RAK3172_UART:      Stack size: 4096
I (1344) RAK3172_UART:      Queue length: 8
I (1354) RAK3172_UART:      Rx: 13
I (1354) RAK3172_UART:      Tx: 14
I (1364) RAK3172_UART:      Baudrate: 115200
I (1314) main_task: Returned from app_main()
I (1374) RAK3172: Perform software reset...
I (1614) RAK3172:      Successful!
I (2114) RAK3172: Perform factory reset...
I (2384) RAK3172:      Successful!
I (2984) main: Firmware: RUI_4.2.2_RAK3272-SiP
I (2984) main: Serial number: 
I (2994) main: Current mode: 1
I (2994) RAK3172_LoRaWAN: Initialize module in LoRaWAN mode...
I (3094) RAK3172_LoRaWAN: Using OTAA mode
I (3254) main: Not joined. Rejoin...
I (11124) RAK3172_UART:  Not joined...
I (11124) RAK3172_UART:   Attempts left: 1
I (34474) main: Request clock sync...
I (39564) main: Clock sync done...
I (39564) main: Time since 01/01/1970
I (39564) main:  Seconds: 36
I (39574) main:  Minutes: 17
I (39574) main:  Hours: 11
I (39574) main:  Day of week: 0
I (39584) main:  Day: 9
I (39584) main:  Month: 10
I (39584) main:  Year: 125
```

## Maintainer

- [Daniel Kampert](mailto:DanielKampert@kampis-elektroecke.de)