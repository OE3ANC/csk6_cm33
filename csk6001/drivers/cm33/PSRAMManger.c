/*
 * PSRAMManger.c
 *
 *  Created on: 2020年11月17日
 *      Author: USER
 */
#include "hal/PSRAMManager.h"
#include "hal/ClockManager.h"
#include "hal/cache.h"

#include "hal/venus_ap.h"

#include "hal/reg/cmn_sysctrl_reg_venus.h"
#include "hal/reg/ana_core_regfile_reg_venus.h"
#include "hal/reg/psram_mc_reg_venus.h"

#define SYSTEM_CTRL_BASE    ((SYSCFG_RegDef* ) CMN_SYSCTRL_BASE)
#define PSRAM_CTRL_BASE     ((PSRAM_MC_RegDef*) CMN_PSRAMC_BASE)
#define PSRAM_IO_BASE       ((ANA_CORE_RegDef*) CMN_ANACORE_BASE)

#define SYSCTRL_RESET_CP    (0xCAFE000B)
#define SYSCTRL_RELEASE_CP  (0xCAFE100B)
#define SYSCTRL_RESET_NPU   (0xCAFE000C)

#if PSRAM_LOG_CHECK == 1
#define PSRAM_LOG(str, ...)  CLOGD(str, ##__VA_ARGS__)
#else
#define PSRAM_LOG(str, ...)
#endif
#define PSRAM_LOGE(str, ...) printk(str, ##__VA_ARGS__)


static uint32_t psram_src_array[PSRAM_SEARCH_DQS_NUM] __attribute__ ((aligned(32)));
static uint32_t psram_dst_array[PSRAM_SEARCH_DQS_NUM] __attribute__ ((aligned(32)));

static void psram_data_init(void){
    unsigned int i=0;
    //init src with random data
    for (i = 0; i < PSRAM_SEARCH_DQS_NUM; i++)
    {
        psram_src_array[i] = (uint32_t)rand();
    }
}

static uint32_t psram_read_data_check(void){
    unsigned int ret = 0;

    uint32_t *ptr_psram = (uint32_t *)PSRAM_BASE;

#if !PSRAM_WT_DQS_SEARCH
    psram_data_init();
    __builtin_memcpy(ptr_psram, psram_src_array,       sizeof(psram_src_array));
#endif
    // flush the data from cache, ensure the random data reside in PSRAM
    // write back + invalidate
    dcache_flush_range((unsigned long)ptr_psram, (unsigned long)(ptr_psram + sizeof(psram_src_array)));
    // reload the PSRAM data to static buffer
    __builtin_memcpy(psram_dst_array,        ptr_psram, sizeof(psram_dst_array));

    ret = __builtin_memcmp(psram_src_array, psram_dst_array, sizeof(psram_src_array));

    // 0 means equal
    return ret;
}

static uint32_t psram_write_data_check(void){
    unsigned int ret = 0;

    uint32_t *ptr_psram = (uint32_t *)PSRAM_BASE;

    psram_data_init();
    __builtin_memcpy(ptr_psram, psram_src_array,       sizeof(psram_src_array));
    // flush the data from cache, ensure the random data reside in PSRAM
    // write back + invalidate
    dcache_flush_range((unsigned long)ptr_psram, (unsigned long)(ptr_psram + sizeof(psram_src_array)));
    // reload the PSRAM data to static buffer
    __builtin_memcpy(psram_dst_array,        ptr_psram, sizeof(psram_dst_array));

    ret = __builtin_memcmp(psram_src_array, psram_dst_array, sizeof(psram_src_array));

    // 0 means equal
    return ret;
}


typedef struct {
    uint32_t delay_max;
    uint32_t delay_min;
    uint32_t delay_gap;
}DqsDelay_FmtDef;

static void search_write_dqs_delay(){
    unsigned int ret=0;
    DqsDelay_FmtDef dqs_delay = {0};
    PSRAM_LOG("search_write_delay, check write delay start...\n");

    uint32_t wt_delay;
    uint8_t finded = 0;
    dqs_delay.delay_max = PSRAM_CTRL_BASE->REG_DLL_DELAY.bit.WRLVL_DELAY;
    dqs_delay.delay_min = PSRAM_CTRL_BASE->REG_DLL_DELAY.bit.WRLVL_DELAY;

    for(wt_delay=dqs_delay.delay_max; wt_delay <= 0xFF; wt_delay++){
        PSRAM_LOG("[SEARCH] WRITE DELAY: 0x%x", wt_delay);
        PSRAM_CTRL_BASE->REG_DLL_DELAY.bit.WRLVL_DELAY = wt_delay;
        PSRAM_CTRL_BASE->REG_DLL_RESYNC.bit.DLL_RESYNC = 1;
        ret = psram_write_data_check();
        if (0 != ret){
            PSRAM_LOG("data compare FAILED! ,max_wt_delay=%d\n", wt_delay);
            // 连续的成功后失败需要退出当前循环
            if (finded == 1){
                dqs_delay.delay_max = (wt_delay - 1);
                break;
            }
        } else{
            // 遇到第一个成功的位置，记录并且finded置位
            PSRAM_LOG("data compare SUCCESSED! ,delay=%d\n", wt_delay);
            if (finded == 0){
                dqs_delay.delay_min = wt_delay;
                finded = 1;
            }
        }
        if (wt_delay == 0xB4){
            PSRAM_LOG("data compare shut! ,max_wt_delay=%d\n", wt_delay);
            if (finded == 1){
                dqs_delay.delay_max = wt_delay;
            }
            break;
        }
    }

    for(wt_delay=dqs_delay.delay_min; wt_delay >= 0; wt_delay--){
        PSRAM_LOG("[SEARCH] WRITE DELAY: 0x%x", wt_delay);
        PSRAM_CTRL_BASE->REG_DLL_DELAY.bit.WRLVL_DELAY = wt_delay;
        PSRAM_CTRL_BASE->REG_DLL_RESYNC.bit.DLL_RESYNC = 1;
        ret = psram_write_data_check();
        if (0 != ret){
            PSRAM_LOG("data compare FAILED! ,min_wt_delay=%d\n", wt_delay);
            if (finded == 1){
                dqs_delay.delay_min = (wt_delay + 1);
                break;
            }
        } else{
            PSRAM_LOG("data compare SUCCESSED! ,delay=%d\n", wt_delay);
            if (finded == 0){
                dqs_delay.delay_max = wt_delay;
                finded = 1;
            }
        }
        if (wt_delay == 0){
            PSRAM_LOG("data compare shut! ,min_wt_delay=%d\n", wt_delay);
            if (finded == 1){
                dqs_delay.delay_min = wt_delay;
            }
            break;
        }
    }

    dqs_delay.delay_gap = (dqs_delay.delay_max - dqs_delay.delay_min)/2 + dqs_delay.delay_min;
    PSRAM_LOG("search finished, delay_gap=%d\n", dqs_delay.delay_gap);

    PSRAM_CTRL_BASE->REG_DLL_DELAY.bit.WRLVL_DELAY = dqs_delay.delay_gap;
    PSRAM_CTRL_BASE->REG_DLL_RESYNC.bit.DLL_RESYNC = 1;
}

static void search_read_dqs_delay(){
    unsigned int ret=0;
    DqsDelay_FmtDef dqs_delay = {0};
    PSRAM_LOG("search_read_delay, check read delay start...\n");

    uint32_t rd_delay;
    uint8_t finded = 0;
    dqs_delay.delay_max = PSRAM_CTRL_BASE->REG_DLL_DELAY.bit.RDLVL_DELAY;
    dqs_delay.delay_min = PSRAM_CTRL_BASE->REG_DLL_DELAY.bit.RDLVL_DELAY;

    for(rd_delay=dqs_delay.delay_max; rd_delay <= 0xFF; rd_delay++){
        PSRAM_LOG("[SEARCH] READ DELAY: 0x%x", rd_delay);
        PSRAM_CTRL_BASE->REG_DLL_DELAY.bit.RDLVL_DELAY = rd_delay;
        PSRAM_CTRL_BASE->REG_DLL_RESYNC.bit.DLL_RESYNC = 1;
        ret = psram_read_data_check();
        if (0 != ret){
            PSRAM_LOG("data compare FAILED! ,max_rd_delay=%d\n", rd_delay);
            // 连续的成功后失败需要退出当前循环
            if (finded == 1){
                dqs_delay.delay_max = (rd_delay - 1);
                break;
            }
        } else{
            // 遇到第一个成功的位置，记录并且finded置位
            PSRAM_LOG("data compare SUCCESSED! ,delay=%d\n", rd_delay);
            if (finded == 0){
                dqs_delay.delay_min = rd_delay;
                finded = 1;
            }
        }
        if (rd_delay == 0xB4){
            PSRAM_LOG("data compare shut! ,max_rd_delay=%d\n", rd_delay);
            if (finded == 1){
                dqs_delay.delay_max = rd_delay;
            }
            break;
        }
    }

    for(rd_delay=dqs_delay.delay_min; rd_delay >= 0; rd_delay--){
        PSRAM_LOG("[SEARCH] READ DELAY: 0x%x", rd_delay);
        PSRAM_CTRL_BASE->REG_DLL_DELAY.bit.RDLVL_DELAY = rd_delay;
        PSRAM_CTRL_BASE->REG_DLL_RESYNC.bit.DLL_RESYNC = 1;
        ret = psram_read_data_check();
        if (0 != ret){
            PSRAM_LOG("data compare FAILED! ,min_rd_delay=%d\n", rd_delay);
            if (finded == 1){
                dqs_delay.delay_min = (rd_delay + 1);
                break;
            }
        } else{
            PSRAM_LOG("data compare SUCCESSED! ,delay=%d\n", rd_delay);
            if (finded == 0){
                dqs_delay.delay_max = rd_delay;
                finded = 1;
            }
        }
        if (rd_delay == 0){
            PSRAM_LOG("data compare shut! ,min_rd_delay=%d\n", rd_delay);
            if (finded == 1){
                dqs_delay.delay_min = rd_delay;
            }
            break;
        }
    }

    dqs_delay.delay_gap = (dqs_delay.delay_max - dqs_delay.delay_min)/2 + dqs_delay.delay_min;
    PSRAM_LOG("search finished, delay_gap=%d\n", dqs_delay.delay_gap);

    PSRAM_CTRL_BASE->REG_DLL_DELAY.bit.RDLVL_DELAY = dqs_delay.delay_gap;
    PSRAM_CTRL_BASE->REG_DLL_RESYNC.bit.DLL_RESYNC = 1;
}

static void search_read_dqs_delay_again(){
    unsigned int ret=0;
    DqsDelay_FmtDef dqs_delay = {0};
    PSRAM_LOG("search_read_delay, check read delay start...\n");

    uint32_t rd_delay;
    uint8_t finded = 0;
    dqs_delay.delay_max = PSRAM_CTRL_BASE->REG_DLL_DELAY.bit.RDLVL_DELAY;
    dqs_delay.delay_min = PSRAM_CTRL_BASE->REG_DLL_DELAY.bit.RDLVL_DELAY;

    for(rd_delay=dqs_delay.delay_max; rd_delay <= 0xFF; rd_delay++){
        PSRAM_LOG("[SEARCH] READ DELAY: 0x%x", rd_delay);
        PSRAM_CTRL_BASE->REG_DLL_DELAY.bit.RDLVL_DELAY = rd_delay;
        PSRAM_CTRL_BASE->REG_DLL_RESYNC.bit.DLL_RESYNC = 1;
        ret = psram_write_data_check();
        if (0 != ret){
            PSRAM_LOG("data compare FAILED! ,max_rd_delay=%d\n", rd_delay);
            // 连续的成功后失败需要退出当前循环
            if (finded == 1){
                dqs_delay.delay_max = (rd_delay - 1);
                break;
            }
        } else{
            // 遇到第一个成功的位置，记录并且finded置位
            PSRAM_LOG("data compare SUCCESSED! ,delay=%d\n", rd_delay);
            if (finded == 0){
                dqs_delay.delay_min = rd_delay;
                finded = 1;
            }
        }
        if (rd_delay == 0xB4){
            PSRAM_LOG("data compare shut! ,max_rd_delay=%d\n", rd_delay);
            if (finded == 1){
                dqs_delay.delay_max = rd_delay;
            }
            break;
        }
    }

    for(rd_delay=dqs_delay.delay_min; rd_delay >= 0; rd_delay--){
        PSRAM_LOG("[SEARCH] READ DELAY: 0x%x", rd_delay);
        PSRAM_CTRL_BASE->REG_DLL_DELAY.bit.RDLVL_DELAY = rd_delay;
        PSRAM_CTRL_BASE->REG_DLL_RESYNC.bit.DLL_RESYNC = 1;
        ret = psram_write_data_check();
        if (0 != ret){
            PSRAM_LOG("data compare FAILED! ,min_rd_delay=%d\n", rd_delay);
            if (finded == 1){
                dqs_delay.delay_min = (rd_delay + 1);
                break;
            }
        } else{
            PSRAM_LOG("data compare SUCCESSED! ,delay=%d\n", rd_delay);
            if (finded == 0){
                dqs_delay.delay_max = rd_delay;
                finded = 1;
            }
        }
        if (rd_delay == 0){
            PSRAM_LOG("data compare shut! ,min_rd_delay=%d\n", rd_delay);
            if (finded == 1){
                dqs_delay.delay_min = rd_delay;
            }
            break;
        }
    }

    dqs_delay.delay_gap = (dqs_delay.delay_max - dqs_delay.delay_min)/2 + dqs_delay.delay_min;
    PSRAM_LOG("search finished, delay_gap=%d\n", dqs_delay.delay_gap);

    PSRAM_CTRL_BASE->REG_DLL_DELAY.bit.RDLVL_DELAY = dqs_delay.delay_gap;
    PSRAM_CTRL_BASE->REG_DLL_RESYNC.bit.DLL_RESYNC = 1;
}


static void check_reg(uint32_t* address){
    unsigned int rdata = 0x0;

    rdata = *address;
    PSRAM_LOG("0x%x reg: 0x%x",address, rdata);
}

void PSRAM_Initialize(void){
    // TODO
    // Clock reset

    // enable PSRAM clock
    SYSTEM_CTRL_BASE->REG_NVM_CLK_CFG.bit.ENA_PSRAM_CLK = 0x1;
    PSRAM_IO_BASE->REG_PSRAMIO_MISC_CFG.bit.PSRAMIO_VREF_CFG = 63;
    PSRAM_IO_BASE->REG_PSRAMIO_MISC_CFG.bit.PSRAMIO_CLK_DRV_CFG = 0x2;

    // reset CP & NPU
    if(SYSTEM_CTRL_BASE->REG_SW_RESET_CP.all){ // = 1 CP reset
        // pass
    } else{
        SYSTEM_CTRL_BASE->REG_SW_RESET_CP.all = SYSCTRL_RESET_CP;
    }

    if(SYSTEM_CTRL_BASE->REG_SW_RESET_NPU.all){ // = 1 NPU reset
        // pass
    } else{
        SYSTEM_CTRL_BASE->REG_SW_RESET_NPU.all = SYSCTRL_RESET_NPU;
    }

#if PSRAM_ODT_ENA
    { // ODT
        PSRAM_IO_BASE->REG_PSRAMIO_DQ0_CFG.bit.PSRAMIO_DQ0_DRV_CFG = 0x7;
        PSRAM_IO_BASE->REG_PSRAMIO_DQ0_CFG.bit.PSRAMIO_DQ0_ODT_CFG = 0x0;
        PSRAM_IO_BASE->REG_PSRAMIO_DQ0_CFG.bit.PSRAMIO_DQ0_NCALIB = 0x7;
        PSRAM_IO_BASE->REG_PSRAMIO_DQ0_CFG.bit.PSRAMIO_DQ0_ODT_EN_DN_VAL = 0x1;

        PSRAM_IO_BASE->REG_PSRAMIO_DQ1_CFG.bit.PSRAMIO_DQ1_DRV_CFG = 0x7;
        PSRAM_IO_BASE->REG_PSRAMIO_DQ1_CFG.bit.PSRAMIO_DQ1_ODT_CFG = 0x0;
        PSRAM_IO_BASE->REG_PSRAMIO_DQ1_CFG.bit.PSRAMIO_DQ1_NCALIB = 0x7;
        PSRAM_IO_BASE->REG_PSRAMIO_DQ1_CFG.bit.PSRAMIO_DQ1_ODT_EN_DN_VAL = 0x1;

        PSRAM_IO_BASE->REG_PSRAMIO_DQ2_CFG.bit.PSRAMIO_DQ2_DRV_CFG = 0x7;
        PSRAM_IO_BASE->REG_PSRAMIO_DQ2_CFG.bit.PSRAMIO_DQ2_ODT_CFG = 0x0;
        PSRAM_IO_BASE->REG_PSRAMIO_DQ2_CFG.bit.PSRAMIO_DQ2_NCALIB = 0x7;
        PSRAM_IO_BASE->REG_PSRAMIO_DQ2_CFG.bit.PSRAMIO_DQ2_ODT_EN_DN_VAL = 0x1;

        PSRAM_IO_BASE->REG_PSRAMIO_DQ3_CFG.bit.PSRAMIO_DQ3_DRV_CFG = 0x7;
        PSRAM_IO_BASE->REG_PSRAMIO_DQ3_CFG.bit.PSRAMIO_DQ3_ODT_CFG = 0x0;
        PSRAM_IO_BASE->REG_PSRAMIO_DQ3_CFG.bit.PSRAMIO_DQ3_NCALIB = 0x7;
        PSRAM_IO_BASE->REG_PSRAMIO_DQ3_CFG.bit.PSRAMIO_DQ3_ODT_EN_DN_VAL = 0x1;

        PSRAM_IO_BASE->REG_PSRAMIO_DQ4_CFG.bit.PSRAMIO_DQ4_DRV_CFG = 0x7;
        PSRAM_IO_BASE->REG_PSRAMIO_DQ4_CFG.bit.PSRAMIO_DQ4_ODT_CFG = 0x0;
        PSRAM_IO_BASE->REG_PSRAMIO_DQ4_CFG.bit.PSRAMIO_DQ4_NCALIB = 0x7;
        PSRAM_IO_BASE->REG_PSRAMIO_DQ4_CFG.bit.PSRAMIO_DQ4_ODT_EN_DN_VAL = 0x1;

        PSRAM_IO_BASE->REG_PSRAMIO_DQ5_CFG.bit.PSRAMIO_DQ5_DRV_CFG = 0x7;
        PSRAM_IO_BASE->REG_PSRAMIO_DQ5_CFG.bit.PSRAMIO_DQ5_ODT_CFG = 0x0;
        PSRAM_IO_BASE->REG_PSRAMIO_DQ5_CFG.bit.PSRAMIO_DQ5_NCALIB = 0x7;
        PSRAM_IO_BASE->REG_PSRAMIO_DQ5_CFG.bit.PSRAMIO_DQ5_ODT_EN_DN_VAL = 0x1;

        PSRAM_IO_BASE->REG_PSRAMIO_DQ6_CFG.bit.PSRAMIO_DQ6_DRV_CFG = 0x7;
        PSRAM_IO_BASE->REG_PSRAMIO_DQ6_CFG.bit.PSRAMIO_DQ6_ODT_CFG = 0x0;
        PSRAM_IO_BASE->REG_PSRAMIO_DQ6_CFG.bit.PSRAMIO_DQ6_NCALIB = 0x7;
        PSRAM_IO_BASE->REG_PSRAMIO_DQ6_CFG.bit.PSRAMIO_DQ6_ODT_EN_DN_VAL = 0x1;

        PSRAM_IO_BASE->REG_PSRAMIO_DQ7_CFG.bit.PSRAMIO_DQ7_DRV_CFG = 0x7;
        PSRAM_IO_BASE->REG_PSRAMIO_DQ7_CFG.bit.PSRAMIO_DQ7_ODT_CFG = 0x0;
        PSRAM_IO_BASE->REG_PSRAMIO_DQ7_CFG.bit.PSRAMIO_DQ7_NCALIB = 0x7;
        PSRAM_IO_BASE->REG_PSRAMIO_DQ7_CFG.bit.PSRAMIO_DQ7_ODT_EN_DN_VAL = 0x1;

        PSRAM_IO_BASE->REG_PSRAMIO_DQS_CFG.bit.PSRAMIO_DQS_DRV_CFG = 0x7;
        PSRAM_IO_BASE->REG_PSRAMIO_DQS_CFG.bit.PSRAMIO_DQS_ODT_CFG = 0x0;
        PSRAM_IO_BASE->REG_PSRAMIO_DQS_CFG.bit.PSRAMIO_DQS_NCALIB = 0x7;
        PSRAM_IO_BASE->REG_PSRAMIO_DQS_CFG.bit.PSRAMIO_DQS_ODT_EN_DN_VAL = 0x1;
    }
#endif

    // read sync
    check_reg((uint32_t*)&PSRAM_CTRL_BASE->REG_MEM_CMD.all);

    PSRAM_CTRL_BASE->REG_DDR_TIM_CONFIG.bit.TCSP_CFG = CONTROLLER_TIME_CONFIG_TCSP;

    // set DLL_MASTER_CTRL (0x10)
    PSRAM_CTRL_BASE->REG_DDR_MASTER_CTRL.bit.DLL_START_POINT = 0x4;
    PSRAM_CTRL_BASE->REG_DDR_MASTER_CTRL.bit.PHASE_DETECT_SEL = 0x7;

    // DLL choose bypass mode or none bypass mode
    PSRAM_CTRL_BASE->REG_DDR_MASTER_CTRL.bit.DLL_BYPASS = 0x0;

    check_reg((uint32_t*)&PSRAM_CTRL_BASE->REG_DDR_MASTER_CTRL.all);

    // reset DLL (0x24)
    PSRAM_CTRL_BASE->REG_DLL_RSTN.bit.DLL_RST_N = 0x0;
    SYSTEM_CTRL_BASE->REG_SW_RESET2.bit.PSRAM_CTRL_RESET = 0x1;

    // check DLL lock done (0x20)
    {
        uint32_t rdata = 0x0;
        uint32_t max_loop_times = 100000;
        do {
            rdata = PSRAM_CTRL_BASE->REG_LOCK_DONE.bit.LOCK_DONE;
        }while ((!(rdata & 0x1)) && (max_loop_times--));

        if (rdata == 0){
            PSRAM_LOGE("DLL line lock done failed");
        } else {
            PSRAM_LOG("DLL line lock Done\n");
        }
    }

    // set priority (0x40)
    PSRAM_CTRL_BASE->REG_PRIO_CTRL.all = CONTROLLER_PRIO_SETTING;
    check_reg((uint32_t*)&PSRAM_CTRL_BASE->REG_PRIO_CTRL.all);

    // check DLL_OBS_REG0 register (0x18)
    check_reg((uint32_t*)&PSRAM_CTRL_BASE->REG_DLL_OBS_REG0.all);

    // PSRAM max CE# low width (0x4)
    // tcem * clock < 4us
    // tcph = 6 (200mhz)
    //      = 4 (133mhz)
    PSRAM_CTRL_BASE->REG_DDR_TIM_CONFIG.all = ((CONTROLLER_TIME_CONFIG_TCSP << 12) | (CONTROLLER_TIME_CONFIG_TCEM << 8) | (CONTROLLER_TIME_CONFIG_TCPH));
    check_reg((uint32_t*)&PSRAM_CTRL_BASE->REG_DDR_TIM_CONFIG.all);

    // set dll
    // set 1 means: use delay cell
    // set 0 means: percentage
    PSRAM_CTRL_BASE->REG_DDR_MASTER_CTRL.bit.DLL_BYPASS = PSRAM_DLL_BYPASS;

    // set DQS delay cell (0x14)
    // 0x18_(dll_lock_value) * 0x14_(delay)/256 = 0x1c_(delay)
    {
        uint32_t div = PSRAM_CTRL_BASE->REG_DLL_OBS_REG0.bit.HALF_CLOCK_MODE;
        if (div){
            div = 2;
        } else{
            div = 4;
        }
#if DEFAULT_RD_DELAY == 0
        PSRAM_CTRL_BASE->REG_DLL_DELAY.bit.RDLVL_DELAY = PSRAM_CTRL_BASE->REG_DLL_OBS_REG1.bit.DECODER_OUT_RD/div;
#else
        PSRAM_CTRL_BASE->REG_DLL_DELAY.bit.RDLVL_DELAY = PSRAM_DQS_RD_DELAY;
#endif
#if DEFAULT_WT_DELAY == 0
        PSRAM_CTRL_BASE->REG_DLL_DELAY.bit.WRLVL_DELAY = PSRAM_CTRL_BASE->REG_DLL_OBS_REG1.bit.DECODER_OUT_WR/div;
#else
        PSRAM_CTRL_BASE->REG_DLL_DELAY.bit.WRLVL_DELAY = PSRAM_DQS_WR_DELAY;
#endif
        check_reg((uint32_t*)&PSRAM_CTRL_BASE->REG_DLL_DELAY.all);
    }

    // resync DLL (0x28)
    PSRAM_CTRL_BASE->REG_DLL_RESYNC.bit.DLL_RESYNC = 0x1;

    // chip initialize (0x30)
    PSRAM_CTRL_BASE->REG_CHIP_CONFIG.bit.CHIP_INIT = 0x1;
    check_reg((uint32_t*)&PSRAM_CTRL_BASE->REG_CHIP_CONFIG.all);

    // set DDR wait cycle, before send reset command to PSRAM
    // PSRAM reset w_wait must be zero

    PSRAM_CTRL_BASE->REG_DDR_WAIT.bit.MEM_R_WAIT = CONTROLLER_MEM_R_WAIT_CYCLES;
    // set write wait to zero
    // 这里使用默认参数，是因为这时PSRAM还没有配置成功
    PSRAM_CTRL_BASE->REG_DDR_WAIT.bit.MEM_W_WAIT = 0x0;   // default parameter
    PSRAM_CTRL_BASE->REG_DDR_WAIT.bit.MODE_R_WAIT = 0x4;

    // reset PSRAM
    {
        // command to reset PSRAM (0x0)
        PSRAM_CTRL_BASE->REG_MEM_CMD.bit.WRITE_REG_CMD = 0xff;

        // send PSRAM reset
        PSRAM_CTRL_BASE->REG_MR0.all = 0x0;
        // wait
        {
            int i, j;
            for(i = 0; i < 100; i++){
                for(j = 0; j < 1000; j++);
            }
        }

        // reset command
        PSRAM_CTRL_BASE->REG_MEM_CMD.bit.WRITE_REG_CMD = 0xc0;
    }

    {
        uint32_t i = 100;
        uint32_t rdata = 0;

        // read latency
        while(i--){
            rdata = PSRAM_CTRL_BASE->REG_MR0.all;
            if ((rdata & 0xff) == ((PSRAM_MR0_LT << 5) | (PSRAM_MEM_R_WAIT_CYCLES << 2) | PSRAM_MR0_DRIVE_STR)){
                break;
            }
            PSRAM_CTRL_BASE->REG_MR0.all = ((PSRAM_MR0_LT << 5) | (PSRAM_MEM_R_WAIT_CYCLES << 2) | PSRAM_MR0_DRIVE_STR);

            // adjust to current parameter
            PSRAM_CTRL_BASE->REG_DDR_WAIT.bit.MODE_R_WAIT = CONTROLLER_MODE_R_WAIT_CYCLES;
        }

        if (i == 0){
            PSRAM_LOGE("PSRAM MR0 configure error");
        }

        // wrap 1K
        i = 100;
        while(i--){
            rdata = PSRAM_CTRL_BASE->REG_MR8.all;
            if ((rdata & 0xff) == ((PSRAM_MR8_BT << 2) | PSRAM_MR8_BL)){
                break;
            }
            PSRAM_CTRL_BASE->REG_MR8.all = ((PSRAM_MR8_BT << 2) | PSRAM_MR8_BL);
        }

        // write latency
        i = 100;
        while(i--){
            rdata = PSRAM_CTRL_BASE->REG_MR4.all;
            if ((rdata & 0xff) == ((PSRAM_MEM_W_WAIT_CYCLES << 5) | (PSRAM_MR4_RF << 3) | PSRAM_MR4_PASR)){
                PSRAM_CTRL_BASE->REG_DDR_WAIT.bit.MEM_W_WAIT = CONTROLLER_MEM_W_WAIT_CYCLES;
                break;
            }
            PSRAM_CTRL_BASE->REG_MR4.all = ((PSRAM_MEM_W_WAIT_CYCLES << 5) | (PSRAM_MR4_RF << 3) | PSRAM_MR4_PASR);
        }

        check_reg((uint32_t*)&PSRAM_CTRL_BASE->REG_MR0.all);
        check_reg((uint32_t*)&PSRAM_CTRL_BASE->REG_MR1.all);
        check_reg((uint32_t*)&PSRAM_CTRL_BASE->REG_MR2.all);
        check_reg((uint32_t*)&PSRAM_CTRL_BASE->REG_MR3.all);
        check_reg((uint32_t*)&PSRAM_CTRL_BASE->REG_MR4.all);
        check_reg((uint32_t*)&PSRAM_CTRL_BASE->REG_MR6.all);
        check_reg((uint32_t*)&PSRAM_CTRL_BASE->REG_MR8.all);

    }
#if PSRAM_WT_DQS_SEARCH
    /*
     * Using 24Mhz write data to PSRAM
     * */
    clock_src_name_t src;
    CRM_GetIpConfig(&src, NULL, CRM_Psram);
    // Check to 24Mhz
    CRM_SetIpSrc(CRM_Psram, CRM_IpSrcXtal);
    PSRAM_LOG("Current Frequence: %d", CRM_GetIpFreq(CRM_Psram));

    // Write data to PSRAM in low frequence
    {
        uint32_t *ptr_psram = (uint32_t *)PSRAM_BASE;

        psram_data_init();
        __builtin_memcpy(ptr_psram, psram_src_array,       sizeof(psram_src_array));
    }

    // Restore psram clock
    CRM_SetIpSrc(CRM_Psram, src);
    PSRAM_LOG("Restore Frequence: %d", CRM_GetIpFreq(CRM_Psram));
#endif

    // search read DQS delay
#if PSRAM_RD_DQS_SEARCH
    search_read_dqs_delay();
#endif

#if PSRAM_WT_DQS_SEARCH
    search_write_dqs_delay();

    search_read_dqs_delay_again();
#endif

#if TEST_MODE == 0
    // enable CP & NPU clock
    SYSTEM_CTRL_BASE->REG_CORE_CLK_CFG.bit.ENA_CP_CORE_CLK = 0x1;
    SYSTEM_CTRL_BASE->REG_CORE_CLK_CFG.bit.ENA_NPU_CLK = 0x1;

    // release CP & NPU
    if(SYSTEM_CTRL_BASE->REG_SW_RESET_CP.all){ // = 1 CP reset
        SYSTEM_CTRL_BASE->REG_SW_RESET_CP.all = SYSCTRL_RELEASE_CP;
    }

    if(SYSTEM_CTRL_BASE->REG_SW_RESET_NPU.all){ // = 1 NPU reset
        SYSTEM_CTRL_BASE->REG_SW_RESET_NPU.all = SYSCTRL_RESET_NPU;
    }

    // run HIFI4
    SYSTEM_CTRL_BASE->REG_CP_CTRL1.bit.CP_HIFI4_RUNSTALL = 0x0;
#endif
}
