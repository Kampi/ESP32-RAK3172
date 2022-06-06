# ESP32 driver for [RAK3172](https://store.rakwireless.com/products/wisduo-lpwan-module-rak3172) LoRa SoM.

## Table of Contents

- [About](#about)
- [Use with PlatformIO](#use-with-platformio)
- [Use with esp-idf](#use-with-esp-idf)
- [History](#history)

## About

ESP32 driver for the UART based RAK3172 LoRaWAN / LoRa P2P SoM.

> Binary mode with RUI3 API isn´t supported yet!

## Use with PlatformIO

- Add the library to the `lib_deps`parameter
```
lib_deps =
    https://gitlab.server-kampert.de/esp32/libraries/rak3172.git
```
- Copy the `Kconfig` file into the `src` directory
- Rename `Kconfig` to `Kconfig.projbuild`
- Run `pio -t menuconfig` from the root of your project to configure the driver

## Use with esp-idf

- Create a directory `components` in your project root
- Clone the repository into the `components` directory
- Run `menuconfig` from the root of your project to configure the driver

## Maintainer

- [Daniel Kampert](mailto:daniel.kameprt@kampis-elektroecke.de)