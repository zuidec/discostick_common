/*
 *	lfs_conf.h
 *	(enter file description here)
 *
 *	Created by zuidec on 01/17/26
 */

#ifndef LFS_CONF_H
#define LFS_CONF_H	// BEGIN LFS_CONF_H

/*
 *	Includes
 */
#include "lfs.h"
#include "w25q16jv.h"


/*
 *	Defines
 */
#ifndef LFS_CACHE_SIZE
#define LFS_CACHE_SIZE 1024
#endif

#ifndef LFS_READ_SIZE
#define LFS_READ_SIZE 1024
#endif

#ifndef LFS_LOOKAHEAD_SIZE
#define LFS_LOOKAHEAD_SIZE 1024
#endif

#ifndef LFS_BLOCK_CYCLES
#define LFS_BLOCK_CYCLES 500
#endif


/*
 *	Structs, unions, etc...
 */

// Error codes negative to allow positive return and spaced out 
// to not interfere with littleFS or flash error codes
enum fs_error {
    FS_ERR_MNT_FAIL = -7,
    FS_ERR_OPEN_FAIL = -8,
    FS_ERR_READ_FAIL = -10,
    FS_ERR_WRITE_FAIL = -11,
    FS_ERR_CLOSE_FAIL = -13,
    FS_ERR_NOAXIS = -15
};

/*
 *  Exported variables
 */


/*
 *	Function prototypes
 */


void format_flashfs(w25q16_handle_t* flash_dev);
int32_t init_flashfs(w25q16_handle_t* flash_dev);

int32_t load_calibration(uint8_t axis, uint8_t* data, uint32_t size);
int32_t save_calibration(uint8_t axis, uint8_t* data, uint32_t size);

int read_flash_region(const struct lfs_config* cfg, uint32_t block, uint32_t off, void* buffer, uint32_t size);
int prog_flash_region(const struct lfs_config* cfg, uint32_t block, uint32_t off, const void* buffer, uint32_t size);
int erase_flash_region(const struct lfs_config* cfg, uint32_t block);
int sync_flash(const struct lfs_config* cfg);

#endif	// END LFS_CONF_H
