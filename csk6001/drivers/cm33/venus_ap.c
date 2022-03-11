/*
 * venus_ap.c
 *
 *  Created on: Aug 13, 2020
 *
 */

#include "venus_ap.h"

// whether interrupt vectors table can be altered?
//#define VECTORS_ALTERABLE   1 // 0

//default ISR for all interrupts, see startup_ARMCM33.S
//extern void Default_Handler(void);
//TODO 暂时不做任何处理
void Default_Handler(void)
{
    (void) 0;
}


#if VECTORS_ALTERABLE // modify directly interrupt vectors table!!

//TODO: _Vectors may be moved to elsewhere, SHOULD relocate it...
extern uint32_t __VECTOR_ITCM;
//extern uint32_t __Vectors;
#define INTERRUPT0_OFFSET   16 // in WORD

// Register ISR into Interrupt Vector Table
void register_ISR(uint32_t irq_no, ISR isr, ISR* isr_old)
{
    uint32_t *pIVT = &__VECTOR_ITCM;
    pIVT += INTERRUPT0_OFFSET;

    if (irq_no > IRQ_MAX)   return;
    if (isr_old) {
        ISR old = (ISR)(pIVT[irq_no]);
        *isr_old = (old == Default_Handler ? NULL : old);
    }
    pIVT[irq_no] = (uint32_t)isr;
    // default interrupt priority is 0, we assume that
    // 0 indicates the interrupt has not been initialized before...
    if (NVIC_GetPriority(irq_no) == 0)
        NVIC_SetPriority(irq_no, DEF_INTERRUPT_PRIORITY);
}

#else // interrupt vectors table CANNOT be modified!!

//global array to save hardware ISR routines
void* OS_CPU_Vector_Table[32] = { 0 };

// Register ISR into Interrupt Vector Table
void register_ISR(uint32_t irq_no, ISR isr, ISR* isr_old)
{
    if (irq_no > IRQ_MAX)   return;
    if (isr_old) *isr_old = (ISR)(OS_CPU_Vector_Table[irq_no]);
    OS_CPU_Vector_Table[irq_no] = isr;
    // default interrupt priority is 0, we assume that
    // 0 indicates the interrupt has not been initialized before...
    if (NVIC_GetPriority(irq_no) == 0)
        NVIC_SetPriority(irq_no, DEF_INTERRUPT_PRIORITY);
}

// Function template of all interrupt handlers
#define DEFINE_INTERRUPT_HANDLER(irq)               \
    void Interrupt##irq##_Handler(void)             \
    {                                               \
        if (OS_CPU_Vector_Table[irq] != NULL)       \
            ((ISR)(OS_CPU_Vector_Table[irq]))();    \
        else                                        \
            Default_Handler();                      \
    }

// define interrupt handler functions
DEFINE_INTERRUPT_HANDLER(0);
DEFINE_INTERRUPT_HANDLER(1);
DEFINE_INTERRUPT_HANDLER(2);
DEFINE_INTERRUPT_HANDLER(3);
DEFINE_INTERRUPT_HANDLER(4);
DEFINE_INTERRUPT_HANDLER(5);
DEFINE_INTERRUPT_HANDLER(6);
DEFINE_INTERRUPT_HANDLER(7);
DEFINE_INTERRUPT_HANDLER(8);
DEFINE_INTERRUPT_HANDLER(9);
DEFINE_INTERRUPT_HANDLER(10);
DEFINE_INTERRUPT_HANDLER(11);
DEFINE_INTERRUPT_HANDLER(12);
DEFINE_INTERRUPT_HANDLER(13);
DEFINE_INTERRUPT_HANDLER(14);
DEFINE_INTERRUPT_HANDLER(15);
DEFINE_INTERRUPT_HANDLER(16);
DEFINE_INTERRUPT_HANDLER(17);
DEFINE_INTERRUPT_HANDLER(18);
DEFINE_INTERRUPT_HANDLER(19);
DEFINE_INTERRUPT_HANDLER(20);
DEFINE_INTERRUPT_HANDLER(21);
DEFINE_INTERRUPT_HANDLER(22);
DEFINE_INTERRUPT_HANDLER(23);
DEFINE_INTERRUPT_HANDLER(24);
DEFINE_INTERRUPT_HANDLER(25);
DEFINE_INTERRUPT_HANDLER(26);
DEFINE_INTERRUPT_HANDLER(27);
DEFINE_INTERRUPT_HANDLER(28);
DEFINE_INTERRUPT_HANDLER(29);
DEFINE_INTERRUPT_HANDLER(30);
DEFINE_INTERRUPT_HANDLER(31);

#endif // VECTORS_ALTERABLE

//--------------------------------------------------------

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

/*
void test_divNM()
{
    uint32_t divNM, divNM2, clk_out;

    clk_out = 60000; //KHz
    divNM = eval_freq_divNM(7, 7, 300000, clk_out, NULL);
    divNM2 = eval_freq_divNM2(7, 7, 300000, clk_out, NULL);

    clk_out = 15000; //KHz
    divNM = eval_freq_divNM(7, 7, 300000, clk_out, NULL);
    divNM2 = eval_freq_divNM2(7, 7, 300000, clk_out, NULL);

    clk_out = 100000; //KHz
    divNM = eval_freq_divNM(7, 7, 300000, clk_out, NULL);
    divNM2 = eval_freq_divNM2(7, 7, 300000, clk_out, NULL);

    clk_out = 80000; //KHz
    divNM = eval_freq_divNM(7, 7, 300000, clk_out, NULL);
    divNM2 = eval_freq_divNM2(7, 7, 300000, clk_out, NULL);

    clk_out = 81000; //KHz
    divNM = eval_freq_divNM(7, 7, 300000, clk_out, NULL);
    divNM2 = eval_freq_divNM2(7, 7, 300000, clk_out, NULL);
}
*/

#define __HAL_CP_VECTOR_MAX        32

void* CP_Vector_Table[__HAL_CP_VECTOR_MAX] = { 0 };

static void HAL_CP_Interrupt_Handler(){
    int locate = 0;
    /// get interrupt status
    uint32_t status = IP_SYSCTRL->REG_CP2AP_INTS.all;
    /// locate the leading one position, if return value == -1, mean all interrupt have been disposed
    locate = 31 - __CLZ(status);

    while(locate != -1){

        /// Run vector handler
        if (CP_Vector_Table[(uint8_t)locate]){
            ((ISR)(CP_Vector_Table[locate]))();
        }
        /// NULL, haven't been registered
        else{
            Default_Handler();
        }

        locate = 31 - __CLZ(status & ~(0x1 << locate));
    }

    /// clean all interrupt status
    IP_SYSCTRL->REG_CP2AP_INTS.all = status;
}

static uint32_t hal_global_cpint_count = 0;

void HAL_CP_Interrupt_Init(){
    hal_global_cpint_count++;
    if (hal_global_cpint_count > 1){
        return;
    }
    /// enable CP to AP interrupt
    register_ISR(IRQ_CP0_VECTOR, HAL_CP_Interrupt_Handler, NULL);
    enable_IRQ(IRQ_CP0_VECTOR);
}

void HAL_CP_Interrupt_Deinit(){
    if (hal_global_cpint_count == 0U){
        return;
    }

    hal_global_cpint_count--;
    if (hal_global_cpint_count != 0U){
        return;
    }
    disable_IRQ(IRQ_CP0_VECTOR);
    register_ISR(IRQ_CP0_VECTOR, NULL, NULL);
}

void HAL_CP_Interrupt_Enable(uint32_t irq_no){
    if (irq_no > __HAL_CP_VECTOR_MAX){
        return;
    }
    IP_SYSCTRL->REG_CP2AP_INTE.all |= (0x1 << irq_no);
}

void HAL_CP_Interrupt_Disable(uint32_t irq_no){
    if (irq_no > __HAL_CP_VECTOR_MAX){
        return;
    }
    IP_SYSCTRL->REG_CP2AP_INTE.all &= ~(0x1 << irq_no);
}

void HAL_CP_Interrupt_RegisterISR(uint32_t irq_no, ISR isr, ISR* isr_old){
    if (irq_no > __HAL_CP_VECTOR_MAX){
        return;
    }
    if (isr_old){
        *isr_old = (ISR)(CP_Vector_Table[irq_no]);
    }
    CP_Vector_Table[irq_no] = isr;
}
