/*
 *	spi_bus.c
 *	(enter file description here)
 *
 *	Created by zuidec on 01/27/26
 */
#include "hal_shim.h"
#include "spi_bus.h"


static inline void cs_enable(spi_bus_ctx_t* ctx)	{
        HAL_GPIO_WritePin((GPIO_TypeDef*)ctx->cs_port, ctx->cs_pin, GPIO_PIN_RESET);
}

static inline void cs_release(spi_bus_ctx_t* ctx)	{
        HAL_GPIO_WritePin((GPIO_TypeDef*)ctx->cs_port, ctx->cs_pin, GPIO_PIN_SET);
}

static int spi_bus_write(bus_t* bus, uint8_t opcode, uint32_t addr, uint8_t addr_len,
                        uint8_t dummy, const uint8_t* txbuf, size_t len)    {
    spi_bus_ctx_t* ctx = (spi_bus_ctx_t*)bus->ctx;
    SPI_HandleTypeDef* hspi = (SPI_HandleTypeDef*)ctx->hspi;

    int ret = 0;
    cs_enable(ctx);
    /* Opcode */
    ret = HAL_SPI_Transmit(hspi, &opcode, 1, ctx->timeout);
    if(HAL_OK!= ret) {goto error;}

    /* Address (MSB first) */
    for (int i = addr_len - 1; i >= 0; i--) {
        uint8_t b = (addr >> (8 * i)) & 0xFF;
        ret = HAL_SPI_Transmit(hspi, &b, 1, ctx->timeout);
        if(HAL_OK!= ret) {goto error;}
    }

    /* Dummy cycles (clock out dummy bytes) */
    if (dummy) {
        uint8_t d = 0xFF;
        for (uint8_t i = 0; i < dummy; i++) {
            if (HAL_SPI_Transmit(hspi, &d, 1, ctx->timeout) != HAL_OK)    {
                // Error
                ret = -1;
            }
        if(HAL_OK!= ret) {goto error;}
        }
    }

    if(len > 0 && txbuf) {
        ret = HAL_SPI_Transmit(hspi, (uint8_t*)txbuf, len, ctx->timeout);
        if(HAL_OK!= ret) {goto error;}
    }

error:
    cs_release(ctx);
    return ret;
}

static int spi_bus_read(bus_t* bus, uint8_t opcode, uint32_t addr, uint8_t addr_len,
                        uint8_t dummy, uint8_t* rxbuf, size_t len)    {
    spi_bus_ctx_t* ctx = (spi_bus_ctx_t*)bus->ctx;
    SPI_HandleTypeDef* hspi = (SPI_HandleTypeDef*)ctx->hspi;

    int ret = 0;
    cs_enable(ctx);
    /* Opcode */
    ret = HAL_SPI_Transmit(hspi, &opcode, 1, ctx->timeout);
    if(HAL_OK!= ret) {goto error;}

    /* Address (MSB first) */
    for (int i = addr_len - 1; i >= 0; i--) {
        uint8_t b = (addr >> (8 * i)) & 0xFF;
        ret = HAL_SPI_Transmit(hspi, &b, 1, ctx->timeout);
        if(HAL_OK!= ret) {goto error;}
    }

    /* Dummy cycles (clock out dummy bytes) */
    if (dummy) {
        uint8_t d = 0xFF;
        for (uint8_t i = 0; i < dummy; i++) {
            if (HAL_SPI_Transmit(hspi, &d, 1, ctx->timeout) != HAL_OK)    {
                // Error
                ret = -1;
            }
        if(HAL_OK!= ret) {goto error;}
        }
    }

    if(len > 0 && rxbuf) {
        ret = HAL_SPI_Receive(hspi, rxbuf, len, ctx->timeout);
        if(HAL_OK!= ret) {goto error;}
    }

error:
    cs_release(ctx);
    return ret;
}

int spi_bus_init(bus_t *bus, spi_bus_ctx_t *ctx)    {
    if(!bus || !ctx)    {
        return -1;
    }
    bus->read = spi_bus_read;
    bus->write= spi_bus_write;
    bus->ctx = ctx;
    ctx->timeout = SPI_BUS_TIMEOUT_MS;
    return 0;
}

void spi_bus_set_timeout(spi_bus_ctx_t* ctx, uint32_t millis)   {
    ctx->timeout = millis;
}

