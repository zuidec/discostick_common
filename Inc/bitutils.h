/*
 * bitutils.h
 *
 *  Created on: Mar 5, 2025
 *      Author: zuidec
 */

#ifndef INC_BITUTILS_H_
#define INC_BITUTILS_H_

#include "stdint.h"

typedef union float_st	{
	float value;
	uint8_t bytes[4];
}float_st;

int16_t u8_to_i16(const uint8_t* num);
uint32_t u8_to_u32(const uint8_t* num);
float u8_to_float(const uint8_t* num);
uint8_t i16_high_to_u8(int16_t num);
uint8_t i16_low_to_u8(int16_t num);
void i16_to_u8(int16_t num, uint8_t* data);
void float_to_u8(float num, uint8_t* data);
void u32_to_u8(uint32_t num, uint8_t* data);

#endif /* INC_BITUTILS_H_ */
