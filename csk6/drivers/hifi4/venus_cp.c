/*
 * venus_cp.c
 *
 *  Created on: Jul 9, 2020
 *
 */

#include "venus_cp.h"

// value returned by xthal_disable_interrupts() or xthal_enable_interrupts,
// used to restores interrupts in the call xthal_restore_interrupts().
// It is supposed to be the value of Program State Register (PS) on XEA2 ?
// DON'T MODIFY it MANUALLY!
uint32_t gint_flag = UINT_MAX; // 0

_FAST_FUNC_RO void enable_GINT()
{
    //if (GINT_enabled())
    //    return;

#ifdef XOS_USED
    if (gint_flag == UINT_MAX)
        xos_set_int_pri_level(0);
    else
        xos_restore_interrupts(gint_flag);
#else
    if (gint_flag == UINT_MAX) {
        xthal_enable_interrupts();
        XT_RSYNC();
    } else
        xthal_restore_interrupts(gint_flag);
#endif
}

//=====================================================================================================
#ifndef CSK6_MPW // combined interrupts are supported only in post-CSK6_MPW IC versions

// Subordinate interrupts are reported to the processor via IRQ_AP2CP_VECTOR (AP2CP combined Interrupt)
#define SUB_IRQ_COUNT   32 // max 32 AP IRQs?

// sub ISR table
_FAST_DATA_ZI ISR g_sub_isr_table[SUB_IRQ_COUNT] = { 0 };

// bit flags, 1 indicates that corresponding sub ISR has been registered!
_FAST_DATA_ZI uint32_t g_sub_isr_flag = 0;

// Clear sub interrupt
_FAST_FUNC_RO static void clear_sub_IRQ(uint32_t sub_irq_no)
{
    uint32_t sub_irq_bit = 1 << sub_irq_no;
    if (IP_SYSCTRL->REG_AP2CP_INTS.all & sub_irq_bit)
        IP_SYSCTRL->REG_AP2CP_INTS.all = sub_irq_bit;
}


_FAST_FUNC_RO static void ap2cp_isr(void)
{
    uint32_t i, sta;

    sta = IP_SYSCTRL->REG_AP2CP_INTS.all;
    for (i=0; i<32; i++) {
        if (sta & (1 << i)) {
            clear_sub_IRQ(i); // clear interrupt status
            if (i < SUB_IRQ_COUNT && g_sub_isr_table[i]) {
                (*g_sub_isr_table[i])(); // call ISR of sub IRQ
            }
        } // end if sta
    } // end for i
}


// Register sub ISR
void register_sub_ISR(uint32_t sub_irq_no, ISR isr, ISR* isr_old)
{
    // sub(AP) IRQ should be less than 32 and main IRQ should be IRQ_AP2CP_VECTOR
    uint32_t main_irq_no = IRQ_MAIN_VECTOR;
    if (sub_irq_no >= SUB_IRQ_COUNT)
        return;

    // register main IRQ if necessary
    if (g_sub_isr_flag == 0 && isr != NULL) {
    #ifdef XOS_USED
        xos_register_interrupt_handler(main_irq_no, (XosIntFunc *)ap2cp_isr, NULL);
    #else
        _xtos_set_interrupt_handler(main_irq_no, ap2cp_isr);
    #endif
    }

    // return original sub ISR
    if (isr_old != NULL)
        *isr_old = g_sub_isr_table[sub_irq_no];

    // store current sub ISR
    g_sub_isr_table[sub_irq_no] = isr;

    // set or clear flag, and unregister main IRQ if necessary
    if (isr != NULL) {
        g_sub_isr_flag |= (1 << sub_irq_no);
    } else {
        g_sub_isr_flag &= ~(1 << sub_irq_no);
        if (g_sub_isr_flag == 0) {
        #ifdef XOS_USED
            xos_register_interrupt_handler(main_irq_no, (XosIntFunc *)NULL, NULL);
        #else
            _xtos_set_interrupt_handler(main_irq_no, NULL);
        #endif
        }
    }
}


// Enable sub interrupt
void enable_sub_IRQ(uint32_t sub_irq_no)
{
    uint32_t main_irq_no = IRQ_MAIN_VECTOR;
    uint32_t irq_bit = 1 << sub_irq_no;
    if (!(IP_SYSCTRL->REG_AP2CP_INTE.all & irq_bit)) {
        IP_SYSCTRL->REG_AP2CP_INTE.all |= irq_bit;
    }
    if (!IRQ_enabled(main_irq_no)) {
        enable_IRQ(main_irq_no);
    }
}


// Disable sub interrupt
void disable_sub_IRQ(uint32_t sub_irq_no)
{
    uint32_t main_irq_no = IRQ_MAIN_VECTOR;
    uint32_t sub_irq_bit = 1 << sub_irq_no;
    uint32_t sub_irq_en = IP_SYSCTRL->REG_AP2CP_INTE.all;
    if (sub_irq_en & sub_irq_bit) {
        sub_irq_en &= ~sub_irq_bit;
        IP_SYSCTRL->REG_AP2CP_INTE.all = sub_irq_en;
        if (sub_irq_en == 0) { // no sub irq enabled
            disable_IRQ(main_irq_no);
        }
    }
}


// Is sub interrupt enabled?
uint8_t sub_IRQ_enabled(uint32_t sub_irq_no)
{
    uint32_t main_irq_no = IRQ_MAIN_VECTOR;
    if (!IRQ_enabled(main_irq_no))
        return 0;
    return (IP_SYSCTRL->REG_AP2CP_INTE.all & (1 << sub_irq_no)) ? 1 : 0;
}

#endif // CSK6_MPW
//=====================================================================================================

// evaluate Greatest Common Divisor
uint32_t GCD(uint32_t a, uint32_t b)
{
    //assert(a!=0 && b!=0);
    while (a != b) {
        if (a > b)
            a -= b;
        else
            b -= a;
    }
    return a;
}

// evaluate frequency division coefficient: N, M (N <= M, N=numerator, M=denominator),
// the divided frequency CAN be more than, less than or equal to khz_out_req.
// return value: N @ high 16bits, M @ low 16bits. 0 indicates failure.
// *khz_out_p = khz_out_divided
uint32_t eval_freq_divNM(uint16_t N_max, uint16_t M_max, uint32_t khz_in,
                        uint32_t khz_out_req, int32_t *khz_out_p)
{
    uint16_t N, M, N_sel, M_sel;
    int32_t diff, diff_min;

    // output frequency CANNOT be greater than input frequency
    if (khz_out_req > khz_in)
        return 0;
    if (khz_out_req == khz_in) {
        if (khz_out_p != NULL)
            *khz_out_p = khz_out_req;
        return ((1 << 16) | 1);
    }

    N_sel = M_sel = 1;
    diff_min = (khz_in - khz_out_req) * 10;
    for (M = 2; M <= M_max; M++) {
        for (N = 1; N < M; N++) {
            //both M & N are even, ignore it
            if (!(M & 0x1) && !(N & 0x1))
                continue;
            // M & N have Greatest Common Divisor which is greater than 2
            if (GCD(M, N) > 2)
                continue;
            //diff = khz_out_req * M / N - khz_in;
            diff = khz_out_req * M - khz_in * N;
            // use absolute value if the divided frequency is greater than requested output
            if (diff < 0)
                diff = 0 - diff;
            // magnify the difference ten-fold
            diff = diff * 10 / N;
            if (diff < diff_min) {
                diff_min = diff;
                N_sel = N;
                M_sel = M;
            }
        } // end N
    } // end M

    if (khz_out_p != NULL && M_sel > 0) {
        *khz_out_p = (khz_in * N_sel + (M_sel >> 1)) / M_sel;
    }
    return ( (N_sel << 16) | M_sel );
}


// evaluate frequency division coefficient: N, M (N <= M, N=numerator, M=denominator),
// the divided frequency CANNOT be more than khz_out_max!
// return value: N @ high 16bits, M @ low 16bits. 0 indicates failure.
// *khz_out_p = khz_out_divided
uint32_t eval_freq_divNM2(uint16_t N_max, uint16_t M_max, uint32_t khz_in,
                        uint32_t khz_out_max, int32_t *khz_out_p)
{
    uint16_t N, M, N_sel, M_sel;
    int32_t diff, diff_min = INT32_MAX;

    // output frequency CANNOT be greater than input frequency
    if (khz_out_max > khz_in)
        return 0;
    if (khz_out_max == khz_in) {
        if (khz_out_p != NULL)
            *khz_out_p = khz_out_max;
        return ((1 << 16) | 1);
    }

    N_sel = M_sel = 0;
    for (M = 2; M <= M_max; M++) {
        for (N = 1; N < M; N++) {
            //both M & N are even, ignore it
            if (!(M & 0x1) && !(N & 0x1))
                continue;
            // M & N have Greatest Common Divisor which is greater than 2
            if (GCD(M, N) > 2)
                continue;
            //diff = khz_out_max * M / N - khz_in;
            diff = khz_out_max * M - khz_in * N;
            // the divided frequency is greater than max. output
            if (diff < 0)   continue;
            diff = diff * 10 / N; // magnify ten-fold
            if (diff < diff_min) {
                diff_min = diff;
                N_sel = N;
                M_sel = M;
            }
        } // end N
    } // end M

    if (khz_out_p != NULL && M_sel > 0) {
        *khz_out_p = (khz_in * N_sel + (M_sel >> 1)) / M_sel;
    }
    return ( (N_sel << 16) | M_sel );
}
