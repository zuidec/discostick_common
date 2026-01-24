/*
 * uart.h
 *
 *  Created on: Mar 3, 2025
 *      Author: zuidec
 */

#ifndef INC_UART_H_
#define INC_UART_H_

#ifdef __cplusplus
extern "C" {
#endif


#include "main.h"
#include "stdint.h"
#include "fifo.h"
#include "com_packet.h"

#ifndef UART_BUFFER_SIZE
#define UART_BUFFER_SIZE	(128)
#endif

#define DMA_BUF_SIZE		(UART_BUFFER_SIZE)
#define MAX_UART_COUNT		(8)	// 8 bits to store 8 uart instances in uint8_t

typedef enum {
	UART_OK,
	UART_BUSY,
	UART_INIT_FAIL,
    UART_BAD_HANDLE,
	UART_TX_FAIL,
	UART_RX_FAIL,
	UART_RX_FULL
}uart_status_t;

typedef struct	{
	UART_HandleTypeDef* huart;
	uint8_t instance_id;
	uart_status_t status;
	bool tx_busy;
	bool tx_waiting;
	uint32_t tx_remaining;
	uint32_t tx_size;
	volatile uint8_t rx_buffer[UART_BUFFER_SIZE];
	uint8_t tx_buffer[UART_BUFFER_SIZE];
	fifo_buffer_t rx_fifo;
	fifo_buffer_t tx_fifo;
    bool rxlock;
    volatile uint8_t dma_buffer[DMA_BUF_SIZE];
	volatile uint32_t dma_index;
	volatile uint32_t last_dma_size;
	uint32_t unread_bytes;

}uart_handle_t;

uart_status_t uart_init(uart_handle_t* uart, UART_HandleTypeDef* h_uart);
void uart_deinit(uart_handle_t* uart);
uint8_t uart_register_instance(uart_handle_t* uart);
void uart_unregister_instance(uart_handle_t* uart);
uart_handle_t* get_uart_handle(UART_HandleTypeDef* huart);
uart_status_t uart_update(uart_handle_t* uart);
uint32_t uart_read(uart_handle_t* uart, uint8_t* data, uint32_t size);
uint32_t uart_write(uart_handle_t* uart, uint8_t* data, uint32_t size);
void uart_write_packet(uart_handle_t* uart, com_packet_t* packet);

#ifdef __cplusplus
}
#endif

#endif /* INC_UART_H_ */
