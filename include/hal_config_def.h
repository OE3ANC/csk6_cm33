#ifndef __HAL_DEF_CONFIG_H__

#ifdef HAL_CUSTOM_CONFIG_FILE
#include HAL_CUSTOM_CONFIG_FILE
#endif

#ifndef SDHC_CLK_FREQ
#define SDHC_CLK_FREQ 25
#else
#if (SDHC_CLK_FREQ != 25) && (SDHC_CLK_FREQ != 50)
#error SDHC_CLK_FREQ must be 25Mhz or 50Mhz
#endif
#endif

#ifndef SDHC_SDMA_LINE_BOUND
#define SDHC_SDMA_LINE_BOUND 4
#endif

#endif
