/*
 * bitutils.c
 *
 *  Created on: Mar 5, 2025
 *      Author: zuidec
 */


#include "bitutils.h"

int16_t u8_to_i16(const uint8_t* num)	{
	return (int16_t)(num[1] << 8 | num[0]);
}

uint32_t u8_to_u32(const uint8_t* num)	{
    return (((uint32_t)num[3] << 24) | ((uint32_t)num[2] << 16) | ((uint32_t)num[1] << 8) | ((uint32_t)num[0]<< 0));
}

float u8_to_float(const uint8_t* num)	{
	//float_st temp_num = {.bytes[0] = num[0], .bytes[1] = num[1], .bytes[2] = num[2], .bytes[3] = num[3]};
    //return temp_num.value;
    return (float)(((uint32_t)num[3] << 24) | ((uint32_t)num[2] << 16) | ((uint32_t)num[1] << 8) | ((uint32_t)num[0]<< 0));

}

uint8_t i16_high_to_u8(int16_t num)	{
	return (uint8_t)(num >> 8);
}

uint8_t i16_low_to_u8(int16_t num)	{
	return (uint8_t)(num & 0xFF);
}

void i16_to_u8(int16_t num, uint8_t* data)	{
	//data[0] = i16_high_to_u8(num);
//	data[1] = i16_low_to_u8(num);
    data[0] = (uint8_t)(num & 0xFF);
    data[1] = (uint8_t)(num >> 8);
    
}

void float_to_u8(float num, uint8_t* data)	{
    /*
	float_st temp_num = {.value = num};

	data[0] = temp_num.bytes[0];
	data[1] = temp_num.bytes[1];
	data[2] = temp_num.bytes[2];
	data[3] = temp_num.bytes[3];
    */

    data[0] = (uint8_t)((uint32_t)num & 0xFF);
    data[1] = (uint8_t)(((uint32_t)num >> 8) & 0xFF);
    data[2] = (uint8_t)(((uint32_t)num >> 16) & 0xFF);
    data[3] = (uint8_t)(((uint32_t)num >> 24) & 0xFF);

}

void u32_to_u8(uint32_t num, uint8_t* data)	{
    /*
	union	{
		uint32_t value;
		uint8_t bytes[4];
	}temp_num;

	temp_num.value = num;
	data[0] = temp_num.bytes[0];
	data[1] = temp_num.bytes[1];
	data[2] = temp_num.bytes[2];
	data[3] = temp_num.bytes[3];
*/

    data[0] = (uint8_t)(num & 0xFF);
    data[1] = (uint8_t)((num >> 8) & 0xFF);
    data[2] = (uint8_t)((num >> 16) & 0xFF);
    data[3] = (uint8_t)((num >> 24) & 0xFF);
}
