/*
 * errorhandler.h
 *
 *  Created on: Dec 10, 2025
 *      Author: zuidec
 */

#ifndef INC_ERRORHANDLER_H_
#define INC_ERRORHANDLER_H_

#ifdef __cplusplus
	extern "C" {
#endif

#include "stdint.h"

#define HARDFAULT_HANDLING_ASM(_x)               \
  __asm__ volatile(                                \
      "tst lr, #4 \n"                            \
      "ite eq \n"                                \
      "mrseq r0, msp \n"                         \
      "mrsne r0, psp \n"                         \
      "b _Hardfault_Handler \n"                  \
                                                 )

typedef struct __attribute__((packed)) ContextStateFrame {
	uint32_t r0;
	uint32_t r1;
	uint32_t r2;
	uint32_t r3;
	uint32_t r12;
	uint32_t lr;
	uint32_t return_address;
	uint32_t xpsr;
} sContextStateFrame;

typedef enum log_level_t    {
    LOG_LEVEL_WARN,
    LOG_LEVEL_ERROR,
    LOG_LEVEL_DEBUG,
    LOG_LEVEL_INFO
} log_level_t;

void log_error(const char* err,...);
void log_warning(const char* warn,...);
void log_info(const char* info,...);
void log_debug(const char* debug,...); 
void log_trace(const char* trace,...);
void log_printf(const char* fmt,...);
void _Hardfault_Handler(sContextStateFrame *frame);
void _Error_Handler(const char *, int);
const char* get_hal_error_string(uint32_t err);

#define Error_Handler() _Error_Handler(__FILE__, __LINE__)

#ifdef __cplusplus
	}
#endif
#endif /* INC_ERRORHANDLER_H_ */
