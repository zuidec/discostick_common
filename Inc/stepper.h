/*
 * stepper.h
 *
 *  Created on: Jan 3, 2025
 *      Author: zuidec
 */

#ifndef INC_STEPPER_H_
#define INC_STEPPER_H_

#include "main.h"
#include <stdint.h>
#include <stdbool.h>

#define MOTOR_TYPE		NEMA_17
#define STEP_ANGLE		(1.80f)
#define STEPS_PER_REV	(200)
#define UINT16_T_MAX				(65536)


typedef enum	{
	MICROSTEP_1 = 200,
	MICROSTEP_2A = 400,
	MICROSTEP_2B = 400,
	MICROSTEP_4 = 800,
	MICROSTEP_8 = 1600,
	MICROSTEP_16 = 3200,
	MICROSTEP_32 = 6400

} stepper_mode_t;

typedef struct	{
	GPIO_TypeDef* en_gpio_port;
	uint32_t en_gpio_pin;
	GPIO_TypeDef* dir_gpio_port;
	uint32_t dir_gpio_pin;
	GPIO_TypeDef* pul_gpio_port;
	uint32_t pul_gpio_pin;
	TIM_HandleTypeDef* timer;
	stepper_mode_t mode;
    bool master_enable;
    bool is_enabled; 
} stepper_handle_t;

typedef enum	{
	STEPPER_CW = GPIO_PIN_SET,
	STEPPER_CCW = GPIO_PIN_RESET
}stepper_dir;

void stepper_enable(stepper_handle_t* motor);
void stepper_disable(stepper_handle_t* motor);
void stepper_master_enable(stepper_handle_t* motor);
void stepper_master_disable(stepper_handle_t* motor);
void stepper_set_dir(stepper_handle_t* motor, stepper_dir direction);
void stepper_set_rpm(stepper_handle_t* motor, uint32_t rpm);
void stepper_move(stepper_handle_t* motor, float angle);
uint32_t angle_to_steps(stepper_handle_t* motor, float angle);

#endif /* INC_STEPPER_H_ */
