/*
 * stepper.c
 *
 *  Created on: Jan 3, 2025
 *      Author: zuidec
 */


#include "stepper.h"

void stepper_enable(stepper_handle_t* motor)	{
    if(true == motor->master_enable)    {
        HAL_GPIO_WritePin(motor->en_gpio_port, motor->en_gpio_pin, GPIO_PIN_SET);
        motor->is_enabled = true;
    }
}

void stepper_disable(stepper_handle_t* motor)	{
	HAL_GPIO_WritePin(motor->en_gpio_port, motor->en_gpio_pin, GPIO_PIN_RESET);
    motor->is_enabled = false;
}

void stepper_master_enable(stepper_handle_t* motor)	{
    motor->master_enable = true;
    stepper_enable(motor);
}

void stepper_master_disable(stepper_handle_t* motor)	{
    motor->master_enable = false;
    stepper_disable(motor);
}
void stepper_set_dir(stepper_handle_t* motor, stepper_dir direction)	{
	switch(direction)	{
	case STEPPER_CW:
		HAL_GPIO_WritePin(motor->dir_gpio_port, motor->dir_gpio_pin, GPIO_PIN_SET);
		break;
	case STEPPER_CCW:
		HAL_GPIO_WritePin(motor->dir_gpio_port, motor->dir_gpio_pin, GPIO_PIN_RESET);
		break;
	default:
		break;
	}
}

void stepper_set_rpm(stepper_handle_t* motor, uint32_t rpm)	{

}

void stepper_move(stepper_handle_t* motor, float angle)	{
	//stepper_enable(motor);

	  uint32_t steps = angle_to_steps(motor, angle);
	  //uint32_t remainder = 0;
	  while(steps > 0){
		  //HAL_TIM_PWM_Stop(motor->timer, TIM_CHANNEL_2);
		  if(steps > UINT16_T_MAX)	{
			  steps -= UINT16_T_MAX;
			  motor->timer->Instance->CNT = UINT16_T_MAX-1;
			  //motor->timer->Init.RepetitionCounter = 256-1;
		  }
		  else	{
			  motor->timer->Instance->CNT = steps-1;
			  steps -= steps;
		  }
		 // HAL_TIM_Base_Init(motor->timer);
		 // HAL_TIM_PWM_Start(motor->timer, TIM_CHANNEL_2);
		  motor->timer->Instance->CR1 |= TIM_CR1_CEN;
		  TIM4->CR1 |= TIM_CR1_CEN;
	  }

}

uint32_t angle_to_steps(stepper_handle_t* motor, float angle)	{
	if(angle <0){
		angle *= -1.0f;
	}
	angle = (angle/STEP_ANGLE) + 0.5f;

	return (uint32_t)angle * ((uint32_t)motor->mode/(uint32_t)STEPS_PER_REV);
}


