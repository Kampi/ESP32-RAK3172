#ifndef LORA_P2P_DEFAULT_H_
#define LORA_P2P_DEFAULT_H_

#include <stdint.h>

/** @brief Reset pin used by the LoRaWAN module.
 */
#define LORAWAN_RST                     15

/** @brief Tx pin used by the LoRaWAN module (Rx pin used by the ESP32).
 */
#ifndef LORAWAN_RX
    #define LORAWAN_RX                  22
#endif

/** @brief Rx pin used by the LoRaWAN module (Tx pin used by the ESP32).
 */
#ifndef LORAWAN_TX
    #define LORAWAN_TX                  23
#endif

#endif /* LORA_P2P_DEFAULT_H_ */