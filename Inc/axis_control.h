/*
 *	axis_control.h
 *	(enter file description here)
 *
 *	Created by zuidec on 01/17/26
 */

#ifndef AXIS_CONTROL_H
#define AXIS_CONTROL_H	// BEGIN AXIS_CONTROL_H

/*
 *	Includes
 */
#include <stdint.h>
#include <stdbool.h>
#include "stepper.h"
#include "input_filter.h"



/*
 *	Defines
 */

#define DEFAULT_LOGICAL_MAX			(1024)
#define DEFAULT_LOGICAL_MIN			(-1024)


/*
 *	Structs, unions, etc...
 */

typedef struct __attribute__((packed)) cyclic_report_t {
	uint8_t buttons;
	int16_t roll;
	int16_t pitch;
    uint8_t padding;
}cyclic_report_t;

typedef struct __attribute__((packed)) pedal_report_t {
	int16_t yaw;
	uint16_t l_brake;
	uint16_t r_brake;
}pedal_report_t;

typedef struct __attribute__((packed)) collective_report_t  {
    uint32_t buttons;
    uint16_t thrust;
} collective_report_t;

typedef union axis_report_t {
    cyclic_report_t cyclic;
    pedal_report_t pedal;
    collective_report_t collective;
} axis_report_t;

typedef struct __attribute__((packed))	{
	int16_t physical_max;
	int16_t physical_min;
	int16_t physical_range;
	int16_t logical_max;
	int16_t logical_min;
	int16_t logical_range;
	int16_t zero;
	float   step_size_pos;
	float   step_size_neg;
    float   alpha;
	uint8_t is_inferred_zero;
    uint8_t is_symmetric;
}axis_calibration_factors_t;

typedef enum axis_t {
    axis_none       = 0x00,
    axis_pitch      = 0x01,
    axis_roll       = 0x02,
    axis_yaw        = 0x03,
    axis_thrust     = 0x04,
    axis_l_brake    = 0x05,
    axis_r_brake    = 0x06
} axis_t;

typedef struct __attribute__((packed)) axis_handle_t    {
    uint8_t axis;
    uint8_t is_connected;
    axis_report_t report; 
    stepper_handle_t motor;
    axis_calibration_factors_t calibration;
    input_filter_t filter;
} axis_handle_t;

/*
 *  Exported variables
 */

extern axis_calibration_factors_t cal_factor_template;

/*
 *	Function prototypes
 */


int16_t apply_calibration(axis_calibration_factors_t* cal, int16_t input);
void set_calibration(axis_calibration_factors_t* cal, uint8_t* data, uint8_t data_size);
void get_calibration(axis_calibration_factors_t* cal, uint8_t* data, uint8_t data_size);
void clear_calibration(axis_calibration_factors_t* cal);
void recalculate_calibration(axis_calibration_factors_t* cal);
void save_calibrations(axis_calibration_factors_t* cal[], uint8_t calibration_count);
bool verify_calibration(axis_calibration_factors_t* cal);


#endif	// END AXIS_CONTROL_H
