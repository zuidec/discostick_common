/*
 * input_filter.c
 *
 *  Created on: Dec 31, 2024
 *      Author: zuidec
 */


#include "input_filter.h"

void input_filter_init(input_filter_t* filter, float alpha)	{
	input_filter_set_alpha(filter, alpha);
	filter->output = 0.0f;
}

void input_filter_set_alpha(input_filter_t* filter, float alpha)	{
	if(alpha > 1.0f){
		alpha = 1.0f;
	}
	else if (alpha < 0.0f){
		alpha = 0.0f;
	}
	filter->alpha = alpha;
}

float input_filter_update(input_filter_t* filter, float input)	{
	filter->output = filter->alpha * input + (1.0f - filter->alpha) * filter->output;
	return filter->output;
}
