#ifndef LORAWAN_DEFAULT_H_
#define LORAWAN_DEFAULT_H_

#include <stdint.h>

/** @brief  The APPEUI is in big-endian.
 */
static const uint8_t APPEUI[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

/** @brief  The DEVEUI is in big-endian.
 */
static const uint8_t DEVEUI[8] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

/** @brief  The APPKEY is in big-endian.
 */
static const uint8_t APPKEY[16] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

/** @brief Default JOIN attempts when using LoRaWAN mode.
 */
#define LORAWAN_JOIN_ATTEMPTS           5

/** @brief Maximum JOIN interval when using LoRaWAN mode.
 */
#define LORAWAN_MAX_JOIN_INTERVAL_S     15

/** @brief Default transmission attempts when using LoRaWAN mode.
 */
#define LORAWAN_TX_ATTEMPTS             5

#endif /* LORAWAN_DEFAULT_H_ */