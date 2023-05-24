/*
 * AON_IOMuxManager.h
 *
 *  Created on: 2020年12月14日
 *      Author: USER
 */

#ifndef INCLUDE_DRIVER_AON_IOMUXMANAGER_H_
#define INCLUDE_DRIVER_AON_IOMUXMANAGER_H_

#include <stdint.h>

#include "Driver_Common.h"

#define CSK_AON_IOMUX_PAD_B                 1
// PIN NUM: 0-5

// IO mux function selector
#define CSK_AON_IOMUX_FUNC_DEFAULT                 (0U)
#define CSK_AON_IOMUX_FUNC_ALTER1                  (1U)
#define CSK_AON_IOMUX_FUNC_ALTER2                  (2U)
#define CSK_AON_IOMUX_FUNC_ALTER3                  (3U)
#define CSK_AON_IOMUX_FUNC_ALTER4                  (4U)
#define CSK_AON_IOMUX_FUNC_NORMAL                  (5U)

#define HAL_AON_IOMUX_NONE_MODE                    (0U)
#define HAL_AON_IOMUX_PULLUP_MODE                  (1U)
#define HAL_AON_IOMUX_PULLDOWN_MODE                (2U)

// Pin identifier
typedef struct _CSK_AON_IOMUX_PIN_ID {
    uint8_t       pad;
    uint8_t       num;

    // bit mask: Function Selector | ...
    uint32_t      config_val;
} CSK_AON_IOMUX_PIN_ID;

/**
    Set pin function and electrical characteristics

    param[in]   pad         Port number
                            (CSK_IOMUX_PAD_A, CSK_IOMUX_PAD_B)
    param[in]   pin_num     Pin number
                            (Depend on relative pad)
    param[in]   pin_cfg     pin_cfg configuration bit mask
                            pin_cfg==0U means reset the pin.
    return value:
        CSK_DRIVER_OK       function succeeded
        CSK_DRIVER_ERROR    function failed
*/
int32_t AON_IOMuxManager_PinConfigure (uint8_t pad, uint8_t pin_num, uint32_t pin_cfg);

int32_t AON_IOMuxManager_ModeConfigure(uint8_t pad, uint8_t pin_num, uint8_t pin_mode);

#endif /* INCLUDE_DRIVER_AON_IOMUXMANAGER_H_ */
