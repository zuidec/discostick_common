/*
 * errorhandler.c
 *
 *  Created on: Dec 10, 2025
 *      Author: zuidec
 */

#include <stdio.h>
#include <stdarg.h>
#include <stddef.h>
#include <string.h>
#include "logger.h"
#include "main.h"
#define NANOPRINTF_IMPLEMENTATION
#include "nanoprintf.h"

#define LOG_DEBUG 1
#define LOG_TRACE 1

#define HALT_IF_DEBUGGING()                              \
  do {                                                   \
    if ((*(volatile uint32_t *)0xE000EDF0) & (1 << 0)) { \
      __asm__("bkpt 1");                                 \
    }                                                    \
} while (0)

#ifndef LOG_BUF_SIZE
#define LOG_BUF_SIZE 1024
#endif

static char temp[LOG_BUF_SIZE] = {0};
static const char* hal_error_strings[4] ={ "HAL_ERROR\0","HAL_BUSY\0","HAL_TIMEOUT\0",
                                    "UNKNOWN_ERROR\0"};


void __attribute__((optimize("O0"))) _Error_Handler(const char *file, int line) {
	__disable_irq();
	log_error("Soft fault reached at line %d in file %s\n", line, file);
	__BKPT();
	while (1) {
		__asm__("NOP");
	}
}


void __attribute__((optimize("O0")))_Hardfault_Handler(sContextStateFrame *frame) {
	// If and only if a debugger is attached, execute a breakpoint
	// instruction so we can take a look at what triggered the fault
	printf("\n\033[38;5;197m[F]\tHardfault reached, frame dump:\n");
	printf("\tr0: \t\t0x%010lx\n\tr1: \t\t0x%010lx\n\tr2: \t\t0x%010lx\n\tr3: \t\t0x%010lx\n\tr12: \t\t0x%010lx\n\tlr: \t\t0x%010lx\n\treturn_address: 0x%010lx\n\txpsr: \t\t0x%010lx\n",
	       frame->r0, frame->r1, frame->r2, frame->r3, frame->r12, frame->lr,
	       frame->return_address, frame->xpsr);
	HALT_IF_DEBUGGING();

	while (1) {
		;;
	}
	// Logic for dealing with the exception. Typically:
	//  - log the fault which occurred for postmortem analysis
	//  - If the fault is recoverable,
	//    - clear errors and return back to Thread Mode
	//  - else
	//    - reboot system
}

void log_error(const char* err,...) {
    memset(temp, 0, sizeof(temp));
    va_list args;
    va_start (args, err);
    npf_vsnprintf(temp,sizeof(temp),err,args);
    va_end (args);
    printf("\033[0;31m[E]%s\n",temp);
}

void log_warning(const char* warn,...) {
    memset(temp, 0, sizeof(temp));
    va_list args;
    va_start (args, warn);
    npf_vsnprintf(temp,sizeof(temp),warn,args);
    va_end (args);
    printf("\033[0;33m[W]%s\n",temp);
}

void log_info(const char* info,...) {
    memset(temp, 0, sizeof(temp));
    va_list args;
    va_start (args, info);
    npf_vsnprintf(temp,sizeof(temp),info,args);
    printf("\033[0;36m[I]%s\n",temp);
    //printf(info,args);
   // printf("\n");
    va_end (args);
}

void log_printf(const char* fmt,...)    {
    memset(temp, 0, sizeof(temp));
    va_list args;
    va_start (args, fmt);
    npf_vsnprintf(temp,sizeof(temp),fmt,args);
    va_end (args);
    printf("%s",temp);
}

void log_debug(const char* debug,...) {
#if LOG_DEBUG==1
    memset(temp, 0, sizeof(temp));
    va_list args;
    va_start (args, debug);
    npf_vsnprintf(temp,sizeof(temp),debug,args);
    va_end (args);
    printf("\033[38;5;135m[D]%s\n",temp);
#endif
}

void log_trace(const char* trace,...)   {
#if LOG_TRACE==1
    memset(temp, 0, sizeof(temp));
    va_list args;
    va_start (args, trace);
    npf_vsnprintf(temp,sizeof(temp),trace,args);
    va_end (args);
    printf("\033[38;5;246m[T]%s\n",temp);
#endif
}

const char* get_hal_error_string(uint32_t err)    {
    const char* ret;
    switch(err) {
        case 0x01:
            ret = hal_error_strings[0];
            break;
        case 0x02:
            ret = hal_error_strings[1];
            break;
        case 0x03:
            ret = hal_error_strings[2];
            break;
        default:
            ret = hal_error_strings[3];
            break;
    }
    return ret;
}

