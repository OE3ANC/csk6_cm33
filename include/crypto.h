/*
 * crypto_venus.h
 *
 *  Created on: 2020年9月25日
 *      Author: USER
 */

#ifndef MODULES_DRIVER_PRIVATE_INCLUDE_CRYPTO_H_
#define MODULES_DRIVER_PRIVATE_INCLUDE_CRYPTO_H_

#include "crypto_reg_venus.h"

#include "Driver_CRYPTO.h"

#include "venus_ap.h"

#define CRYPTO_PROTECT_KEY            (0xf5 << 24)

#define VENUS_CRYPTO0                 ((CRYPTO_RegDef *) CRYPTO_BASE)

// CRYPTO flags
#define CRYPTO_FLAG_INITIALIZED       (1U << 0)
#define CRYPTO_FLAG_POWERED           (1U << 1)

#define CRYPTO_MODE_SYS_KEY           (0U)
#define CRYPTO_MODE_USER_KEY          (1U)
#define CRYPTO_MODE_TEST_KEY          (2U)

#define CRYPTO_MODE_NONE              (0U)
#define CRYPTO_MODE_ECB               (1U)
#define CRYPTO_MODE_CBC               (2U)
#define CRYPTO_MODE_HASH              (3U)

#define CRYPTO_ENCRYPT                (1U)
#define CRYPTO_DECRYPT                (2U)

typedef struct _ECB_INFO {
    uint32_t *src;
    uint32_t *dst;
    uint32_t size;
    uint32_t mode;
} ECB_INFO;

typedef struct _CBC_INFO {
    uint32_t *src;
    uint32_t *dst;
    uint32_t size;
    uint32_t *ins;
    uint32_t mode;
} CBC_INFO;

typedef struct _HASH_INFO {
    uint32_t *src;
    uint32_t *dst;
    uint32_t size;
    uint32_t updata;
} HASH_INFO;

// CRYPTO Information (Run-Time)
typedef struct _CRYPTO_INFO {
    // Event callback
    CSK_CRYPTO_SignalEvent_t cb_event;
    void* workspace;

    ECB_INFO*          ecb_info;
    CBC_INFO*          cbc_info;
    HASH_INFO*         hash_info;

    uint32_t *         user_key;

    volatile uint8_t   key_mode;
    // driver state
    volatile uint8_t   mode;                 // default: none

    volatile uint8_t   hardware_busy;

    uint8_t            flags;         // CRYPTO driver flags
<<<<<<< HEAD
=======
    uint8_t            sha_inv;
>>>>>>> feature/zephyr
} CRYPTO_INFO;

// CRYPTO Resources definitions
typedef struct {
    CRYPTO_RegDef        *reg;           // Pointer to crypto peripheral
    uint32_t                irq_num;       // IRQ Number
    void (*irq_handler)(void);
    CRYPTO_INFO             *info;          // Run-Time Information
} const CRYPTO_RESOURCES;

#endif /* MODULES_DRIVER_PRIVATE_INCLUDE_CRYPTO_H_ */
