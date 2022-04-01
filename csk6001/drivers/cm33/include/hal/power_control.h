/*
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __POWER_CONTROL_H_
#define __POWER_CONTROL_H_


#include <stdio.h>


int32_t enter_lowpower_mode(uint32_t freq, uint8_t psram_ctrl);

int32_t exit_lowpower_mode(void);

#endif /* __POWER_CONTROL_H_ */
