/*
 * calibration.h
 *
 *  Created on: Mar 5, 2025
 *      Author: zuidec
 */

#ifndef INC_CALIBRATION_H_
#define INC_CALIBRATION_H_

#include <stdint.h>
#include <stdbool.h>
#include "axis_control.h"

#define DEFAULT_LOGICAL_MAX			(1024)
#define DEFAULT_LOGICAL_MIN			(-1024)


int16_t apply_calibration(axis_calibration_factors_t* cal, int16_t input);
void set_calibration(axis_calibration_factors_t* cal, uint8_t* data, uint8_t data_size);
void get_calibration(axis_calibration_factors_t* cal, uint8_t* data, uint8_t data_size);
void clear_calibration(axis_calibration_factors_t* cal);
void recalculate_calibration(axis_calibration_factors_t* cal);
void load_calibrations(axis_calibration_factors_t* cal[], uint8_t calibration_count);
void save_calibrations(axis_calibration_factors_t* cal[], uint8_t calibration_count);
bool verify_calibration(axis_calibration_factors_t* cal);

#endif /* INC_CALIBRATION_H_ */
