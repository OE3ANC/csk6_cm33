#ifndef __CSK_IOMUX_MANAGER_H
#define __CSK_IOMUX_MANAGER_H

#include <stdint.h>

#include "Driver_Common.h"


#define CSK_IOMUX_PAD_A                 0
// PIN NUM: 0-31

#define CSK_IOMUX_PAD_B                 1
// PIN NUM: 0-15

// IO mux function selector
#define CSK_IOMUX_FUNC_DEFAULT                 (0U)
#define CSK_IOMUX_FUNC_ALTER1                  (1U)
#define CSK_IOMUX_FUNC_ALTER2                  (2U)
#define CSK_IOMUX_FUNC_ALTER3                  (3U)
#define CSK_IOMUX_FUNC_ALTER4                  (4U)
#define CSK_IOMUX_FUNC_ALTER5                  (5U)
#define CSK_IOMUX_FUNC_ALTER6                  (6U)
#define CSK_IOMUX_FUNC_ALTER7                  (7U)
#define CSK_IOMUX_FUNC_ALTER8                  (8U)
#define CSK_IOMUX_FUNC_ALTER9                  (9U)
#define CSK_IOMUX_FUNC_ALTER10                 (10U)
#define CSK_IOMUX_FUNC_ALTER11                 (11U)
#define CSK_IOMUX_FUNC_ALTER12                 (12U)
#define CSK_IOMUX_FUNC_ALTER13                 (13U)
#define CSK_IOMUX_FUNC_ALTER14                 (14U)
#define CSK_IOMUX_FUNC_ALTER15                 (15U)
#define CSK_IOMUX_FUNC_ALTER16                 (16U)
#define CSK_IOMUX_FUNC_ALTER17                 (17U)
#define CSK_IOMUX_FUNC_ALTER18                 (18U)
#define CSK_IOMUX_FUNC_ALTER19                 (19U)


#define HAL_IOMUX_NONE_MODE                    (0U)
#define HAL_IOMUX_PULLUP_MODE                  (1U)
#define HAL_IOMUX_PULLDOWN_MODE                (2U)

// Pin identifier
typedef struct _CSK_IOMUX_PIN_ID {
    uint8_t       pad;
    uint8_t       num;

    // bit mask: Function Selector | ...
    uint32_t      config_val;
} CSK_IOMUX_PIN_ID;

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
int32_t IOMuxManager_PinConfigure (uint8_t pad, uint8_t pin_num, uint32_t pin_cfg);

int32_t IOMuxManager_ModeConfigure(uint8_t pad, uint8_t pin_num, uint8_t pin_mode);


#endif /* __CSK_IOMUX_MANAGER_H */
