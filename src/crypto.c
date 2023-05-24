/*
 * crypto_venus.c
 *
 *  Created on: 2020年9月25日
 *      Author: USER
 */

/*
 * Project:      CRYPTO HAL Driver definitions
 */
#include "assert.h"

#include "crypto.h"
#include "hal/cache.h"

#include "cmn_sysctrl_reg_venus.h"


#define CSK_CRYPTO_DRV_VERSION CSK_DRIVER_VERSION_MAJOR_MINOR(1,1)

// Driver Version
static const CSK_DRIVER_VERSION crypto_driver_version = {
    CSK_CRYPTO_API_VERSION,
    CSK_CRYPTO_DRV_VERSION
};

// CRYPTO0

static void
CRYPTO0_IRQHandler (void);

static ECB_INFO crypto0_ecb = {0};

static CBC_INFO crypto0_cbc = {0};

static HASH_INFO crypto0_hash = {0};

static CRYPTO_INFO crypto0_info = {
        NULL,
        NULL,
        &crypto0_ecb,
        &crypto0_cbc,
        &crypto0_hash,
        NULL,
        CRYPTO_MODE_SYS_KEY,
        CRYPTO_MODE_NONE,
        0,
        0,
        0,
};

static const CRYPTO_RESOURCES crypto0_resources = {
    VENUS_CRYPTO0,
    IRQ_CRYPTO_VECTOR,
    CRYPTO0_IRQHandler,
    &crypto0_info,
};

void* CRYPTO0(){
    return (void*) &crypto0_resources;
}

#define CHECK_RESOURCES(res)  do{\
    if(res != &crypto0_resources){\
        return CSK_DRIVER_ERROR_PARAMETER;\
    }\
}while(0)


CSK_DRIVER_VERSION CRYPTO_GetVersion (void) {
    return crypto_driver_version;
}

int32_t CRYPTO_Initialize (void *res, CSK_CRYPTO_SignalEvent_t cb_event, void* workspace) {

    CHECK_RESOURCES(res);

    CRYPTO_RESOURCES* crypto = (CRYPTO_RESOURCES*)res;

    if (crypto->info->flags & CRYPTO_FLAG_INITIALIZED){
        return CSK_DRIVER_OK;
    }

    crypto->info->cb_event = cb_event;
    crypto->info->workspace = workspace;

    crypto->info->hardware_busy = 0;
    crypto->info->key_mode = CRYPTO_MODE_SYS_KEY;
    crypto->info->mode = CRYPTO_MODE_NONE;
    crypto->info->user_key = NULL;
    // Default using little endian
    crypto->info->sha_inv = 1;

    // clear information
    __builtin_memset(crypto->info->ecb_info, 0, sizeof(ECB_INFO));
    __builtin_memset(crypto->info->cbc_info, 0, sizeof(CBC_INFO));
    __builtin_memset(crypto->info->hash_info, 0, sizeof(HASH_INFO));

    crypto->info->flags = CRYPTO_FLAG_INITIALIZED;

    return CSK_DRIVER_OK;
}


int32_t CRYPTO_Uninitialize (void *res) {
    CHECK_RESOURCES(res);

    CRYPTO_RESOURCES* crypto = (CRYPTO_RESOURCES*)res;

    crypto->info->cb_event = NULL;
    crypto->info->workspace = NULL;

    crypto->info->hardware_busy = 0;
    crypto->info->key_mode = CRYPTO_MODE_SYS_KEY;
    crypto->info->mode = CRYPTO_MODE_NONE;
    crypto->info->user_key = NULL;
    // Default using little endian
    crypto->info->sha_inv = 1;

    // clear information
    __builtin_memset(crypto->info->ecb_info, 0, sizeof(ECB_INFO));
    __builtin_memset(crypto->info->cbc_info, 0, sizeof(CBC_INFO));
    __builtin_memset(crypto->info->hash_info, 0, sizeof(HASH_INFO));

    crypto->info->flags = 0;

    return CSK_DRIVER_OK;
}

int32_t CRYPTO_PowerControl (void* res, CSK_POWER_STATE  state) {

    CHECK_RESOURCES(res);

    CRYPTO_RESOURCES* crypto = (CRYPTO_RESOURCES*)res;

    switch (state) {
    case CSK_POWER_OFF:

        // stop crypto engine
        crypto->reg->REG_CPT_CTRL.all = (CRYPTO_PROTECT_KEY | 0x0);

        // Disable UART IRQ
        disable_IRQ(crypto->irq_num);

        // Uninstall IRQ Handler
        register_ISR(crypto->irq_num, NULL, NULL);

        crypto->info->flags &= ~CRYPTO_FLAG_POWERED;

        crypto->info->hardware_busy = 0;
        crypto->info->mode = CRYPTO_MODE_NONE;
        crypto->info->key_mode = CRYPTO_MODE_SYS_KEY;
        break;

    case CSK_POWER_LOW:
        return CSK_DRIVER_ERROR_UNSUPPORTED;

    case CSK_POWER_FULL:
        if ((crypto->info->flags & CRYPTO_FLAG_INITIALIZED) == 0U) { return CSK_DRIVER_ERROR; }
        if ((crypto->info->flags & CRYPTO_FLAG_POWERED)     != 0U) { return CSK_DRIVER_OK; }

        // stop crypto engine
        crypto->reg->REG_CPT_CTRL.all = (CRYPTO_PROTECT_KEY | 0x0);

        // crypto AHB configuration
        IP_SYSCTRL->REG_SW_RESET2.bit.CRYPTO_RESET = 0x1;

        // Change all input or output format to LITTLE-ENDIAN
        // Out
        crypto->reg->REG_CPT_CONFIG.bit.OUT_WORD_INV = 0x1;
        crypto->reg->REG_CPT_CONFIG.bit.OUT_BYTE_INV = 0x1;
        // In
        crypto->reg->REG_CPT_CONFIG.bit.RESV_2_7 = 0x1;
        crypto->reg->REG_CPT_CONFIG.bit.BYTE_INV = 0x1;
        crypto->reg->REG_CPT_CONFIG.bit.WORD_INV = 0x0;
        // Key
        crypto->reg->REG_CPT_CONFIG.bit.KEY_WORD_INV = 0x1;
        crypto->reg->REG_CPT_CONFIG.bit.KEY_BYTE_INV = 0x1;
        // IV
        crypto->reg->REG_CPT_CONFIG.bit.IV_WORD_INV = 0x1;
        crypto->reg->REG_CPT_CONFIG.bit.IV_BYTE_INV = 0x1;

        // initialization vector
        crypto->reg->REG_CPT_AES_IV0.all = 0x0;
        crypto->reg->REG_CPT_AES_IV1.all = 0x0;
        crypto->reg->REG_CPT_AES_IV2.all = 0x0;
        crypto->reg->REG_CPT_AES_IV3.all = 0x0;

        crypto->reg->REG_CPT_AES_KEY0.all = 0x0;
        crypto->reg->REG_CPT_AES_KEY1.all = 0x0;
        crypto->reg->REG_CPT_AES_KEY2.all = 0x0;
        crypto->reg->REG_CPT_AES_KEY3.all = 0x0;

        crypto->info->hardware_busy = 0;
        crypto->info->mode = CRYPTO_MODE_NONE;
        crypto->info->key_mode = CRYPTO_MODE_SYS_KEY;
        // importance
        crypto->info->hash_info->updata = 0x0;

        crypto->info->flags |= CRYPTO_FLAG_POWERED;

        register_ISR(crypto->irq_num, crypto->irq_handler, NULL);
        enable_IRQ(crypto->irq_num);

        break;

    }
    return CSK_DRIVER_OK;
}

int32_t CRYPTO_ECB_Encrypt (void* res, uint32_t * p_source,
                                uint32_t num_bytes, uint32_t * p_dest) {
    uint32_t temp = 0;

    CHECK_RESOURCES(res);

    CRYPTO_RESOURCES* crypto = (CRYPTO_RESOURCES*)res;

    assert(p_source);
    assert(((uint32_t)p_source % 4) == 0);
    assert((num_bytes % 16) == 0);
    assert(p_dest);
    assert(((uint32_t)p_dest % 4) == 0);

    if ((crypto->info->flags & CRYPTO_FLAG_POWERED) == 0){
        return CSK_DRIVER_ERROR;
    }

    if (crypto->info->hardware_busy == 1){
        return CSK_DRIVER_ERROR_BUSY;
    }

    crypto->info->ecb_info->src = (uint32_t*)p_source;
    crypto->info->ecb_info->size = num_bytes;
    crypto->info->ecb_info->dst = (uint32_t*)p_dest;
    crypto->info->ecb_info->mode = CRYPTO_ENCRYPT;

    crypto->info->hardware_busy = 1;
    crypto->info->mode = CRYPTO_MODE_ECB;

    // flush source
    dcache_flush_range((unsigned long)p_source, (unsigned long)(p_source + (num_bytes / 4)));
    dcache_flush_range((unsigned long)p_dest, (unsigned long)(p_dest + (num_bytes / 4)));

    crypto->reg->REG_CPT_SRC_ADDR.all = (uint32_t)p_source;
    crypto->reg->REG_CPT_SRC_SIZE.all = num_bytes;
    crypto->reg->REG_CPT_DST_ADDR.all = (uint32_t)p_dest;

    temp = crypto->reg->REG_CPT_MODE.all & 0xffffff00; /*keep up the key filed*/
    crypto->reg->REG_CPT_MODE.all = (CRYPTO_PROTECT_KEY | temp | 0x1); /*ecb mode & encrypt & user key | driver key*/

    crypto->reg->REG_CPT_CTRL.all = (CRYPTO_PROTECT_KEY | 0x1); /*start procedure*/

    crypto->reg->REG_CPT_INT_EN.all = 0x3; /*done & error interrupt*/

    return CSK_DRIVER_OK;
}

int32_t CRYPTO_ECB_Decrypt (void* res, uint32_t * p_source,
                                uint32_t num_bytes, uint32_t * p_dest) {
    uint32_t temp = 0;

    CHECK_RESOURCES(res);

    CRYPTO_RESOURCES* crypto = (CRYPTO_RESOURCES*)res;

    assert(p_source);
    assert(((uint32_t)p_source % 4) == 0);
    assert((num_bytes % 16) == 0);
    assert(p_dest);
    assert(((uint32_t)p_dest % 4) == 0);

    if ((crypto->info->flags & CRYPTO_FLAG_POWERED) == 0){
        return CSK_DRIVER_ERROR;
    }

    if (crypto->info->hardware_busy == 1){
        return CSK_DRIVER_ERROR_BUSY;
    }

    crypto->info->ecb_info->src = (uint32_t*)p_source;
    crypto->info->ecb_info->size = num_bytes;
    crypto->info->ecb_info->dst = (uint32_t*)p_dest;
    crypto->info->ecb_info->mode = CRYPTO_DECRYPT;

    crypto->info->hardware_busy = 1;
    crypto->info->mode = CRYPTO_MODE_ECB;

    // flush source
    dcache_flush_range((unsigned long)p_source, (unsigned long)(p_source + (num_bytes / 4)));
    dcache_flush_range((unsigned long)p_dest, (unsigned long)(p_dest + (num_bytes / 4)));

    crypto->reg->REG_CPT_SRC_ADDR.all = (uint32_t)p_source;
    crypto->reg->REG_CPT_SRC_SIZE.all = num_bytes;
    crypto->reg->REG_CPT_DST_ADDR.all = (uint32_t)p_dest;

    temp = crypto->reg->REG_CPT_MODE.all & 0xffffff00; /*keep up the key filed*/
    crypto->reg->REG_CPT_MODE.all = (CRYPTO_PROTECT_KEY | temp | 0x5); /*ecb mode & decrypt & user key | driver key*/

    crypto->reg->REG_CPT_CTRL.all = (CRYPTO_PROTECT_KEY | 0x1); /*start procedure*/

    crypto->reg->REG_CPT_INT_EN.all = 0x3; /*done & error interrupt*/

    return CSK_DRIVER_OK;
}


int32_t CRYPTO_CBC_Encrypt (void* res, uint32_t * p_source,
                                uint32_t num_bytes, uint32_t * p_dest) {
    uint32_t temp = 0;

    CHECK_RESOURCES(res);

    CRYPTO_RESOURCES* crypto = (CRYPTO_RESOURCES*)res;

    assert(p_source);
    assert(((uint32_t)p_source % 4) == 0);
    assert((num_bytes % 16) == 0);
    assert(p_dest);
    assert(((uint32_t)p_dest % 4) == 0);

    if ((crypto->info->flags & CRYPTO_FLAG_POWERED) == 0){
        return CSK_DRIVER_ERROR;
    }

    if (crypto->info->hardware_busy == 1){
        return CSK_DRIVER_ERROR_BUSY;
    }

    crypto->info->cbc_info->src = (uint32_t*)p_source;
    crypto->info->cbc_info->size = num_bytes;
    crypto->info->cbc_info->dst = (uint32_t*)p_dest;
    crypto->info->cbc_info->mode = CRYPTO_ENCRYPT;

    crypto->info->hardware_busy = 1;
    crypto->info->mode = CRYPTO_MODE_CBC;

    // flush source
    dcache_flush_range((unsigned long)p_source, (unsigned long)(p_source + (num_bytes / 4)));
    dcache_flush_range((unsigned long)p_dest, (unsigned long)(p_dest + (num_bytes / 4)));

    crypto->reg->REG_CPT_SRC_ADDR.all = (uint32_t)p_source;
    crypto->reg->REG_CPT_SRC_SIZE.all = num_bytes;
    crypto->reg->REG_CPT_DST_ADDR.all = (uint32_t)p_dest;

    temp = crypto->reg->REG_CPT_MODE.all & 0xffffff00; /*keep up the key filed*/
    crypto->reg->REG_CPT_MODE.all = (CRYPTO_PROTECT_KEY | temp | 0x11); /*cbc mode & encrypt & user key | driver key*/

    crypto->reg->REG_CPT_CTRL.all = (CRYPTO_PROTECT_KEY | 0x1); /*start procedure*/

    crypto->reg->REG_CPT_INT_EN.all = 0x3; /*done & error interrupt*/

    return CSK_DRIVER_OK;
}

int32_t CRYPTO_CBC_Decrypt (void* res, uint32_t * p_source,
                                uint32_t num_bytes, uint32_t * p_dest) {
    uint32_t temp = 0;

    CHECK_RESOURCES(res);

    CRYPTO_RESOURCES* crypto = (CRYPTO_RESOURCES*)res;

    assert(p_source);
    assert(((uint32_t)p_source % 4) == 0);
    assert((num_bytes % 16) == 0);
    assert(p_dest);
    assert(((uint32_t)p_dest % 4) == 0);

    if ((crypto->info->flags & CRYPTO_FLAG_POWERED) == 0){
        return CSK_DRIVER_ERROR;
    }

    if (crypto->info->hardware_busy == 1){
        return CSK_DRIVER_ERROR_BUSY;
    }

    crypto->info->cbc_info->src = (uint32_t*)p_source;
    crypto->info->cbc_info->size = num_bytes;
    crypto->info->cbc_info->dst = (uint32_t*)p_dest;
    crypto->info->cbc_info->mode = CRYPTO_DECRYPT;

    crypto->info->hardware_busy = 1;
    crypto->info->mode = CRYPTO_MODE_CBC;

    // flush source
    dcache_flush_range((unsigned long)p_source, (unsigned long)(p_source + (num_bytes / 4)));
    dcache_flush_range((unsigned long)p_dest, (unsigned long)(p_dest + (num_bytes / 4)));

    crypto->reg->REG_CPT_SRC_ADDR.all = (uint32_t)p_source;
    crypto->reg->REG_CPT_SRC_SIZE.all = num_bytes;
    crypto->reg->REG_CPT_DST_ADDR.all = (uint32_t)p_dest;

    temp = crypto->reg->REG_CPT_MODE.all & 0xffffff00; /*keep up the key filed*/
    crypto->reg->REG_CPT_MODE.all = (CRYPTO_PROTECT_KEY | temp | 0x15); /*cbc mode & decrypt & user key | driver key*/

    crypto->reg->REG_CPT_CTRL.all = (CRYPTO_PROTECT_KEY | 0x1); /*start procedure*/

    crypto->reg->REG_CPT_INT_EN.all = 0x3; /*done & error interrupt*/

    return CSK_DRIVER_OK;
}

int32_t CRYPTO_Hash (void* res, uint32_t * p_source,
                    uint32_t num_bytes, uint32_t * p_dest, uint32_t updata) {
    uint32_t temp = 0;

    CHECK_RESOURCES(res);

    CRYPTO_RESOURCES* crypto = (CRYPTO_RESOURCES*)res;

    assert(p_source);
    assert(((uint32_t)p_source % 4) == 0);
    assert((num_bytes % 64) == 0);
    assert(p_dest);

    if ((crypto->info->flags & CRYPTO_FLAG_POWERED) == 0){
        return CSK_DRIVER_ERROR;
    }

    if (crypto->info->hardware_busy == 1){
        return CSK_DRIVER_ERROR_BUSY;
    }

    crypto->info->hash_info->src = (uint32_t*)p_source;
    crypto->info->hash_info->size = num_bytes;
    crypto->info->hash_info->dst = (uint32_t*)p_dest;

    crypto->info->hardware_busy = 1;
    crypto->info->mode = CRYPTO_MODE_HASH;

    // flush source
    dcache_flush_range((unsigned long)p_source, (unsigned long)(p_source + (num_bytes / 4)));

    crypto->reg->REG_CPT_CONFIG.bit.SHA_INV = 0x0;
    crypto->reg->REG_CPT_SRC_ADDR.all = (uint32_t)p_source;
    crypto->reg->REG_CPT_SRC_SIZE.all = num_bytes;

    if (!updata){
        crypto->reg->REG_CPT_SHA_ST0.all = 0x6a09e667;
        crypto->reg->REG_CPT_SHA_ST1.all = 0xbb67ae85;
        crypto->reg->REG_CPT_SHA_ST2.all = 0x3c6ef372;
        crypto->reg->REG_CPT_SHA_ST3.all = 0xa54ff53a;
        crypto->reg->REG_CPT_SHA_ST4.all = 0x510e527f;
        crypto->reg->REG_CPT_SHA_ST5.all = 0x9b05688c;
        crypto->reg->REG_CPT_SHA_ST6.all = 0x1f83d9ab;
        crypto->reg->REG_CPT_SHA_ST7.all = 0x5be0cd19;
    }

    crypto->info->hash_info->updata = updata;

    // TODO protect key
    temp = crypto->reg->REG_CPT_MODE.all & 0xffffff00; /*keep up the key field*/
    crypto->reg->REG_CPT_MODE.all = (CRYPTO_PROTECT_KEY | temp | 0x2); /*cbc mode & decrypt & user key | driver key*/

    crypto->reg->REG_CPT_CTRL.all = (CRYPTO_PROTECT_KEY | 0x1); /*start procedure*/

    crypto->reg->REG_CPT_INT_EN.all = 0x3; /*done & error interrupt*/

    return CSK_DRIVER_OK;
}

int32_t CRYPTO_Control (void* res, uint32_t control,
                        uint32_t arg0) {
    uint32_t temp = 0;

    CHECK_RESOURCES(res);

    CRYPTO_RESOURCES* crypto = (CRYPTO_RESOURCES*)res;

    switch (control & CSK_CRYPTO_AES_KEY_MODE_Msk){
    case CSK_CRYPTO_AES_KEY_MODE_ROOT:
        temp = crypto->reg->REG_CPT_MODE.all & 0xff;
        crypto->reg->REG_CPT_MODE.all = (CRYPTO_PROTECT_KEY | temp | (0x3 << 8));
        crypto->info->key_mode = CRYPTO_MODE_SYS_KEY;
        return CSK_DRIVER_OK;
    case CSK_CRYPTO_AES_KEY_MODE_USER:
        if (arg0){
            temp = crypto->reg->REG_CPT_MODE.all & 0xff;
            // clear bit 8
            temp &= (~(0x1 << 8));
            crypto->reg->REG_CPT_MODE.all = (CRYPTO_PROTECT_KEY | temp);
            crypto->info->key_mode = CRYPTO_MODE_SYS_KEY;

            uint32_t* addr = (uint32_t*)arg0;

            // write user key into register field
            crypto->reg->REG_CPT_AES_KEY0.all = addr[0];
            crypto->reg->REG_CPT_AES_KEY1.all = addr[1];
            crypto->reg->REG_CPT_AES_KEY2.all = addr[2];
            crypto->reg->REG_CPT_AES_KEY3.all = addr[3];
        } else{
            return CSK_DRIVER_ERROR_PARAMETER;
        }
        return CSK_DRIVER_OK;
    case CSK_CRYPTO_AES_KEY_MODE_TEST:
        temp = crypto->reg->REG_CPT_MODE.all & 0xff;
        crypto->reg->REG_CPT_MODE.all = (CRYPTO_PROTECT_KEY | temp | (0x2 << 8));
        crypto->info->key_mode = CRYPTO_MODE_TEST_KEY;
        return CSK_DRIVER_OK;
    }

    switch (control & CSK_CRYPTO_SET_AES_IV_Msk){
    case CSK_CRYPTO_SET_AES_IV:
        assert(arg0);
        uint32_t* addr = (uint32_t*)arg0;

        crypto->info->cbc_info->ins = (uint32_t*)addr;

        // initialization vector
        crypto->reg->REG_CPT_AES_IV0.all = addr[0];
        crypto->reg->REG_CPT_AES_IV1.all = addr[1];
        crypto->reg->REG_CPT_AES_IV2.all = addr[2];
        crypto->reg->REG_CPT_AES_IV3.all = addr[3];
        return CSK_DRIVER_OK;
    }

    switch (control & CSK_CRYPTO_RESET_HASH_Msk){
    case CSK_CRYPTO_RESET_HASH:
        crypto->info->hash_info->updata = 0x0;

        crypto->reg->REG_CPT_SHA_ST0.all = 0x6a09e667;
        crypto->reg->REG_CPT_SHA_ST1.all = 0xbb67ae85;
        crypto->reg->REG_CPT_SHA_ST2.all = 0x3c6ef372;
        crypto->reg->REG_CPT_SHA_ST3.all = 0xa54ff53a;
        crypto->reg->REG_CPT_SHA_ST4.all = 0x510e527f;
        crypto->reg->REG_CPT_SHA_ST5.all = 0x9b05688c;
        crypto->reg->REG_CPT_SHA_ST6.all = 0x1f83d9ab;
        crypto->reg->REG_CPT_SHA_ST7.all = 0x5be0cd19;
        return CSK_DRIVER_OK;
    }

    // Invert output word format
    if (control & CSK_CRYPTO_IV_OUT_WORD_Msk){
        crypto->reg->REG_CPT_CONFIG.bit.OUT_WORD_INV = arg0;
    }

    // Invert output byte format
    if (control & CSK_CRYPTO_IV_OUT_BYTE_Msk){
        crypto->reg->REG_CPT_CONFIG.bit.OUT_BYTE_INV = arg0;
    }

    // Invert input word format
    if (control & CSK_CRYPTO_IV_IN_WORD_Msk){
        crypto->reg->REG_CPT_CONFIG.bit.RESV_2_7 = arg0;
    }

    // Invert input byte format
    if (control & CSK_CRYPTO_IV_IN_BYTE_Msk){
        crypto->reg->REG_CPT_CONFIG.bit.BYTE_INV = arg0;
    }

    // Invert key word format
    if (control & CSK_CRYPTO_IV_KEY_WORD_Msk){
        crypto->reg->REG_CPT_CONFIG.bit.KEY_WORD_INV = arg0;
    }

    // Invert key byte format
    if (control & CSK_CRYPTO_IV_KEY_BYTE_Msk){
        crypto->reg->REG_CPT_CONFIG.bit.KEY_BYTE_INV = arg0;
    }

    // Invert vector word format
    if (control & CSK_CRYPTO_IV_VECTOR_WORD_Msk){
        crypto->reg->REG_CPT_CONFIG.bit.IV_WORD_INV = arg0;
    }

    // Invert vector byte format
    if (control & CSK_CRYPTO_IV_VECTOR_BYTE_Msk){
        crypto->reg->REG_CPT_CONFIG.bit.IV_BYTE_INV = arg0;
    }

    if (control & CSK_CRYPTO_IV_HASH_Msk){
        crypto->info->sha_inv = arg0;
    }

    return CSK_DRIVER_OK;
}

static void CRYPTO_IRQHandler (CRYPTO_RESOURCES *crypto) {
    uint32_t status = 0;
    uint32_t event = 0;
    status = crypto->reg->REG_CPT_STATUS.all;

    // TODO
    // Hardware error, when set engine enable will trigger interrupt immediately
    // but hardware status still equal zero
    if(status == 0){
        return;
    }

    crypto->reg->REG_CPT_INT_DIS.all = status; // disable done and error interrupt

    // complete
    if ((status & 0x1) && (crypto->info->hardware_busy == 1)){
        event |= CSK_CRYPTO_EVENT_DONE;
        // copy the result to destination
        if (crypto->info->mode == CRYPTO_MODE_HASH){
            if (crypto->info->sha_inv){
                crypto->reg->REG_CPT_CONFIG.bit.SHA_INV = 0x1;
            }
            crypto->info->hash_info->dst[0] = crypto->reg->REG_CPT_SHA_ST0.all;
            crypto->info->hash_info->dst[1] = crypto->reg->REG_CPT_SHA_ST1.all;
            crypto->info->hash_info->dst[2] = crypto->reg->REG_CPT_SHA_ST2.all;
            crypto->info->hash_info->dst[3] = crypto->reg->REG_CPT_SHA_ST3.all;
            crypto->info->hash_info->dst[4] = crypto->reg->REG_CPT_SHA_ST4.all;
            crypto->info->hash_info->dst[5] = crypto->reg->REG_CPT_SHA_ST5.all;
            crypto->info->hash_info->dst[6] = crypto->reg->REG_CPT_SHA_ST6.all;
            crypto->info->hash_info->dst[7] = crypto->reg->REG_CPT_SHA_ST7.all;

        }
    }

    if (status & 0x2){
        event |= CSK_CRYPTO_EVENT_HARDWARE_ERROR;
    }

    crypto->info->hardware_busy = 0;

    if (crypto->info->cb_event){
        crypto->info->cb_event(event, crypto->info->workspace);
    }
}

static void
CRYPTO0_IRQHandler (void){
    CRYPTO_IRQHandler(&crypto0_resources);
}
