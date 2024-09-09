# ESP32 driver for [RAK3172](https://store.rakwireless.com/products/wisduo-lpwan-module-rak3172) LoRa SoM

## Table of Contents

- [ESP32 driver for RAK3172 LoRa SoM](#esp32-driver-for-rak3172-lora-som)
  - [Table of Contents](#table-of-contents)
  - [About](#about)
  - [FUOTA](#fuota)
  - [Clock Synchronization](#clock-synchronization)
  - [Use with PlatformIO](#use-with-platformio)
  - [Use with esp-idf](#use-with-esp-idf)
  - [Additional resources](#additional-resources)
  - [Support me](#support-me)
  - [Maintainer](#maintainer)

## About

ESP32 driver for the UART based RAK3172 LoRaWAN / LoRa P2P SoM.

> Binary mode with RUI3 API isn´t supported yet!

## FUOTA

The FUOTA for the host CPU requires [Bootloader Plus](https://github.com/espressif/esp-bootloader-plus) or similar implementations.

## Clock Synchronization

Clock Synchronization is provided for devices with LoRaWAN < 1.1.

## Use with PlatformIO

- Add the library to the `lib_deps` parameter

```sh
lib_deps =
    https://github.com/Kampi/ESP32-RAK3172.git
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

## Additional resources

- [ST AN5554](https://www.st.com/resource/en/application_note/an5554-lorawan-firmware-update-over-the-air-with-stm32cubewl-stmicroelectronics.pdf)
- [LoRaWAN Fragmented Data Block Transport Specification](https://lora-alliance.org/resource_hub/lorawan-fragmented-data-block-transport-specification-v1-0-0/)
- [LoRaWAN FUOTA Process](https://lora-alliance.org/wp-content/uploads/2020/11/tr002-fuota_process_summary-v1.0.0.pdf)
- [LoRaWAN Application Layer Clock Synchronization](https://lora-alliance.org/wp-content/uploads/2020/11/application_layer_clock_synchronization_v1.0.0.pdf)
- [RAK3172 Firmware Download](https://docs.rakwireless.com/Product-Categories/WisDuo/RAK3172-Module/Datasheet/#firmware)
- [RAK3172-SiP Firmware Download](https://docs.rakwireless.com/Product-Categories/WisDuo/RAK3172-SiP/Datasheet/#firmware-os)

## Support me

If you are interested in the RAK3172 modules or in other RAKwireless products, you can use my [reference link](https://rakwireless.kckb.st/kampi) and the code `KAMPI` to get a 3% discount on your next order.

## Maintainer

- [Daniel Kampert](mailto:DanielKampert@kampis-elektroecke.de)
