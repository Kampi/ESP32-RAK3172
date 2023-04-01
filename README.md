# ESP32 driver for [RAK3172](https://store.rakwireless.com/products/wisduo-lpwan-module-rak3172) LoRa SoM

## Table of Contents

- [ESP32 driver for RAK3172 LoRa SoM](#esp32-driver-for-rak3172-lora-som)
  - [Table of Contents](#table-of-contents)
  - [About](#about)
  - [FOTA](#fota)
  - [Use with PlatformIO](#use-with-platformio)
  - [Use with esp-idf](#use-with-esp-idf)
  - [Maintainer](#maintainer)

## About

ESP32 driver for the UART based RAK3172 LoRaWAN / LoRa P2P SoM.

> Binary mode with RUI3 API isn´t supported yet!

## FOTA

The FOTA for the host CPU requires [Bootloader Plus](https://github.com/espressif/esp-bootloader-plus)!

## Use with PlatformIO

- Add the library to the `lib_deps` parameter

```sh
lib_deps =
    https://gitlab.server-kampert.de/esp32/libraries/rak3172.git
```

- Copy the `Kconfig` file into the `src` directory
- Rename `Kconfig` to `Kconfig.projbuild`
- Run `pio -t menuconfig` from the root of your project to configure the driver and the examples
- Build the project

## Use with esp-idf

- Create a directory `components` in your project root
- Clone the repository into the `components` directory
- Run `menuconfig` from the root of your project to configure the driver and the examples
- Build the project

## Maintainer

- [Daniel Kampert](mailto:daniel.kameprt@kampis-elektroecke.de)