/*
 *	bus.h
 *	(enter file description here)
 *
 *	Created by zuidec on 01/27/26
 */

#ifndef BUS_H
#define BUS_H	// BEGIN BUS_H

#ifdef __cplusplus
extern "C" {
#endif

/*
 *	Includes
 */
#include <stdint.h>
#include <stddef.h>



/*
 *	Defines
 */
#define ADDR_NONE   (0)
#define ADDR_8_BIT  (1)
#define ADDR_16_BIT (2)
#define ADDR_24_BIT (3)
#define ADDR_32_BIT (4)
#define DUMMY_NONE  (0)

/*
 *	Structs, unions, etc...
 */


typedef struct bus bus_t;
/*
* Execute a command-style SPI transaction.
*
* Typical sequence:
* [CS low]
* opcode 
* address (0–4 bytes, MSB first)
* dummy cycles (optional)
* data phase (tx or rx)
* [CS high]
*
* Parameters:
* - bus : initialized bus instance
* - opcode : command opcode
* - addr : address value (ignored if addr_len == 0)
* - addr_len : number of address bytes (0–4)
* - dummy : number of dummy cycles (0 if unused)
* - txbuf : data to transmit (NULL if reading)
* - rxbuf : buffer to receive data (NULL if writing)
* - len : number of data bytes
*
* Returns:
* 0 on success, negative error code on failure
*/
typedef int (*bus_read_fn)(
    bus_t* bus, 
    uint8_t opcode, 
    uint32_t addr, 
    uint8_t addr_len, 
    uint8_t dummy, 
    uint8_t* rxbuf, 
    size_t len
);
typedef int (*bus_write_fn)(
    bus_t* bus, 
    uint8_t opcode, 
    uint32_t addr, 
    uint8_t addr_len, 
    uint8_t dummy, 
    const uint8_t* txbuf, 
    size_t len
);

struct bus {
    bus_write_fn write;
    bus_read_fn read;
    void* ctx;
};



/*
 *  Function prototypes
 */

#ifdef __cplusplus
}
#endif

#endif	// END BUS_H
