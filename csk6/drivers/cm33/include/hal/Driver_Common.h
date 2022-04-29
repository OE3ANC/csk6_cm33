/*
 * Project:      Common Driver definitions
 */

#ifndef __DRIVER_COMMON_H
#define __DRIVER_COMMON_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#define CSK_DRIVER_VERSION_MAJOR_MINOR(major,minor) (((major) << 8) | (minor))

/**
 \brief Driver Version
 */
typedef struct _CSK_DRIVER_VERSION
{
    uint16_t api;                         ///< API version
    uint16_t drv;                         ///< Driver version
} CSK_DRIVER_VERSION;

/* General return codes */
#define CSK_DRIVER_OK                 0 ///< Operation succeeded
#define CSK_DRIVER_ERROR             -1 ///< Unspecified error
#define CSK_DRIVER_ERROR_BUSY        -2 ///< Driver is busy
#define CSK_DRIVER_ERROR_TIMEOUT     -3 ///< Timeout occurred
#define CSK_DRIVER_ERROR_UNSUPPORTED -4 ///< Operation not supported
#define CSK_DRIVER_ERROR_PARAMETER   -5 ///< Parameter error
#define CSK_DRIVER_ERROR_SPECIFIC    -6 ///< Start of driver specific errors

/**
 \brief General power states
 */
typedef enum _CSK_POWER_STATE
{
    CSK_POWER_OFF,                        ///< Power off: no operation possible
    CSK_POWER_LOW,                        ///< Low Power mode: retain state, detect and signal wake-up events
    CSK_POWER_FULL                        ///< Power on: full operation at maximum performance
} CSK_POWER_STATE;


/** @addtogroup Exported_macro
  * @{
  */
#define SET_BIT(REG, BIT)     ((REG) |= (BIT))

#define CLEAR_BIT(REG, BIT)   ((REG) &= ~(BIT))

#define READ_BIT(REG, BIT)    ((REG) & (BIT))

#define CLEAR_REG(REG)        ((REG) = (0x0))

#define WRITE_REG(REG, VAL)   ((REG) = (VAL))

#define READ_REG(REG)         ((REG))

#define MODIFY_REG(REG, CLEARMASK, SETMASK)  WRITE_REG((REG), (((READ_REG(REG)) & (~(CLEARMASK))) | (SETMASK)))

#define POSITION_VAL(VAL)     (__CLZ(__RBIT(VAL)))

/**
  * @}
  */



#endif /* __DRIVER_COMMON_H */
