/*
 * PSRAMManger.c
 *
 *  Created on: 2020年11月17日
 *      Author: USER
 */
#include "PSRAMManager.h"
#include "ClockManager.h"
#include "hal/cache.h"



#include "venus_ap.h"

#include <cmsis_gcc.h>

#define SYSCTRL_RESET_CP    (0xCAFE000B)
#define SYSCTRL_RELEASE_CP  (0xCAFE100B)
#define SYSCTRL_RESET_NPU   (0xCAFE000C)

#define max(a,b) \
    ({ __typeof__(a) _a = (a); \
       __typeof__(b) _b = (b); \
       _a > _b ? _a : _b; })

#if PSRAM_LOG_CHECK == 1
#define PSRAM_LOG(str, ...)     
#else
#define PSRAM_LOG(str, ...)
#endif
#define PSRAM_LOGE(str, ...) 

static uint32_t psram_src_array[PSRAM_SEARCH_DQS_NUM] __attribute__ ((aligned(32)));
static uint32_t psram_dst_array[PSRAM_SEARCH_DQS_NUM] __attribute__ ((aligned(32)));

extern int rand();
static void psram_data_init(void){
    unsigned int i=0;
    //init src with random data
    for (i = 0; i < PSRAM_SEARCH_DQS_NUM; i++)
    {
        psram_src_array[i] = (uint32_t)rand();
    }
}

static uint32_t psram_random_data_check(uint8_t writed){
    unsigned int ret = 0;
    unsigned int loop = 0;

    uint32_t *ptr_psram = (uint32_t *)PSRAM_BASE;

    for (loop = 0; loop < PSRAM_INNER_SEARCH_LOOP_NUM; loop++){

        if (writed){
            psram_data_init();
            __builtin_memcpy(ptr_psram, psram_src_array,       PSRAM_SEARCH_DQS_NUM * sizeof(uint32_t));
        }
        // flush the data from cache, ensure the random data reside in PSRAM
        // write back + invalidate
        dcache_flush_range((unsigned long)ptr_psram, (unsigned long)(ptr_psram + PSRAM_SEARCH_DQS_NUM));
        // reload the PSRAM data to static buffer
        __builtin_memcpy(psram_dst_array,        ptr_psram, PSRAM_SEARCH_DQS_NUM * sizeof(uint32_t));

        ret |= __builtin_memcmp(psram_src_array, psram_dst_array, PSRAM_SEARCH_DQS_NUM * sizeof(uint32_t));

        ptr_psram += PSRAM_SEARCH_DQS_NUM;
    }

    // 0 means equal
    return ret;
}

typedef struct {
    uint32_t delay_max;
    uint32_t delay_min;
    uint32_t delay_gap;
}DqsDelay_FmtDef;

static int32_t search_write_dqs_delay(uint8_t writed){
    unsigned int ret=0;
    DqsDelay_FmtDef dqs_delay = {0};
    PSRAM_LOG("[INFO][START]search_write_delay, check write delay start...\n");

    uint32_t wt_delay;
    uint8_t finded = 0;
    dqs_delay.delay_max = IP_PSRAMC->REG_DLL_DELAY.bit.WRLVL_DELAY;
    dqs_delay.delay_min = IP_PSRAMC->REG_DLL_DELAY.bit.WRLVL_DELAY;

    for(wt_delay=dqs_delay.delay_max; wt_delay <= PSRAM_EYE_DIAGRAM_TOP; wt_delay++){
        PSRAM_LOG("[SEARCH] WRITE DELAY: 0x%x", wt_delay);
        IP_PSRAMC->REG_DLL_DELAY.bit.WRLVL_DELAY = wt_delay;
        IP_PSRAMC->REG_DLL_RESYNC.bit.DLL_RESYNC = 1;

        ret = psram_random_data_check(writed);

        if (0 != ret){
            PSRAM_LOG("[FAILED]");
            // 连续的成功后失败需要退出当前循环
            if (finded == 1){
                dqs_delay.delay_max = (wt_delay - 1);
                PSRAM_LOG("[INFO], max_wt_delay=0x%x", dqs_delay.delay_max);
                break;
            }
        } else{
            // 遇到第一个成功的位置，记录并且finded置位
            PSRAM_LOG("[SUCCESSED]");
            if (finded == 0){
                dqs_delay.delay_min = wt_delay;
                finded = 1;
                PSRAM_LOG("[INFO], min_wt_delay=0x%x", dqs_delay.delay_min);
            }
        }
        if (wt_delay == PSRAM_EYE_DIAGRAM_TOP){
            if (finded == 1){
                dqs_delay.delay_max = wt_delay;
            }
            PSRAM_LOG("data compare shut! ,max_wt_delay=0x%x", dqs_delay.delay_max);
            break;
        }
    }

    for(wt_delay=dqs_delay.delay_min; wt_delay >= PSRAM_EYE_DIAGRAM_BOTTOM; wt_delay--){
        PSRAM_LOG("[SEARCH] WRITE DELAY: 0x%x", wt_delay);
        IP_PSRAMC->REG_DLL_DELAY.bit.WRLVL_DELAY = wt_delay;
        IP_PSRAMC->REG_DLL_RESYNC.bit.DLL_RESYNC = 1;

        ret = psram_random_data_check(writed);

        if (0 != ret){
            PSRAM_LOG("[FAILED]");
            if (finded == 1){
                dqs_delay.delay_min = (wt_delay + 1);
                PSRAM_LOG("[INFO], min_wt_delay=0x%x", dqs_delay.delay_min);
                break;
            }
        } else{
            PSRAM_LOG("[SUCCESSED]");
            if (finded == 0){
                dqs_delay.delay_max = wt_delay;
                finded = 1;
                PSRAM_LOG("[INFO], max_wt_delay=0x%x", dqs_delay.delay_max);
            }
        }
        if (wt_delay == 0){
            if (finded == 1){
                dqs_delay.delay_min = wt_delay;
            }
            PSRAM_LOG("data compare shut! ,min_wt_delay=0x%x", dqs_delay.delay_min);
            break;
        }
    }

    if (finded == 0){
        PSRAM_LOGE("[INFO][END] Search finished, Failed!!! Psram initialize program can't find EYE DIAGRAM!!!");
        return -1;
    }

    dqs_delay.delay_gap = (dqs_delay.delay_max - dqs_delay.delay_min)/2 + dqs_delay.delay_min;
    PSRAM_LOG("[INFO][END] Search finished, write delay_gap=0x%x", dqs_delay.delay_gap);

    IP_PSRAMC->REG_DLL_DELAY.bit.WRLVL_DELAY = dqs_delay.delay_gap;
    IP_PSRAMC->REG_DLL_RESYNC.bit.DLL_RESYNC = 1;

    return 0;
}

static int32_t search_read_dqs_delay(uint8_t writed){
    unsigned int ret=0;
    DqsDelay_FmtDef dqs_delay = {0};
    PSRAM_LOG("[INFO][START]search_read_delay, check read delay start...\n");

    uint32_t rd_delay;
    uint8_t finded = 0;
    dqs_delay.delay_max = IP_PSRAMC->REG_DLL_DELAY.bit.RDLVL_DELAY;
    dqs_delay.delay_min = IP_PSRAMC->REG_DLL_DELAY.bit.RDLVL_DELAY;

    for(rd_delay=dqs_delay.delay_max; rd_delay <= PSRAM_EYE_DIAGRAM_TOP; rd_delay++){
        PSRAM_LOG("[SEARCH] READ DELAY: 0x%x", rd_delay);
        IP_PSRAMC->REG_DLL_DELAY.bit.RDLVL_DELAY = rd_delay;
        IP_PSRAMC->REG_DLL_RESYNC.bit.DLL_RESYNC = 1;

        ret = psram_random_data_check(writed);

        if (0 != ret){
            PSRAM_LOG("[FAILED]");
            // 连续的成功后失败需要退出当前循环
            if (finded == 1){
                dqs_delay.delay_max = (rd_delay - 1);
                PSRAM_LOG("[INFO], max_rd_delay=0x%x", dqs_delay.delay_max);
                break;
            }
        } else{
            // 遇到第一个成功的位置，记录并且finded置位
            PSRAM_LOG("[SUCCESSED]");
            if (finded == 0){
                dqs_delay.delay_min = rd_delay;
                finded = 1;
                PSRAM_LOG("[INFO], min_rd_delay=0x%x", dqs_delay.delay_min);
            }
        }
        if (rd_delay == 0xB4){
            if (finded == 1){
                dqs_delay.delay_max = rd_delay;
            }
            PSRAM_LOG("data compare shut! ,max_rd_delay=0x%x", dqs_delay.delay_max);
            break;
        }
    }

    for(rd_delay=dqs_delay.delay_min; rd_delay >= PSRAM_EYE_DIAGRAM_BOTTOM; rd_delay--){
        PSRAM_LOG("[SEARCH] READ DELAY: 0x%x", rd_delay);
        IP_PSRAMC->REG_DLL_DELAY.bit.RDLVL_DELAY = rd_delay;
        IP_PSRAMC->REG_DLL_RESYNC.bit.DLL_RESYNC = 1;

        ret = psram_random_data_check(writed);

        if (0 != ret){
            PSRAM_LOG("[FAILED]");
            if (finded == 1){
                dqs_delay.delay_min = (rd_delay + 1);
                PSRAM_LOG("[INFO]  ,min_rd_delay=0x%x", dqs_delay.delay_min);
                break;
            }
        } else{
            PSRAM_LOG("[SUCCESSED]");
            if (finded == 0){
                dqs_delay.delay_max = rd_delay;
                finded = 1;
                PSRAM_LOG("[INFO], max_rd_delay=0x%x", dqs_delay.delay_max);
            }
        }
        if (rd_delay == 0){
            if (finded == 1){
                dqs_delay.delay_min = rd_delay;
            }
            PSRAM_LOG("data compare shut! ,min_rd_delay=0x%x", dqs_delay.delay_min);
            break;
        }
    }

    if (finded == 0){
        PSRAM_LOGE("[INFO][END] Search finished, Failed!!! Psram initialize program can't find EYE DIAGRAM!!!");
        return -1;
    }

    dqs_delay.delay_gap = (dqs_delay.delay_max - dqs_delay.delay_min)/2 + dqs_delay.delay_min;
    PSRAM_LOG("[INFO][END] Search finished, read delay_gap=0x%x", dqs_delay.delay_gap);

    IP_PSRAMC->REG_DLL_DELAY.bit.RDLVL_DELAY = dqs_delay.delay_gap;
    IP_PSRAMC->REG_DLL_RESYNC.bit.DLL_RESYNC = 1;

    return 0;
}

__unused static void check_reg(uint32_t* address){
    unsigned int rdata = 0x0;

    rdata = *address;
    PSRAM_LOG("0x%x reg: 0x%x",address, rdata);
}

#if defined(CSK6001)

static void PSRAM_ISR_Handler(){
    /// CS time out
    if (IP_PSRAMC->REG_INTR.bit.CS_TMO_IRSR){
        IP_PSRAMC->REG_INTR.bit.CS_TMO_ICR = 0x1;
        PSRAM_LOG("CS timeout");
    }

    if (IP_PSRAMC->REG_INTR.bit.RD_MEM_TMO_IRSR){
        IP_PSRAMC->REG_INTR.bit.RD_MEM_TMO_ICR = 0x1;
        PSRAM_LOG("Memory rd timeout");
    }

    if (IP_PSRAMC->REG_INTR.bit.RD_MR_TMO_IRSR){
        IP_PSRAMC->REG_INTR.bit.RD_MR_TMO_ICR = 0x1;
        PSRAM_LOG("MR rd timeout");
    }
}
#endif

static uint32_t MR_REG[9] = {0};

int32_t PSRAM_Initialize(void){
    // Clock reset
    IP_SYSCTRL->REG_SW_RESET2.bit.PSRAM_CTRL_RESET = 0x1;

    // enable PSRAM clock
    IP_SYSCTRL->REG_NVM_CLK_CFG.bit.ENA_PSRAM_CLK = 0x1;
    IP_ANACORE->REG_PSRAMIO_MISC_CFG.bit.PSRAMIO_VREF_CFG = 63;
    IP_ANACORE->REG_PSRAMIO_MISC_CFG.bit.PSRAMIO_CLK_DRV_CFG = 0x2;

    // reset CP & NPU
    if(IP_SYSCTRL->REG_SW_RESET_CP.all){ // = 1 CP reset
        // pass
    } else{
        IP_SYSCTRL->REG_SW_RESET_CP.all = SYSCTRL_RESET_CP;
    }

    if(IP_SYSCTRL->REG_SW_RESET_NPU.all){ // = 1 NPU reset
        // pass
    } else{
        IP_SYSCTRL->REG_SW_RESET_NPU.all = SYSCTRL_RESET_NPU;
    }

#if PSRAM_ODT_ENA
    { // ODT
        IP_ANACORE->REG_PSRAMIO_DQ0_CFG.bit.PSRAMIO_DQ0_DRV_CFG = 0x7;
        IP_ANACORE->REG_PSRAMIO_DQ0_CFG.bit.PSRAMIO_DQ0_ODT_CFG = 0x0;
        IP_ANACORE->REG_PSRAMIO_DQ0_CFG.bit.PSRAMIO_DQ0_NCALIB = 0x7;
        IP_ANACORE->REG_PSRAMIO_DQ0_CFG.bit.PSRAMIO_DQ0_ODT_EN_DN_VAL = 0x1;

        IP_ANACORE->REG_PSRAMIO_DQ1_CFG.bit.PSRAMIO_DQ1_DRV_CFG = 0x7;
        IP_ANACORE->REG_PSRAMIO_DQ1_CFG.bit.PSRAMIO_DQ1_ODT_CFG = 0x0;
        IP_ANACORE->REG_PSRAMIO_DQ1_CFG.bit.PSRAMIO_DQ1_NCALIB = 0x7;
        IP_ANACORE->REG_PSRAMIO_DQ1_CFG.bit.PSRAMIO_DQ1_ODT_EN_DN_VAL = 0x1;

        IP_ANACORE->REG_PSRAMIO_DQ2_CFG.bit.PSRAMIO_DQ2_DRV_CFG = 0x7;
        IP_ANACORE->REG_PSRAMIO_DQ2_CFG.bit.PSRAMIO_DQ2_ODT_CFG = 0x0;
        IP_ANACORE->REG_PSRAMIO_DQ2_CFG.bit.PSRAMIO_DQ2_NCALIB = 0x7;
        IP_ANACORE->REG_PSRAMIO_DQ2_CFG.bit.PSRAMIO_DQ2_ODT_EN_DN_VAL = 0x1;

        IP_ANACORE->REG_PSRAMIO_DQ3_CFG.bit.PSRAMIO_DQ3_DRV_CFG = 0x7;
        IP_ANACORE->REG_PSRAMIO_DQ3_CFG.bit.PSRAMIO_DQ3_ODT_CFG = 0x0;
        IP_ANACORE->REG_PSRAMIO_DQ3_CFG.bit.PSRAMIO_DQ3_NCALIB = 0x7;
        IP_ANACORE->REG_PSRAMIO_DQ3_CFG.bit.PSRAMIO_DQ3_ODT_EN_DN_VAL = 0x1;

        IP_ANACORE->REG_PSRAMIO_DQ4_CFG.bit.PSRAMIO_DQ4_DRV_CFG = 0x7;
        IP_ANACORE->REG_PSRAMIO_DQ4_CFG.bit.PSRAMIO_DQ4_ODT_CFG = 0x0;
        IP_ANACORE->REG_PSRAMIO_DQ4_CFG.bit.PSRAMIO_DQ4_NCALIB = 0x7;
        IP_ANACORE->REG_PSRAMIO_DQ4_CFG.bit.PSRAMIO_DQ4_ODT_EN_DN_VAL = 0x1;

        IP_ANACORE->REG_PSRAMIO_DQ5_CFG.bit.PSRAMIO_DQ5_DRV_CFG = 0x7;
        IP_ANACORE->REG_PSRAMIO_DQ5_CFG.bit.PSRAMIO_DQ5_ODT_CFG = 0x0;
        IP_ANACORE->REG_PSRAMIO_DQ5_CFG.bit.PSRAMIO_DQ5_NCALIB = 0x7;
        IP_ANACORE->REG_PSRAMIO_DQ5_CFG.bit.PSRAMIO_DQ5_ODT_EN_DN_VAL = 0x1;

        IP_ANACORE->REG_PSRAMIO_DQ6_CFG.bit.PSRAMIO_DQ6_DRV_CFG = 0x7;
        IP_ANACORE->REG_PSRAMIO_DQ6_CFG.bit.PSRAMIO_DQ6_ODT_CFG = 0x0;
        IP_ANACORE->REG_PSRAMIO_DQ6_CFG.bit.PSRAMIO_DQ6_NCALIB = 0x7;
        IP_ANACORE->REG_PSRAMIO_DQ6_CFG.bit.PSRAMIO_DQ6_ODT_EN_DN_VAL = 0x1;

        IP_ANACORE->REG_PSRAMIO_DQ7_CFG.bit.PSRAMIO_DQ7_DRV_CFG = 0x7;
        IP_ANACORE->REG_PSRAMIO_DQ7_CFG.bit.PSRAMIO_DQ7_ODT_CFG = 0x0;
        IP_ANACORE->REG_PSRAMIO_DQ7_CFG.bit.PSRAMIO_DQ7_NCALIB = 0x7;
        IP_ANACORE->REG_PSRAMIO_DQ7_CFG.bit.PSRAMIO_DQ7_ODT_EN_DN_VAL = 0x1;

        IP_ANACORE->REG_PSRAMIO_DQS_CFG.bit.PSRAMIO_DQS_DRV_CFG = 0x7;
        IP_ANACORE->REG_PSRAMIO_DQS_CFG.bit.PSRAMIO_DQS_ODT_CFG = 0x0;
        IP_ANACORE->REG_PSRAMIO_DQS_CFG.bit.PSRAMIO_DQS_NCALIB = 0x7;
        IP_ANACORE->REG_PSRAMIO_DQS_CFG.bit.PSRAMIO_DQS_ODT_EN_DN_VAL = 0x1;
    }
#endif

    /// Prefetch mode
#if defined(CSK6001)
    IP_PSRAMC->REG_CLK_CS_OE_CTRL.bit.DM_EN = 0x1;
    /// PSRAM prefetch FIFO depth
    IP_PSRAMC->REG_DDR_PREFT.bit.PREFT_CNT = PSRAM_PREFETCH_FIFO_DEPTH;
    PSRAM_LOG("[INFO]PSRAM Prefetch level: %u, FIFO depth: %u", PSRAM_PREFETCH_LEVEL, PSRAM_PREFETCH_FIFO_DEPTH);
    switch (PSRAM_PREFETCH_LEVEL){
    case _prefetch_level3:
        IP_PSRAMC->REG_DDR_PREFT.bit.PREFT_EN = 0x1;
        IP_PSRAMC->REG_DDR_PREFT.bit.RFIFO_CLR_EN = 0x0;
        IP_PSRAMC->REG_DDR_PREFT.bit.WAR_EN = 0x1;
        {
            uint32_t clk, sys;
#if IC_BOARD == 0
            clk = BOARD_BOOTCLOCKRUN_PSRAM_CLK;
#else
            clk = CRM_GetPsramFreq();
#endif
            sys = CRM_GetApFreq();
            IP_PSRAMC->REG_DDR_PREFT.bit.PREFT_PRED_NUM = 3*(clk/sys)/2 + 3 + PSRAM_PREFETCH_PRED_NUM;
        }
        break;
    case _prefetch_level2:
        IP_PSRAMC->REG_DDR_PREFT.bit.PREFT_EN = 0x1;
        IP_PSRAMC->REG_DDR_PREFT.bit.RFIFO_CLR_EN = 0x1;
        IP_PSRAMC->REG_DDR_PREFT.bit.WAR_EN = 0x0;
        break;
    case _prefetch_level1:
        IP_PSRAMC->REG_DDR_PREFT.bit.PREFT_EN = 0x1;
        IP_PSRAMC->REG_DDR_PREFT.bit.RFIFO_CLR_EN = 0x0;
        IP_PSRAMC->REG_DDR_PREFT.bit.WAR_EN = 0x0;
        break;
    case _prefetch_level0:
        IP_PSRAMC->REG_DDR_PREFT.bit.PREFT_EN = 0x0;
        break;
    default:
        PSRAM_LOG("[ERROR]PSRAM Prefetch level not defined");
    }
#endif

    // read command
    PSRAM_LOG("[INFO]PSRAM Write memory command: 0x%x", IP_PSRAMC->REG_MEM_CMD.bit.WRITE_MEM_CMD);
    PSRAM_LOG("[INFO]PSRAM Read memory command: 0x%x", IP_PSRAMC->REG_MEM_CMD.bit.READ_MEM_CMD);
    PSRAM_LOG("[INFO]PSRAM Write register command: 0x%x", IP_PSRAMC->REG_MEM_CMD.bit.WRITE_REG_CMD);
    PSRAM_LOG("[INFO]PSRAM Read register command: 0x%x", IP_PSRAMC->REG_MEM_CMD.bit.READ_REG_CMD);

    IP_PSRAMC->REG_DDR_TIM_CONFIG.bit.TCSP_CFG = CONTROLLER_TIME_CONFIG_TCSP;

    // set DLL_MASTER_CTRL (0x10)
    IP_PSRAMC->REG_DDR_MASTER_CTRL.bit.DLL_START_POINT = 0x4;
    IP_PSRAMC->REG_DDR_MASTER_CTRL.bit.PHASE_DETECT_SEL = 0x7;

    // DLL choose bypass mode or none bypass mode
    IP_PSRAMC->REG_DDR_MASTER_CTRL.bit.DLL_BYPASS = 0;
    /// DLL lock
#if defined(CSK6001)
    // reset DLL (0x24)
    IP_PSRAMC->REG_DDR_MASTER_CTRL.bit.DLL_EN = 0x1;
#endif

    // check DLL lock done (0x20)
    {
        uint32_t rdata = 0x0;
        uint32_t max_loop_times = 100000;
        do {
            rdata = IP_PSRAMC->REG_LOCK_DONE.bit.LOCK_DONE;
        }while ((!(rdata & 0x1)) && (max_loop_times--));

        if (rdata == 0){
            PSRAM_LOGE("[ERROR]DLL line lock done failed");
        } else {
            PSRAM_LOG("[INFO]DLL line lock Done\n");
        }
    }

    // set priority (0x40)
    IP_PSRAMC->REG_PRIO_CTRL.all = CONTROLLER_PRIO_SETTING;

    // PSRAM max CE# low width (0x4)
    // tcem * clock < 4us
    // tcph = 6 (200mhz)
    //      = 4 (133mhz)
#if defined(CSK6001_MPW)
    IP_PSRAMC->REG_DDR_TIM_CONFIG.all = ((CONTROLLER_TIME_CONFIG_TCSP << 12) | (CONTROLLER_TIME_CONFIG_TCEM << 8) | (CONTROLLER_TIME_CONFIG_TCPH));
#elif defined(CSK6001)
    {
        uint32_t clk, ahb_clk;
        int32_t sap, tag;
#if IC_BOARD == 0
        clk = BOARD_BOOTCLOCKRUN_PSRAM_CLK;
#else
        clk = CRM_GetPsramFreq();
#endif
        ahb_clk = CRM_GetApahbFreq();

        /// Set CS max count
        IP_PSRAMC->REG_CS_MAX.bit.CS_MAX = (uint32_t)((float)clk * (float)4.0 / __INNER_MICROSEC);
        sap = (uint32_t)((float)clk * (float)PSRAM_REFRESH_TIME / __INNER_MICROSEC);

        tag = sap - 6;
        if (tag <= 0){
            tag = 1;
        }
        IP_PSRAMC->REG_DDR_REFRESH.bit.TCEM_WR = tag;
        PSRAM_LOG("[INFO]WT TCEM: 0x%x", tag);

        tag = sap - (max((2 + (clk/ahb_clk)*2), ((clk/ahb_clk)*3))\
                    + ((clk/ahb_clk)*2)\
                    + 7);
        if (tag <= 0){
            tag = 1;
        }
        PSRAM_LOG("[INFO]RD TCEM: 0x%x", tag);
        IP_PSRAMC->REG_DDR_TIM_CONFIG.all = ((tag << 20) | (CONTROLLER_TIME_CONFIG_TCSP << 12) | (CONTROLLER_TIME_CONFIG_TCPH));
        sap = (sap - 2 - (CONTROLLER_MEM_R_WAIT_CYCLES*2) - 2 - 4)/2;
        if (sap <= 0){
            sap = 1;
        }
        IP_PSRAMC->REG_DDR_REFRESH.bit.RD_CNT_MAX = sap;
        PSRAM_LOG("[INFO]RD MAX CNT: 0x%x", sap);
    }
#endif

    /// PSRAM Interrupt mode
#if PSRAM_CS_TMO_INT_ENABLE || PSRAM_RD_TMO_INT_ENABLE
    HAL_CP_Interrupt_RegisterISR(IRQ_CP_PSRAM_VECTOR, PSRAM_ISR_Handler, NULL);
    HAL_CP_Interrupt_Enable(IRQ_CP_PSRAM_VECTOR);
    HAL_CP_Interrupt_Init();

    /// Enable PSRAM interrupt
    IP_PSRAMC->REG_INTR.bit.RD_TMO_EN = PSRAM_RD_TMO_INT_ENABLE;
    IP_PSRAMC->REG_INTR.bit.CS_TMO_EN = PSRAM_CS_TMO_INT_ENABLE;
#endif

    // set DQS delay cell (0x14)
    // 0x18_(dll_lock_value) * 0x14_(delay)/256 = 0x1c_(delay)
    {
        /// PSRAM DLL会去测量一个PSRAM一个时钟周期需要几个内部固定 delay cell，
        /// 如果测量了一整个时钟周期，则1/4可以通过delay cell / 4，如果时钟周期过大，则
        /// 只能测量半个周期，所以 HALF_CLOCK_MODE置位，采用1/2位系数。
        uint32_t div = IP_PSRAMC->REG_DLL_OBS_REG0.bit.HALF_CLOCK_MODE;
        if (div){
            div = 2;
        } else{
            div = 4;
        }
#if DEFAULT_RD_DELAY == 0
        IP_PSRAMC->REG_DLL_DELAY.bit.RDLVL_DELAY = IP_PSRAMC->REG_DLL_OBS_REG0.bit.DLL_LOCK_VALUE/div;
#else
        IP_PSRAMC->REG_DLL_DELAY.bit.RDLVL_DELAY = DEFAULT_RD_DELAY;
#endif
#if DEFAULT_WT_DELAY == 0
        IP_PSRAMC->REG_DLL_DELAY.bit.WRLVL_DELAY = IP_PSRAMC->REG_DLL_OBS_REG0.bit.DLL_LOCK_VALUE/div;
#else
        IP_PSRAMC->REG_DLL_DELAY.bit.WRLVL_DELAY = DEFAULT_WT_DELAY;
#endif
        PSRAM_LOG("[INFO]Read Delay: 0x%x; Write Delay: 0x%x", IP_PSRAMC->REG_DLL_DELAY.bit.RDLVL_DELAY, IP_PSRAMC->REG_DLL_DELAY.bit.WRLVL_DELAY);
    }

    // set dll
    // set 1 means: use delay cell
    // set 0 means: percentage
    IP_PSRAMC->REG_DDR_MASTER_CTRL.bit.DLL_BYPASS = PSRAM_DLL_BYPASS;

    // resync DLL (0x28)
    IP_PSRAMC->REG_DLL_RESYNC.bit.DLL_RESYNC = 0x1;

    // chip initialize (0x30)
    IP_PSRAMC->REG_CHIP_CONFIG.bit.CHIP_INIT = 0x1;

    // set DDR wait cycle, before send reset command to PSRAM
    // PSRAM reset w_wait must be zero

    IP_PSRAMC->REG_DDR_WAIT.bit.MEM_R_WAIT = CONTROLLER_MEM_R_WAIT_CYCLES;
    // set write wait to zero
    // 这里使用默认参数，是因为这时PSRAM还没有配置成功
    IP_PSRAMC->REG_DDR_WAIT.bit.MEM_W_WAIT = 0x0;   // default parameter
    IP_PSRAMC->REG_DDR_WAIT.bit.MODE_R_WAIT = 0x4;

    // reset PSRAM
    {
        // command to reset PSRAM (0x0)
        IP_PSRAMC->REG_MEM_CMD.bit.WRITE_REG_CMD = 0xff;

        // send PSRAM reset
        IP_PSRAMC->REG_MR0.all = 0x0;
        // wait
        {
            int i, j;
            for(i = 0; i < 100; i++){
                for(j = 0; j < 1000; j++);
            }
        }

        // reset command
        IP_PSRAMC->REG_MEM_CMD.bit.WRITE_REG_CMD = 0xc0;
    }

    {
        uint32_t i = 100;
        uint32_t rdata = 0;

        // read latency
        while(i--){
            rdata = IP_PSRAMC->REG_MR0.all;
            if ((rdata & 0xff) == ((PSRAM_MR0_LT << 5) | (PSRAM_MEM_R_WAIT_CYCLES << 2) | PSRAM_MR0_DRIVE_STR)){
                break;
            }
            IP_PSRAMC->REG_MR0.all = ((PSRAM_MR0_LT << 5) | (PSRAM_MEM_R_WAIT_CYCLES << 2) | PSRAM_MR0_DRIVE_STR);

            // adjust to current parameter
            IP_PSRAMC->REG_DDR_WAIT.bit.MODE_R_WAIT = CONTROLLER_MODE_R_WAIT_CYCLES;
        }

        if (i == 0){
            PSRAM_LOGE("PSRAM MR0 configure error");
        }

        // wrap 1K
        i = 100;
        while(i--){
            rdata = IP_PSRAMC->REG_MR8.all;
            if ((rdata & 0xff) == ((PSRAM_MR8_BT << 2) | PSRAM_MR8_BL)){
                break;
            }
            IP_PSRAMC->REG_MR8.all = ((PSRAM_MR8_BT << 2) | PSRAM_MR8_BL);
        }

        // write latency
        i = 100;
        while(i--){
            rdata = IP_PSRAMC->REG_MR4.all;
            if ((rdata & 0xff) == ((PSRAM_MEM_W_WAIT_CYCLES << 5) | (PSRAM_MR4_RF << 3) | PSRAM_MR4_PASR)){
                IP_PSRAMC->REG_DDR_WAIT.bit.MEM_W_WAIT = CONTROLLER_MEM_W_WAIT_CYCLES;
                break;
            }
            IP_PSRAMC->REG_MR4.all = ((PSRAM_MEM_W_WAIT_CYCLES << 5) | (PSRAM_MR4_RF << 3) | PSRAM_MR4_PASR);
        }

        MR_REG[0] = IP_PSRAMC->REG_MR0.all;
        MR_REG[1] = IP_PSRAMC->REG_MR1.all;
        MR_REG[2] = IP_PSRAMC->REG_MR2.all;
        MR_REG[3] = IP_PSRAMC->REG_MR3.all;
        MR_REG[4] = IP_PSRAMC->REG_MR4.all;
        MR_REG[6] = IP_PSRAMC->REG_MR6.all;
        MR_REG[8] = IP_PSRAMC->REG_MR8.all;

    }

    int32_t ret = 0;

#if PSRAM_WT_DQS_SEARCH
    ret |= search_write_dqs_delay(true);
#endif

#if PSRAM_RD_DQS_SEARCH
    ret |= search_read_dqs_delay(true);
#endif

    return ret;
}

void PSRAM_HalfSleep_Mode(void){
	// close CP
	// synchronization barrier. full system, any-any = 15
	__DSB();
	// enter sleep mode
	IP_PSRAMC->REG_MR6.all = 0xf0;
	// close clock
	IP_PSRAMC->REG_CLK_CS_OE_CTRL.bit.REG_CLK_EN = 0x0;
	// wait 4us
	uint32_t i = CRM_GetApFreq()/10000;  // -> approximate to 100us
	while(i--);
}

void PSRAM_Exit_HalfSleep_Mode(void){
	// change write latency
	IP_PSRAMC->REG_DDR_WAIT.bit.MEM_W_WAIT = 1;
	// write a fake data to psram core
	IP_PSRAMC->REG_MR6.all = 0x0;

	// restore write latency
	IP_PSRAMC->REG_DDR_WAIT.bit.MEM_W_WAIT = CONTROLLER_MEM_W_WAIT_CYCLES;

	// wait 150us
	uint32_t i = CRM_GetApFreq()/1000; // -> approximate to 1ms
	while(i--);

	// enable clock
	IP_PSRAMC->REG_CLK_CS_OE_CTRL.bit.REG_CLK_EN = 0x1;
}

void PSRAM_DeepSleep_Mode(void){
	// close CP
	// synchronization barrier. full system, any-any = 15
	__DSB();
	// enter sleep mode
	IP_PSRAMC->REG_MR6.all = 0xc0;
	// close clock
	IP_PSRAMC->REG_CLK_CS_OE_CTRL.bit.REG_CLK_EN = 0x0;
	// wait 500us
	uint32_t i = CRM_GetApFreq()/500; // -> approximate to 2ms
	while(i--);
}

void PSRAM_Exit_DeepSleep_Mode(void){
	// change write latency
	IP_PSRAMC->REG_DDR_WAIT.bit.MEM_W_WAIT = 1;
	// write a fake data to psram core
	IP_PSRAMC->REG_MR6.all = 0x0;

	// restore write latency
	IP_PSRAMC->REG_DDR_WAIT.bit.MEM_W_WAIT = CONTROLLER_MEM_W_WAIT_CYCLES;

	// wait 150us
	uint32_t i = CRM_GetApFreq()/1000; // -> approximate to 1ms
	while(i--);

	// enable clock
	IP_PSRAMC->REG_CLK_CS_OE_CTRL.bit.REG_CLK_EN = 0x1;

	{
        IP_PSRAMC->REG_MR0.all = MR_REG[0];
        IP_PSRAMC->REG_MR1.all = MR_REG[1];
        IP_PSRAMC->REG_MR2.all = MR_REG[2];
        IP_PSRAMC->REG_MR3.all = MR_REG[3];
        IP_PSRAMC->REG_MR4.all = MR_REG[4];
        IP_PSRAMC->REG_MR6.all = MR_REG[6];
        IP_PSRAMC->REG_MR8.all = MR_REG[8];

	}
}
