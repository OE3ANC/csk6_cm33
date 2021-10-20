/*
 * IOMuxManager.c
 *
 *  Created on: 2020��8��25��
 *      Author: USER
 */

#include "hal/reg/cmn_iomux_reg_venus.h"
#include "hal/reg/aon_iomux_reg_venus.h"

#include "hal/IOMuxManager.h"

#include "hal/venus_ap.h"

static CMN_IOMUX_RegDef* iomux_reg = (CMN_IOMUX_RegDef*)CMN_IOMUX_BASE;
static AON_IOMUX_RegDef* aon_iomux_reg = (AON_IOMUX_RegDef*)AON_IOMUX_BASE;

int32_t IOMuxManager_PinConfigure (uint8_t pad, uint8_t pin_num, uint32_t pin_cfg){
    volatile uint32_t * volatile pin_base = NULL;

    switch (pad){
    case CSK_IOMUX_PAD_A:
    {
        if(pin_num > 31){
            return CSK_DRIVER_ERROR_PARAMETER;
        }
        pin_base = (volatile uint32_t* volatile)&(iomux_reg->REG_PAD_GPIOA_00.all);
        pin_base += pin_num;

        /*clear function select field and set value*/
        (*(volatile uint32_t* volatile)(pin_base)) &= (~0x1f);
        (*(volatile uint32_t* volatile)(pin_base)) |= (pin_cfg & 0x1f);
    }
        break;
    case CSK_IOMUX_PAD_B:
    {
        pin_base = (volatile uint32_t* volatile)&(iomux_reg->REG_PAD_GPIOB_00.all);
        pin_base += pin_num;

        if(pin_num > 15){
            return CSK_DRIVER_ERROR_PARAMETER;
        }

        // GPIOB 0-5 in aon region, using corresponding pad in GPIO mode, need to use function5
        // change the aon region pad in normal mode
        if(pin_num <= 5){
            volatile uint32_t* volatile aon_pin_base = (volatile uint32_t* volatile)&(aon_iomux_reg->REG_PAD_AON_GPIOB_00.all);
            aon_pin_base += pin_num;

            (*(volatile uint32_t* volatile)(aon_pin_base)) &= (~0x1f);
            (*(volatile uint32_t* volatile)(aon_pin_base)) |= 0x5;
        }

        /*clear function select field and set value*/
        (*(volatile uint32_t* volatile)(pin_base)) &= (~0x1f);
        (*(volatile uint32_t* volatile)(pin_base)) |= (pin_cfg & 0x1f);
    }
        break;
    default:
        return CSK_DRIVER_ERROR_PARAMETER;
    }

    // store in global value

    return CSK_DRIVER_OK;
}

int32_t IOMuxManager_ModeConfigure(uint8_t pad, uint8_t pin_num, uint8_t pin_mode){
    volatile uint32_t * volatile pin_base = NULL;

    switch (pad){
    case CSK_IOMUX_PAD_A:
    {
        if(pin_num > 31){
            return CSK_DRIVER_ERROR_PARAMETER;
        }
        pin_base = (volatile uint32_t* volatile)&(iomux_reg->REG_PAD_GPIOA_00.all);
        pin_base += pin_num;
    }
        break;
    case CSK_IOMUX_PAD_B:
    {
        if(pin_num > 15){
            return CSK_DRIVER_ERROR_PARAMETER;
        }
        pin_base = (volatile uint32_t* volatile)&(iomux_reg->REG_PAD_GPIOB_00.all);
        pin_base += pin_num;
    }
        break;

    default:
        return CSK_DRIVER_ERROR_PARAMETER;
    }

    switch (pin_mode){
    case HAL_IOMUX_NONE_MODE:
    {
        (*(volatile uint32_t* volatile)(pin_base)) &= (~0x40000);
    }
        break;
    case HAL_IOMUX_PULLUP_MODE:
    {
        // disable pull down
        (*(volatile uint32_t* volatile)(pin_base)) &= (~0x10000);
        (*(volatile uint32_t* volatile)(pin_base)) |= 0x40000;
        // enable pull up
        (*(volatile uint32_t* volatile)(pin_base)) |= 0x20000;
    }
        break;
    case HAL_IOMUX_PULLDOWN_MODE:
    {
        // disable pull up
        (*(volatile uint32_t* volatile)(pin_base)) &= (~0x20000);
        (*(volatile uint32_t* volatile)(pin_base)) |= 0x40000;
        // enable pull down
        (*(volatile uint32_t* volatile)(pin_base)) |= 0x10000;
    }
        break;
    }

    return CSK_DRIVER_OK;
}

/**
      Attention:
      
      the new api for zephyr in blow   
**/

int32_t csk_pinmux_set_func (uint8_t pin_num, uint32_t pin_cfg){
    volatile uint32_t * volatile pin_base = NULL;

    if(pin_num > CSK_PINMIX_NUM_MAX){
        return CSK_DRIVER_ERROR_PARAMETER;
    }

    pin_base = (volatile uint32_t* volatile)&(iomux_reg->REG_PAD_GPIOA_00.all);
    pin_base += pin_num;

    /*clear function select field and set value*/
    (*(volatile uint32_t* volatile)(pin_base)) &= (~0x1f);
    (*(volatile uint32_t* volatile)(pin_base)) |= (pin_cfg & 0x1f);


    // GPIOB 0-5 in aon region, using corresponding pad in GPIO mode, need to use function5
    // change the aon region pad in normal mode
    // the GPIOB 0 num is 31; GPIOB 5 num is 35
    if((pin_num >= 31)&&(pin_num <= 35)){
        volatile uint32_t* volatile aon_pin_base = (volatile uint32_t* volatile)&(aon_iomux_reg->REG_PAD_AON_GPIOB_00.all);  
        // the offset value is (pin_num - offset)   
        aon_pin_base += (pin_num-31);

        (*(volatile uint32_t* volatile)(aon_pin_base)) &= (~0x1f);
        (*(volatile uint32_t* volatile)(aon_pin_base)) |= 0x5;
    }

    // store in global value

    return CSK_DRIVER_OK;
}


int32_t csk_pinmux_get_func (uint8_t pin_num, uint32_t *pin_cfg){
    volatile uint32_t * volatile pin_base = NULL;
    if(pin_num > CSK_PINMIX_NUM_MAX){
        return CSK_DRIVER_ERROR_PARAMETER;
    }

    pin_base = (volatile uint32_t* volatile)&(iomux_reg->REG_PAD_GPIOA_00.all);
    pin_base += pin_num;
    *pin_cfg = (*(volatile uint32_t* volatile)(pin_base)) & 0x1f;

    return CSK_DRIVER_OK;
}

int32_t csk_pinmux_set_mode( uint8_t pin_num, uint8_t pin_mode){
    volatile uint32_t * volatile pin_base = NULL;

    if(pin_num > CSK_PINMIX_NUM_MAX){
        return CSK_DRIVER_ERROR_PARAMETER;
    }

    pin_base = (volatile uint32_t* volatile)&(iomux_reg->REG_PAD_GPIOA_00.all);

    switch (pin_mode){
    case CSK_PINMUX_PULLUP_ENABLE:
    {
        // disable pull down
        (*(volatile uint32_t* volatile)(pin_base)) &= (~0x10000);
        (*(volatile uint32_t* volatile)(pin_base)) |= 0x40000;
        // enable pull up
        (*(volatile uint32_t* volatile)(pin_base)) |= 0x20000;
    }
        break;
    case CSK_PINMUX_PULLUP_DISABLE:
    {
        // disable pull up
        (*(volatile uint32_t* volatile)(pin_base)) &= (~0x20000);
        (*(volatile uint32_t* volatile)(pin_base)) |= 0x40000;
    }
        break;
    }

    return CSK_DRIVER_OK;    
}
