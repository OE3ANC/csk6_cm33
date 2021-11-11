/*
 * venus_cp.c
 *
 *  Created on: Jul 9, 2020
 *
 */

#include "hal/venus_cp.h"

// value returned by xthal_disable_interrupts() or xthal_enable_interrupts,
// used to restores interrupts in the call xthal_restore_interrupts().
// It is supposed to be the value of Program State Register (PS) on XEA2 ?
// DON'T MODIFY it MANUALLY!
uint32_t gint_flag = UINT_MAX; // 0

void enable_GINT()
{
	//FIXME: is it OK on HiFi4?
/*
	if (GINT_enabled())
		return;

	if ((gint_flag & 0xFUL) != 0xFUL) {
#ifdef XOS_USED
		xos_restore_interrupts(gint_flag);
#else
		xthal_restore_interrupts(gint_flag);
		XT_RSYNC();
#endif
	} else {
#ifdef XOS_USED
		xos_restore_interrupts(gint_flag);
#else
		xthal_enable_interrupts();
//		XTOS_SET_INTLEVEL(0);
		XT_RSYNC();
#endif
	}
*/

#ifdef XOS_USED
    xos_restore_interrupts(gint_flag);
#else
    if (gint_flag == UINT_MAX) {
        xthal_enable_interrupts();
        XT_RSYNC();
    } else
        xthal_restore_interrupts(gint_flag);
#endif
}
