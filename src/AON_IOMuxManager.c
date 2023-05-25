/*
 * AON_IOMuxManager.c
 *
 *  Created on: 2020年12月14日
 *      Author: USER
 */

#include "aon_iomux_reg_venus.h"

#include "AON_IOMuxManager.h"

#include "venus_ap.h"

static AON_IOMUX_RegDef* aon_iomux_reg = (AON_IOMUX_RegDef*)AON_IOMUX_BASE;

int32_t AON_IOMuxManager_PinConfigure (uint8_t pad, uint8_t pin_num, uint32_t pin_cfg){
    volatile uint32_t* volatile aon_pin_base = (volatile uint32_t* volatile)&(aon_iomux_reg->REG_PAD_AON_GPIOB_00.all);

    switch (pad){
    case CSK_AON_IOMUX_PAD_B:
    {
        aon_pin_base += pin_num;

        if(pin_num > 6){
            return CSK_DRIVER_ERROR_PARAMETER;
        }

        // GPIOB 0-5 in aon region, using corresponding pad in GPIO mode, need to use function5
        // change the aon region pad in normal mode

        /*clear function select field and set value*/
        (*(volatile uint32_t* volatile)(aon_pin_base)) &= (~0x1f);
        (*(volatile uint32_t* volatile)(aon_pin_base)) |= (pin_cfg & 0x1f);
    }
        break;
    default:
        return CSK_DRIVER_ERROR_PARAMETER;
    }

    // store in global value

    return CSK_DRIVER_OK;
}

int32_t AON_IOMuxManager_ModeConfigure(uint8_t pad, uint8_t pin_num, uint8_t pin_mode){
    volatile uint32_t* volatile aon_pin_base = (volatile uint32_t* volatile)&(aon_iomux_reg->REG_PAD_AON_GPIOB_00.all);

    switch (pad){
    case CSK_AON_IOMUX_PAD_B:
    {
        if(pin_num > 6){
            return CSK_DRIVER_ERROR_PARAMETER;
        }
        aon_pin_base += pin_num;
    }
<<<<<<< HEAD

=======
    	break;
>>>>>>> feature/zephyr
    default:
        return CSK_DRIVER_ERROR_PARAMETER;
    }

    switch (pin_mode){
    case HAL_AON_IOMUX_NONE_MODE:
    {
        (*(volatile uint32_t* volatile)(aon_pin_base)) &= (~0x40000);
    }
        break;
    case HAL_AON_IOMUX_PULLUP_MODE:
    {
        // disable pull down
        (*(volatile uint32_t* volatile)(aon_pin_base)) &= (~0x10000);
        (*(volatile uint32_t* volatile)(aon_pin_base)) |= 0x40000;
        // enable pull up
        (*(volatile uint32_t* volatile)(aon_pin_base)) |= 0x20000;
    }
        break;
    case HAL_AON_IOMUX_PULLDOWN_MODE:
    {
        // disable pull up
        (*(volatile uint32_t* volatile)(aon_pin_base)) &= (~0x20000);
        (*(volatile uint32_t* volatile)(aon_pin_base)) |= 0x40000;
        // enable pull down
        (*(volatile uint32_t* volatile)(aon_pin_base)) |= 0x10000;
    }
        break;
    }

    return CSK_DRIVER_OK;
}

