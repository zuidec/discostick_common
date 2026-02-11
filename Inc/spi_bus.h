/*
 *	spi_bus.h
 *	(enter file description here)
 *
 *	Created by zuidec on 01/27/26
 */

#ifndef SPI_BUS_H
#define SPI_BUS_H	// BEGIN SPI_BUS_H

#ifdef __cplusplus
extern "C" {
#endif

/*
 *	Includes
 */
#include <stdint.h>
#include "bus.h"


/*
 *	Defines
 */

#define SPI_BUS_TIMEOUT_MS (100)


/*
 *	Structs, unions, etc...
 */

typedef struct {
    void *hspi; /* SPI_HandleTypeDef* */
    void *cs_port; /* GPIO_TypeDef* */
    uint16_t cs_pin;
    uint32_t timeout;
} spi_bus_ctx_t;


/*
 *	Function prototypes
 */

int spi_bus_init(bus_t* bus, spi_bus_ctx_t* ctx);
void spi_bus_set_timeout(spi_bus_ctx_t* ctx, uint32_t millis);



#ifdef __cplusplus
}
#endif

#endif	// END SPI_BUS_H
