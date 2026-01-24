/*
 * fifo.h
 *
 *  Created on: Mar 3, 2025
 *      Author: zuidec
 */

#ifndef INC_FIFO_H_
#define INC_FIFO_H_

#include "stdbool.h"
#include "stdint.h"

typedef struct fifo_buffer_t    {

    uint8_t* buffer;
    uint32_t mask;
    uint32_t read_index;
    uint32_t write_index;
    uint32_t size;
} fifo_buffer_t;

void fifo_init(fifo_buffer_t* fifo, uint8_t* buffer, uint32_t size);
bool fifo_is_empty(fifo_buffer_t* fifo);
uint32_t fifo_bytes_available(fifo_buffer_t* fifo);
uint32_t fifo_write(fifo_buffer_t* fifo, uint8_t* data, uint32_t size);
uint32_t fifo_read(fifo_buffer_t* fifo, uint8_t* data, uint32_t size);
void fifo_flush_unread(fifo_buffer_t* fifo);
uint8_t fifo_peek(fifo_buffer_t* fifo, uint32_t peek_distance);
uint32_t fifo_peek_continuous(fifo_buffer_t* fifo, uint8_t* data, uint32_t distance, uint32_t size);
void fifo_push_read_index(fifo_buffer_t* fifo, uint32_t distance);


#endif /* INC_FIFO_H_ */
