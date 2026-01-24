/*
 * calibration.c
 *
 *  Created on: Mar 5, 2025
 *      Author: zuidec
 */

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "axis_control.h"
//#include "bitutils.h"


#define P_MAX_OFFSET    (0)
#define P_MIN_OFFSET    P_MAX_OFFSET + (sizeof(int16_t))
#define P_RNG_OFFSET    P_MIN_OFFSET + (sizeof(int16_t))
#define L_MAX_OFFSET    P_RNG_OFFSET + (sizeof(int16_t))
#define L_MIN_OFFSET    L_MAX_OFFSET + (sizeof(int16_t))
#define L_RNG_OFFSET    L_MIN_OFFSET + (sizeof(int16_t))
#define ZERO_OFFSET     L_RNG_OFFSET + (sizeof(int16_t))
#define S_POS_OFFSET    ZERO_OFFSET  + (sizeof(int16_t))
#define S_NEG_OFFSET    S_POS_OFFSET + (sizeof(float))
#define ALPHA_OFFSET    S_NEG_OFFSET + (sizeof(float))
#define IS_IZ_OFFSET    ALPHA_OFFSET + (sizeof(float))
#define IS_SYM_OFFSET   IS_IZ_OFFSET + (sizeof(uint8_t))


/******************************************************************************
 *  Variables
 *****************************************************************************/

axis_calibration_factors_t cal_factor_template = {
    .physical_max = DEFAULT_LOGICAL_MAX,
    .physical_min = DEFAULT_LOGICAL_MIN,
    .physical_range = DEFAULT_LOGICAL_MAX - DEFAULT_LOGICAL_MIN,
    .logical_max = DEFAULT_LOGICAL_MAX,
    .logical_min = DEFAULT_LOGICAL_MIN,
    .logical_range = DEFAULT_LOGICAL_MAX - DEFAULT_LOGICAL_MIN,
    .zero = 0,
    .step_size_pos = 1.0f,
    .step_size_neg = -1.0f,
    .alpha = 0.3f,
    .is_inferred_zero = 1,
    .is_symmetric = 1
};

/******************************************************************************
 * Static Functions 
 *****************************************************************************/

static int32_t absval_int(int32_t val)    {
    if(val < 0) {
        // we'll ignore the possibility of INT_MIN for this implementation
        return val * -1;
    }
    else    {
        return val;
    }
}

/* 
// Commented out to stop gcc warning about unused functions
static float absval_float(float val)    {
    if(val < 0.0f) {
        return val * -1.0f;
    }
    else    {
        return val;
    }
}
*/
/******************************************************************************
 * Functions 
 *****************************************************************************/

int16_t apply_calibration(axis_calibration_factors_t *cal, int16_t input) {

    // This only works if the inputs we are getting are positive, and
    // is what is expected when reading from an ADC

	int16_t value = 0;
    /*if(input < cal->physical_min)	{
    	input = cal->physical_min;
    }
    else if(input > cal->physical_max)	{
    	input = cal->physical_max;
    }*/
	if(input == cal->zero) {
		value = 0;
	}
    else if(1 == cal->is_symmetric) {
        if(input > cal->zero)   {
            value = input - cal->zero;
            value = (int16_t) (cal->step_size_pos * (float) value);
        }
        else    {
            value = cal->zero - input;
            value = (int16_t) (cal->step_size_neg * (float) value);
        }
	} 
    else {
        if(input < cal->zero)   {
            value = 0;
        }
        else    {
            value = input - cal->zero;
        }
		value = (int16_t) (cal->step_size_pos * (float) value);
	}

    if (value >= cal->logical_max) {
		value = cal->logical_max;
	} 
    else if (value <= cal->logical_min) {
		value = cal->logical_min;
	}

	return value;
}

void set_calibration(axis_calibration_factors_t *cal, uint8_t *data,
        uint8_t data_size) {
	if (data_size != sizeof(axis_calibration_factors_t)) {
		return;
	}
    /*
	cal->physical_max = u8_to_i16(data[0], data[1]);
	cal->physical_min = u8_to_i16(data[2], data[3]);
	cal->physical_range = u8_to_i16(data[4], data[5]);
	cal->logical_max = DEFAULT_LOGICAL_MAX; //u8_to_i16(data[6], data[7]);
	cal->logical_min = DEFAULT_LOGICAL_MIN; //u8_to_i16(data[8], data[9]);
	cal->logical_range = cal->logical_max - cal->logical_min; //u8_to_i16(data[10], data[11]);
	cal->zero = u8_to_i16(data[12], data[13]);
	cal->step_size_pos = u8_to_float(&data[14]);
	cal->step_size_neg= u8_to_float(&data[18]);
    cal->alpha = u8_to_float(&data[22]);
	cal->is_inferred_zero = data[26];
	cal->is_symmetric = data[27];
    */
    
    memcpy(&cal->physical_max,&data[P_MAX_OFFSET], sizeof(int16_t));
    memcpy(&cal->physical_min,&data[P_MIN_OFFSET], sizeof(int16_t));
    memcpy(&cal->physical_range,&data[P_RNG_OFFSET], sizeof(int16_t));
    memcpy(&cal->logical_max,&data[L_MAX_OFFSET], sizeof(int16_t));
    memcpy(&cal->logical_min,&data[L_MIN_OFFSET], sizeof(int16_t));
    memcpy(&cal->logical_range,&data[L_RNG_OFFSET], sizeof(int16_t));
    memcpy(&cal->zero,&data[ZERO_OFFSET], sizeof(int16_t));
    memcpy(&cal->step_size_pos,&data[S_POS_OFFSET], sizeof(float));
    memcpy(&cal->step_size_neg,&data[S_NEG_OFFSET], sizeof(float));
    memcpy(&cal->alpha,&data[ALPHA_OFFSET], sizeof(float));
    memcpy(&cal->is_inferred_zero,&data[IS_IZ_OFFSET], sizeof(uint8_t));
    memcpy(&cal->is_symmetric,&data[IS_SYM_OFFSET], sizeof(uint8_t));

}

void get_calibration(axis_calibration_factors_t *cal, uint8_t *data,
        uint8_t data_size) {
	if (data_size != sizeof(axis_calibration_factors_t)) {
		return;
	}
    /*
	i16_to_u8(cal->physical_max, &data[0]);
	i16_to_u8(cal->physical_min, &data[2]);
	i16_to_u8(cal->physical_range, &data[4]);
	i16_to_u8(cal->logical_max, &data[6]);
	i16_to_u8(cal->logical_min, &data[8]);
	i16_to_u8(cal->logical_range, &data[10]);
	i16_to_u8(cal->zero, &data[12]);
	float_to_u8(cal->step_size_pos, &data[14]);
	float_to_u8(cal->step_size_neg, &data[18]);
    float_to_u8(cal->alpha, &data[22]);
	cal->is_inferred_zero = data[26];
	cal->is_symmetric = data[27];
    */

    memcpy(&data[P_MAX_OFFSET],&cal->physical_max, sizeof(int16_t));
    memcpy(&data[P_MIN_OFFSET],&cal->physical_min, sizeof(int16_t));
    memcpy(&data[P_RNG_OFFSET],&cal->physical_range, sizeof(int16_t));
    memcpy(&data[L_MAX_OFFSET],&cal->logical_max, sizeof(int16_t));
    memcpy(&data[L_MIN_OFFSET],&cal->logical_min, sizeof(int16_t));
    memcpy(&data[L_RNG_OFFSET],&cal->logical_range, sizeof(int16_t));
    memcpy(&data[ZERO_OFFSET],&cal->zero, sizeof(int16_t));
    memcpy(&data[S_POS_OFFSET],&cal->step_size_pos, sizeof(float));
    memcpy(&data[S_NEG_OFFSET],&cal->step_size_neg, sizeof(float));
    memcpy(&data[ALPHA_OFFSET],&cal->alpha, sizeof(float));
    memcpy(&data[IS_IZ_OFFSET],&cal->is_inferred_zero, sizeof(uint8_t));
    memcpy(&data[IS_SYM_OFFSET],&cal->is_symmetric, sizeof(uint8_t));
}

void clear_calibration(axis_calibration_factors_t *cal) {
	cal->physical_max = 0;
	cal->physical_min = 0;
	cal->physical_range = 0;
	cal->step_size_pos = 0.0f;
	cal->step_size_neg = 0.0f;
}

void recalculate_calibration(axis_calibration_factors_t *cal) {

    /*
     *  First branch is whether axis has two directions. is_symmetric does not
     *  mean the axis is truly symmetric, only that it has a positive and 
     *  negative deflection.
     *
     *  Then set the physical max, min, and range accordingly. If no adjustments 
     *  are needed to the zero point, or the axis is unidirectional, then set
     *  is_inferred_zero to 1 (true) and set the zero here as well.
     */

    if (cal->is_symmetric) {
        if (cal->physical_max > cal->physical_min) {
			cal->physical_range = cal->physical_max - cal->physical_min;
            if(1 == cal->is_inferred_zero)  {
                cal->zero = (cal->physical_max + cal->physical_min) / 2;
            }
		} 
        else {
            int16_t temp = cal->physical_max;
            cal->physical_max = cal->physical_min;
            cal->physical_min = temp;
			cal->physical_range = cal->physical_max - cal->physical_min;
            if(1 == cal->is_inferred_zero)  {
                cal->zero = (cal->physical_max + cal->physical_min) / 2;
            }
		}
	} 
    else {


        if (cal->physical_max < cal->physical_min) {
            int16_t temp = cal->physical_max;
            cal->physical_max = cal->physical_min;
            cal->physical_min = temp;
		}
        cal->physical_range = cal->physical_max - cal->physical_min;
        cal->zero = cal->physical_min;
        cal->is_inferred_zero = 1;
	}

    /*
     *  Calculate step sizes. Inferred zero means the axis is either truly 
     *  symmetric or it is unidirectional
     */

    if(1 == cal->is_inferred_zero)  {
        if (cal->physical_range != 0) { // exlicitly check to avoid divide by 0
            cal->step_size_pos = (float) cal->logical_range
                / (float) cal->physical_range;
            cal->step_size_neg = -1.0f * cal->step_size_pos;
        }
        else {
            cal->step_size_pos = 1.0f;
            cal->step_size_neg = -1.0f;
        }
    }
    else    {
        if(cal->zero != cal->physical_max && cal->zero != cal->physical_min)    {
            cal->step_size_pos = ((float)cal->logical_range / 2.0f) 
                / (float)(cal->physical_max - cal->zero);
            cal->step_size_neg = -1.0f * (((float)cal->logical_range / 2.0f)
                / (float)(cal->zero - cal->physical_max));
        }
        else    {
            cal->step_size_pos = ((float)cal->logical_range / 2.0f) 
                / ((float)(cal->physical_max - cal->zero) + 0.001f);
            cal->step_size_neg = -1.0f * (((float)cal->logical_range / 2.0f)
                / ((float)(cal->zero - cal->physical_max) + 0.001f));
            
        }
    }
} 

bool verify_calibration(axis_calibration_factors_t *cal)    {
    bool ret = false;

    if(cal->physical_max < cal->physical_min)   {
        ret = false;
    }
    else if(cal->logical_max < cal->logical_min)    {
        ret = false;
    }
    else if(absval_int(cal->physical_max) + absval_int(cal->physical_min)  != cal->physical_range) {
        ret = false;
    }
    else if(cal->alpha > 1.0f || cal->alpha < 0.0f)  {
        ret = false;
    }
    else if(absval_int(cal->logical_max) + absval_int(cal->logical_min) != cal->logical_range)  {
        ret = false;
    }
    else if(cal->is_inferred_zero > 1) {
        ret = false;
    }
    else if(cal->is_symmetric > 1){
        ret = false;
    }
    else if(cal->zero > cal->physical_max || cal->zero < cal->physical_min)  {
        ret = false;
    }
    else if(cal->step_size_pos <= 0.0f) {
        ret = false;
    }
    else if(cal->step_size_neg >= 0.0f) {
        ret = false;
    }
    else    {
        ret = true;
    }

    return ret;
}

