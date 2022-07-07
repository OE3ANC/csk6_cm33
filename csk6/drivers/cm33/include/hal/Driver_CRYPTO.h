/*
 * Driver_CRYPTO.h
 *
 *  Created on: 2020年9月25日
 *      Author: USER
 */

#ifndef INCLUDE_DRIVER_DRIVER_CRYPTO_H_
#define INCLUDE_DRIVER_DRIVER_CRYPTO_H_


#include "Driver_Common.h"

#define CSK_CRYPTO_API_VERSION CSK_DRIVER_VERSION_MAJOR_MINOR(1,01)  /* API version */


/****** CRYPTO Control Codes *****/
/*----- CRYPTO Control Codes: AES Parameters: Key mode -----*/
#define CSK_CRYPTO_AES_KEY_MODE_Pos               0
#define CSK_CRYPTO_AES_KEY_MODE_Msk               (3UL << CSK_CRYPTO_AES_KEY_MODE_Pos)
#define CSK_CRYPTO_AES_KEY_MODE_ROOT              (1UL << CSK_CRYPTO_AES_KEY_MODE_Pos)
#define CSK_CRYPTO_AES_KEY_MODE_USER              (2UL << CSK_CRYPTO_AES_KEY_MODE_Pos)
#define CSK_CRYPTO_AES_KEY_MODE_TEST              (3UL << CSK_CRYPTO_AES_KEY_MODE_Pos)

/*----- CRYPTO Control Codes: AES Parameters: Set SES Init Vector -----*/
#define CSK_CRYPTO_SET_AES_IV_Pos                 5
#define CSK_CRYPTO_SET_AES_IV_Msk                 (1UL << CSK_CRYPTO_SET_AES_IV_Pos)
#define CSK_CRYPTO_SET_AES_IV                     (1UL << CSK_CRYPTO_SET_AES_IV_Pos)

#define CSK_CRYPTO_RESET_HASH_Pos                 7
#define CSK_CRYPTO_RESET_HASH_Msk                 (1UL << CSK_CRYPTO_RESET_HASH_Pos)
#define CSK_CRYPTO_RESET_HASH                     (1UL << CSK_CRYPTO_RESET_HASH_Pos)

/*----- CRYPTO Control Codes: AES Invert Parameter -----*/
#define CSK_CRYPTO_IV_OUT_WORD_Pos                8
#define CSK_CRYPTO_IV_OUT_WORD_Msk                (1UL << CSK_CRYPTO_IV_OUT_WORD_Pos)
#define CSK_CRYPTO_IV_OUT_WORD                    (1UL << CSK_CRYPTO_IV_OUT_WORD_Pos)

#define CSK_CRYPTO_IV_OUT_BYTE_Pos                9
#define CSK_CRYPTO_IV_OUT_BYTE_Msk                (1UL << CSK_CRYPTO_IV_OUT_BYTE_Pos)
#define CSK_CRYPTO_IV_OUT_BYTE                    (1UL << CSK_CRYPTO_IV_OUT_BYTE_Pos)

#define CSK_CRYPTO_IV_IN_WORD_Pos                 10
#define CSK_CRYPTO_IV_IN_WORD_Msk                 (1UL << CSK_CRYPTO_IV_IN_WORD_Pos)
#define CSK_CRYPTO_IV_IN_WORD                     (1UL << CSK_CRYPTO_IV_IN_WORD_Pos)

#define CSK_CRYPTO_IV_IN_BYTE_Pos                 11
#define CSK_CRYPTO_IV_IN_BYTE_Msk                 (1UL << CSK_CRYPTO_IV_IN_BYTE_Pos)
#define CSK_CRYPTO_IV_IN_BYTE                     (1UL << CSK_CRYPTO_IV_IN_BYTE_Pos)

#define CSK_CRYPTO_IV_KEY_WORD_Pos                12
#define CSK_CRYPTO_IV_KEY_WORD_Msk                (1UL << CSK_CRYPTO_IV_KEY_WORD_Pos)
#define CSK_CRYPTO_IV_KEY_WORD                    (1UL << CSK_CRYPTO_IV_KEY_WORD_Pos)

#define CSK_CRYPTO_IV_KEY_BYTE_Pos                13
#define CSK_CRYPTO_IV_KEY_BYTE_Msk                (1UL << CSK_CRYPTO_IV_KEY_BYTE_Pos)
#define CSK_CRYPTO_IV_KEY_BYTE                    (1UL << CSK_CRYPTO_IV_KEY_BYTE_Pos)

#define CSK_CRYPTO_IV_VECTOR_WORD_Pos             14
#define CSK_CRYPTO_IV_VECTOR_WORD_Msk             (1UL << CSK_CRYPTO_IV_VECTOR_WORD_Pos)
#define CSK_CRYPTO_IV_VECTOR_WORD                 (1UL << CSK_CRYPTO_IV_VECTOR_WORD_Pos)

#define CSK_CRYPTO_IV_VECTOR_BYTE_Pos             15
#define CSK_CRYPTO_IV_VECTOR_BYTE_Msk             (1UL << CSK_CRYPTO_IV_VECTOR_BYTE_Pos)
#define CSK_CRYPTO_IV_VECTOR_BYTE                 (1UL << CSK_CRYPTO_IV_VECTOR_BYTE_Pos)

#define CSK_CRYPTO_IV_HASH_Pos                    16
#define CSK_CRYPTO_IV_HASH_Msk                    (1UL << CSK_CRYPTO_IV_HASH_Pos)
#define CSK_CRYPTO_IV_HASH                        (1UL << CSK_CRYPTO_IV_HASH_Pos)

/****** CRYPTO Event *****/
#define CSK_CRYPTO_EVENT_DONE                     (1UL << 0)
// If this happens, a new start of crypto peripheral is needed.
#define CSK_CRYPTO_EVENT_HARDWARE_ERROR           (1UL << 1)

typedef void (*CSK_CRYPTO_SignalEvent_t) (uint32_t event, void* workspace);

CSK_DRIVER_VERSION
CRYPTO_GetVersion (void);

int32_t
CRYPTO_Initialize (void *res, CSK_CRYPTO_SignalEvent_t cb_event, void* workspace);

int32_t
CRYPTO_Uninitialize (void *res);

int32_t
CRYPTO_PowerControl (void* res, CSK_POWER_STATE state);

int32_t
CRYPTO_Control (void* res, uint32_t control,
                uint32_t arg0);

int32_t
CRYPTO_ECB_Encrypt (void* res, uint32_t * p_source,
                    uint32_t num_bytes, uint32_t * p_dest);

int32_t
CRYPTO_ECB_Decrypt (void* res, uint32_t * p_source,
                    uint32_t num_bytes, uint32_t * p_dest);

int32_t
CRYPTO_CBC_Encrypt (void* res, uint32_t * p_source,
                    uint32_t num_bytes, uint32_t * p_dest);

int32_t
CRYPTO_CBC_Decrypt (void* res, uint32_t * p_source,
                    uint32_t num_bytes, uint32_t * p_dest);

int32_t
CRYPTO_Hash (void* res, uint32_t * p_source,
            uint32_t num_bytes, uint32_t * p_dest, uint32_t updata);

void* CRYPTO0();

#endif /* INCLUDE_DRIVER_DRIVER_CRYPTO_H_ */
