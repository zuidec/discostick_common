/*
 * input_filter.h
 *
 *  Created on: Dec 31, 2024
 *      Author: zuidec
 */

#ifndef INC_INPUT_FILTER_H_
#define INC_INPUT_FILTER_H_

typedef struct __attribute__((packed))  {
	float alpha;
	float output;
} input_filter_t;


void input_filter_init(input_filter_t* filter, float alpha);
void input_filter_set_alpha(input_filter_t* filter, float alpha);
float input_filter_update(input_filter_t* filter, float input);
#endif /* INC_INPUT_FILTER_H_ */
