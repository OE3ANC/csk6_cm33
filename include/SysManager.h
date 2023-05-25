/*
 * SysManager.h
 *
 *  Created on: Feb 9, 2022
 *      Author: USER
 */

#ifndef INCLUDE_DRIVER_SYSMANAGER_H_
#define INCLUDE_DRIVER_SYSMANAGER_H_

#include "Driver_Common.h"
#include "venus_ap.h"

static inline uint32_t __HAL_SYS_CHIP_ID(){
    return IP_SYSCTRL->REG_AON_WAIT_XTAL.bit.CHIP_ID;
}

/////// AP configure
#define __HAL_SYS_AP_RESET_KEY           0xCAFE000A
#define __HAL_SYS_AP_DERESET_KEY

// AP reset configure
#define __HAL_SYS_AP_RESET() do { \
    IP_SYSCTRL->REG_SW_RESET_AP.all = __HAL_SYS_AP_RESET_KEY;\
}while(0)

/////// CP configure
#define __HAL_SYS_CP_RESET_KEY           0xCAFE000B
#define __HAL_SYS_CP_DERESET_KEY         0xCAFE100B

// Get CP reset status
static inline uint8_t __HAL_SYS_IS_CP_RESET(){
    return IP_SYSCTRL->REG_SW_RESET_CP.all;
}

// Reset CP
#define __HAL_SYS_CP_RESET() do { \
    IP_SYSCTRL->REG_SW_RESET_CP.all = __HAL_SYS_CP_RESET_KEY;\
}while(0)

// De-reset CP
#define __HAL_SYS_CP_RELEASE() do { \
    IP_SYSCTRL->REG_SW_RESET_CP.all = __HAL_SYS_CP_DERESET_KEY;\
}while(0)

#define __HAL_SYS_CP_RUN() do { \
    IP_SYSCTRL->REG_CP_CTRL1.bit.CP_HIFI4_RUNSTALL = 0x0; \
}while(0)

#define __HAL_SYS_CP_STALL() do { \
    IP_SYSCTRL->REG_CP_CTRL1.bit.CP_HIFI4_RUNSTALL = 0x1; \
}while(0)

static inline uint32_t __HAL_SYS_CP_GET_BOOT_ADDR(){
    return IP_SYSCTRL->REG_CP_CTRL2.bit.CP_BOOT_ADDR;
}

#define __HAL_SYS_CP_SET_BOOT_ADDR(addr) do { \
    IP_SYSCTRL->REG_CP_CTRL2.bit.CP_BOOT_ADDR = addr; \
    IP_SYSCTRL->REG_CP_CTRL1.bit.CP_STAT_VECTOR_SEL = 0x1; \
}while(0)

/////// NPU configure
#define __HAL_SYS_NPU_RESET_KEY          0xCAFE000C
#define __HAL_SYS_NPU_DERESET_KEY        __HAL_SYS_NPU_RESET_KEY

// Get NPU reset status
static inline uint8_t __HAL_SYS_NPU_IS_RESET(){
    return IP_SYSCTRL->REG_SW_RESET_NPU.all;
}

// Reset NPU
#define __HAL_SYS_NPU_RESET() do { \
    IP_SYSCTRL->REG_SW_RESET_NPU.all = __HAL_SYS_NPU_RESET_KEY;\
}while(0)

// De-reset NPU
#define __HAL_SYS_NPU_RELEASE() do { \
    IP_SYSCTRL->REG_SW_RESET_NPU.all = __HAL_SYS_NPU_DERESET_KEY;\
}while(0)

#endif /* INCLUDE_DRIVER_SYSMANAGER_H_ */
