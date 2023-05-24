/*
 * Copyright(C) 2022 listenai
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 *
 */

#ifndef _HAL_OS_H_
#define _HAL_OS_H_

#include <stdint.h>
#if defined(__GNUC__)
#include "cmsis_gcc.h"
#else
#error "Unsupported compiler"
#endif

#if defined(__ZEPHYR__)
#include "arch/cpu.h"

static inline uint32_t hal_irq_lock(void)
{
    return arch_irq_lock();
}

static inline void hal_irq_unlock(uint32_t k)
{
    return arch_irq_unlock(k);
}
#else

#error "Compile without OS. Comment this line or adapt a new OS."

#define HAL_IRQ_PRI_BASE (1 << 5)

static inline uint32_t hal_irq_lock(void)
{
    uint32_t k = __get_BASEPRI();

    __set_BASEPRI(HAL_IRQ_PRI_BASE);

    return k;
}

static inline void hal_irq_unlock(uint32_t k)
{
    __set_BASEPRI(k);
}

#endif

static inline void hal_global_irq_disable(void)
{
    __set_PRIMASK(1);
}

static inline void hal_global_irq_enable(void)
{
    __set_PRIMASK(0);
}

#endif
