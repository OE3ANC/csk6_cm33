/*
 * venus_log.h
 *
 *  Created on: 2020.10.27
 *      Author: USER
 */

#ifndef INCLUDE_DEBUG_VENUS_LOG_H_
#define INCLUDE_DEBUG_VENUS_LOG_H_

#include <stdint.h>
#include <stdio.h>

//#include "tinyprintf.h"

extern void logInit(int dbg, uint32_t baudrate);
extern void logDbg(const char* format, ...);

//#define CLOG(fmt, ...)   tfp_printf(fmt"\n", ##__VA_ARGS__)
#define CLOG(fmt, ...)   logDbg(fmt"\n", ##__VA_ARGS__)

#define CLOG_LEVEL_NONE     0
#define CLOG_LEVEL_ERROR    1
#define CLOG_LEVEL_WARN     2
#define CLOG_LEVEL_INFO     3
#define CLOG_LEVEL_DEBUG    4
#define CLOG_LEVEL_VERBOSE  5

#ifndef CLOG_LEVEL
#define CLOG_LEVEL   CLOG_LEVEL_DEBUG
#endif

#define CLOGE(fmt, ...)     do {if (CLOG_LEVEL >= CLOG_LEVEL_ERROR) CLOG(fmt,##__VA_ARGS__);} while(0)
#define CLOGW(fmt, ...)     do {if (CLOG_LEVEL >= CLOG_LEVEL_WARN) CLOG(fmt,##__VA_ARGS__);} while(0)
#define CLOGI(fmt, ...)     do {if (CLOG_LEVEL >= CLOG_LEVEL_INFO) CLOG(fmt,##__VA_ARGS__);} while(0)
#define CLOGD(fmt, ...)     do {if (CLOG_LEVEL >= CLOG_LEVEL_DEBUG) CLOG(fmt,##__VA_ARGS__);} while(0)
#define CLOGV(fmt, ...)     do {if (CLOG_LEVEL >= CLOG_LEVEL_VERBOSE) CLOG(fmt,##__VA_ARGS__);} while(0)



#endif /* INCLUDE_DEBUG_VENUS_LOG_H_ */
