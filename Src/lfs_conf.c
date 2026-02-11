/*
 *	lfs_pedals.c
 *	(enter file description here)
 *
 *	Created by zuidec on 01/17/26
 */

#include "lfs_conf.h"
#include "lfs.h"
#include "w25qxx.h"
#include "logger.h"
#include "bus.h"

#define PITCH_CONFIG_FILENAME       "pitch_config"
#define ROLL_CONFIG_FILENAME        "roll_config"
#define YAW_CONFIG_FILENAME         "yaw_config"
#define THRUST_CONFIG_FILENAME      "thrust_config"
#define L_BRAKE_CONFIG_FILENAME     "l_brake_config"
#define R_BRAKE_CONFIG_FILENAME     "r_brake_config"
#define LOG_PREFIX                  "[flashfs]"

#ifdef USE_HW_CRC
#define LFS_CRC(x,y,z) crc32_calc(x,y,z)
#endif

//
// These are defined in axis_control.h, avoiding including unnecessary headers
//
#define AXIS_PITCH       0x01
#define AXIS_ROLL        0x02
#define AXIS_YAW         0x03
#define AXIS_THRUST      0x04
#define AXIS_L_BRAKE     0x05
#define AXIS_R_BRAKE     0x06


/******************************************************************************
 *  Functions declarations
 *****************************************************************************/

uint32_t(*crc32_calc)(uint32_t,const void*, size_t);

static void log_file_open_error(char*,int);
static void log_file_read_error(char*,int);
static void log_file_close_error(char*,int);
static void log_file_write_error(char*,int);
static void log_fs_mount_error(int);

static int read_flash_region(const struct lfs_config* cfg, uint32_t block, uint32_t off, void* buffer, uint32_t size);
static int prog_flash_region(const struct lfs_config* cfg, uint32_t block, uint32_t off, const void* buffer, uint32_t size);
static int erase_flash_region(const struct lfs_config* cfg, uint32_t block);
static int sync_flash(const struct lfs_config* cfg);

/******************************************************************************
 * Variables 
 *****************************************************************************/

static uint8_t lfs_read_buffer[LFS_READ_SIZE] = {0};
static uint8_t lfs_prog_buffer[LFS_CACHE_SIZE] = {0};
static uint8_t lfs_lookahead_buffer[LFS_LOOKAHEAD_SIZE] = {0};
static uint8_t lfs_file_buffer[LFS_CACHE_SIZE] = {0};

static bool fs_initialized = false;

lfs_t flash_fs;

static struct lfs_file_config file_cfg =   {
    .buffer = lfs_file_buffer
};

static lfs_file_t config_file = {
    .cfg = &file_cfg 
};

struct lfs_config flash_config = {
    // block device operations
    .context = NULL,
    .read  = read_flash_region,
    .prog  = prog_flash_region,
    .erase = erase_flash_region,
    .sync  = sync_flash,

    // block device configuration
    .read_size = sizeof(uint8_t),
    .prog_size = sizeof(uint8_t),
    .block_size = W25Q_BLOCK_SIZE,
    .block_count = W25Q_BLOCK_COUNT,
    .cache_size = LFS_CACHE_SIZE,
    .lookahead_size = LFS_LOOKAHEAD_SIZE,
    .block_cycles = LFS_BLOCK_CYCLES,
    .read_buffer = lfs_read_buffer,
    .prog_buffer = lfs_prog_buffer,
    .lookahead_buffer = lfs_lookahead_buffer
};

/******************************************************************************
 * Static functions 
 *****************************************************************************/

static void log_file_open_error(char* filename,int error)  {
    log_error("%sFailed to open file %s with error: %d",LOG_PREFIX,filename, error);
}

static void log_file_read_error(char* filename,int error)  {
    log_error("%sFailed to read file %s with error: %d",LOG_PREFIX,filename, error);
}
static void log_file_close_error(char* filename,int error) {
    log_error("%sFailed to close file %s with error: %d",LOG_PREFIX,filename, error);
}

static void log_file_write_error(char* filename,int error) {
    log_error("%sFailed to write file %s with error: %d",LOG_PREFIX,filename, error);
}

static void log_fs_mount_error(int error) {
    log_error("%sFailed to mount filesystem with error: %d",LOG_PREFIX,error);
}

static int read_flash_region(const struct lfs_config* cfg, uint32_t block, uint32_t off, void* buffer, uint32_t size)    {
    int32_t ret = 0;
    bus_t* flash_dev = cfg->context;
    uint32_t address = (block * W25Q_BLOCK_SIZE) + off;
    w25q_read(flash_dev, address, (uint8_t*)buffer, size);

    return ret;
}

static int prog_flash_region(const struct lfs_config* cfg, uint32_t block, uint32_t off, const void* buffer, uint32_t size)  {
    int32_t ret = 0;
    bus_t* flash_dev = cfg->context;
    uint32_t address = (block * W25Q_BLOCK_SIZE) + off;
    ret = w25q_write(flash_dev, address, (uint8_t*)buffer, size);
    if(ret != size) {
        log_error("Expected %lu bytes written but got %d!", size, ret);
        ret = -5;
    }
    else    {
        ret = 0; //littlefs expects returning 0 unless there was an error
    }
    return ret;
}

static int erase_flash_region(const struct lfs_config* cfg, uint32_t block) {
    int32_t ret = 0;
    bus_t* flash_dev = cfg->context;
    uint32_t address = block * W25Q_BLOCK_SIZE;
    w25q_sector_erase_4k(flash_dev, address);
    return ret;
}

static int sync_flash(const struct lfs_config* cfg)    {
    return 0;
}
/******************************************************************************
 * Functions 
 *****************************************************************************/

void register_flashfs_crc32(uint32_t(*cb)(uint32_t,const void*, size_t))    {
   crc32_calc = cb; 
}

void format_flashfs(bus_t* flash_dev) {
    uint32_t conf_code = w25q_get_chip_erase_confirmation();
    bool result = w25q_chip_erase(flash_dev, conf_code);
    if(result)  {
        lfs_format(&flash_fs, &flash_config);
    }
}

int32_t init_flashfs(bus_t* flash_dev)  {

    flash_config.context = flash_dev;
    int result = w25q_init(flash_config.context);
    if(result >= 0) { 
        result = lfs_mount(&flash_fs, &flash_config);
    }
    if(result < 0)  {
        log_fs_mount_error(result);
    }
    else    {
        fs_initialized = true;
    }

    (void) lfs_unmount(&flash_fs);  // Can safely ignore return, always returns 0
                                    // without threads
    
    return (int32_t)result;
}

int32_t read_file(const char* filename, uint8_t* data, uint32_t size)   {
    int32_t ret =0;
    if(true != fs_initialized)  {
        ret = FS_ERR_NO_INIT; 
        goto cleanup;
    }
    
    ret = lfs_mount(&flash_fs, &flash_config);
    if(ret < 0)  {
        log_fs_mount_error(ret);
        goto cleanup;
    }
    ret = lfs_file_opencfg(&flash_fs, &config_file, filename, LFS_O_RDONLY, &file_cfg );
    if(ret < 0)  {
        log_file_open_error((char*)filename, ret);
        goto cleanup;
    }
    ret = lfs_file_read(&flash_fs, &config_file, data, size);
    if(ret<0)    {
        log_file_read_error((char*)filename, ret);
        goto cleanup;
    }
    int32_t result = ret;
    ret = lfs_file_close(&flash_fs, &config_file);
    if(ret >0)   {
        log_file_close_error((char*)filename, ret);
        goto cleanup;
    }

    ret = result; // Return the result from write if successful
cleanup:
    (void)lfs_unmount(&flash_fs); // Can safely ignore return, always returns 0
                                  // without threads
    return ret;

}

int32_t save_file(const char* filename, const uint8_t* data, uint32_t size)   {
    int32_t ret =0;
    if(true != fs_initialized)  {
        ret = FS_ERR_NO_INIT; 
        goto cleanup;
    }
    
    ret = lfs_mount(&flash_fs, &flash_config);
    if(ret < 0)  {
        log_fs_mount_error(ret);
        goto cleanup;
    }
    ret = lfs_file_opencfg(&flash_fs, &config_file, filename, LFS_O_RDWR | LFS_O_CREAT, &file_cfg );
    if(ret < 0)  {
        log_file_open_error((char*)filename, ret);
        goto cleanup;
    }
    int32_t result = ret;
    ret = lfs_file_write(&flash_fs, &config_file, data, size);
    if(ret<0)    {
        log_file_write_error((char*)filename, ret);
        goto cleanup;
    }
    ret = lfs_file_close(&flash_fs, &config_file);
    if(ret >0)   {
        log_file_close_error((char*)filename, ret);
        goto cleanup;
    }

    ret = result; // Return the result from write if successful
cleanup:
    (void)lfs_unmount(&flash_fs); // Can safely ignore return, always returns 0
                                  // without threads
    return ret;

}

int32_t load_calibration(uint8_t axis, uint8_t* data, uint32_t size){
    int32_t ret =0;
    if(true != fs_initialized)  {
        ret = FS_ERR_NO_INIT; 
        return ret;
    }
    
    int result = lfs_mount(&flash_fs, &flash_config);
    if(result < 0)  {
        log_fs_mount_error(result);
    }
    switch(axis)    {
        case AXIS_PITCH:
            if(result >= 0) {
                //result = lfs_file_open(&flash_fs, &config_file, PITCH_CONFIG_FILENAME, LFS_O_RDONLY );
                result = lfs_file_opencfg(&flash_fs, &config_file, PITCH_CONFIG_FILENAME, LFS_O_RDONLY, &file_cfg );
                if(result < 0)  {
                    log_file_open_error(PITCH_CONFIG_FILENAME, result);
                }
            }
            if(result >= 0) {
                result = lfs_file_read(&flash_fs, &config_file, data, size);
                ret = result;
                if(result<0)    {
                    log_file_read_error(PITCH_CONFIG_FILENAME, result);
                }
                result = lfs_file_close(&flash_fs, &config_file);
                if(result >0)   {
                    log_file_close_error(PITCH_CONFIG_FILENAME, result);
                }
            }
            break;
        case AXIS_ROLL:
            if(result >= 0) {
                //result = lfs_file_open(&flash_fs, &config_file, ROLL_CONFIG_FILENAME, LFS_O_RDONLY );
                result = lfs_file_opencfg(&flash_fs, &config_file, ROLL_CONFIG_FILENAME, LFS_O_RDONLY, &file_cfg );
                if(result < 0)  {
                    log_file_open_error(ROLL_CONFIG_FILENAME, result);
                }
            }
            if(result >= 0) {
                result = lfs_file_read(&flash_fs, &config_file, data, size);
                ret = result;
                if(result<0)    {
                    log_file_read_error(ROLL_CONFIG_FILENAME, result);
                }
                result = lfs_file_close(&flash_fs, &config_file);
                if(result >0)   {
                    log_file_close_error(ROLL_CONFIG_FILENAME, result);
                }
            }
            break;
        case AXIS_THRUST:
            if(result >= 0) {
                result = lfs_file_open(&flash_fs, &config_file, THRUST_CONFIG_FILENAME, LFS_O_RDONLY );
                result = lfs_file_opencfg(&flash_fs, &config_file, ROLL_CONFIG_FILENAME, LFS_O_RDONLY, &file_cfg );
                if(result < 0)  {
                    log_file_open_error(THRUST_CONFIG_FILENAME, result);
                }
            }
            if(result >= 0) {
                result = lfs_file_read(&flash_fs, &config_file, data, size);
                ret = result;
                if(result<0)    {
                    log_file_read_error(THRUST_CONFIG_FILENAME, result);
                }
                result = lfs_file_close(&flash_fs, &config_file);
                if(result >0)   {
                    log_file_close_error(THRUST_CONFIG_FILENAME, result);
                }
            }
            break;
        case AXIS_YAW:
            if(result >= 0) {
                result = lfs_file_open(&flash_fs, &config_file, YAW_CONFIG_FILENAME, LFS_O_RDONLY );
                result = lfs_file_opencfg(&flash_fs, &config_file, ROLL_CONFIG_FILENAME, LFS_O_RDONLY, &file_cfg );
                if(result < 0)  {
                    log_file_open_error(YAW_CONFIG_FILENAME, result);
                }
            }
            if(result >= 0) {
                result = lfs_file_read(&flash_fs, &config_file, data, size);
                ret = result;
                if(result<0)    {
                    log_file_read_error(YAW_CONFIG_FILENAME, result);
                }
                result = lfs_file_close(&flash_fs, &config_file);
                if(result >0)   {
                    log_file_close_error(YAW_CONFIG_FILENAME, result);
                }
            }
            break;
        case AXIS_L_BRAKE:
            if(result >= 0) {
                result = lfs_file_open(&flash_fs, &config_file, L_BRAKE_CONFIG_FILENAME, LFS_O_RDONLY );
                result = lfs_file_opencfg(&flash_fs, &config_file, ROLL_CONFIG_FILENAME, LFS_O_RDONLY, &file_cfg );
                if(result < 0)  {
                    log_file_open_error(L_BRAKE_CONFIG_FILENAME, result);
                }
            }
            if(result >= 0) {
                result = lfs_file_read(&flash_fs, &config_file, data, size);
                ret = result;
                if(result<0)    {
                    log_file_read_error(L_BRAKE_CONFIG_FILENAME, result);
                }
                result = lfs_file_close(&flash_fs, &config_file);
                if(result >0)   {
                    log_file_close_error(L_BRAKE_CONFIG_FILENAME, result);
                }
            }
            break;
        case AXIS_R_BRAKE:
            if(result >= 0) {
                result = lfs_file_open(&flash_fs, &config_file, R_BRAKE_CONFIG_FILENAME, LFS_O_RDONLY );
                result = lfs_file_opencfg(&flash_fs, &config_file, ROLL_CONFIG_FILENAME, LFS_O_RDONLY, &file_cfg );
                if(result < 0)  {
                    log_file_open_error(R_BRAKE_CONFIG_FILENAME, result);
                }
            }
            if(result >= 0) {
                result = lfs_file_read(&flash_fs, &config_file, data, size);
                ret = result;
                if(result<0)    {
                    log_file_read_error(R_BRAKE_CONFIG_FILENAME, result);
                }
                result = lfs_file_close(&flash_fs, &config_file);
                if(result >0)   {
                    log_file_close_error(R_BRAKE_CONFIG_FILENAME, result);
                }
            }
            break;
        default:
            result = FS_ERR_NO_AXIS;
            break;
    }

    (void)lfs_unmount(&flash_fs); // Can safely ignore return, always returns 0
                                  // without threads
    if(result < 0)  {
        ret = result;
    }

    // Else returining bytes read 
    return ret;
}

int32_t save_calibration(uint8_t axis, uint8_t* data, uint32_t size)    {
    int32_t ret =0;
    if(true != fs_initialized)  {
        ret = FS_ERR_NO_INIT; 
    }
    
    int result = lfs_mount(&flash_fs, &flash_config);
    if(result < 0)  {
        log_fs_mount_error(result);
    }
    switch(axis)    {
        case AXIS_PITCH:
            if(result >= 0) {
                result = lfs_file_opencfg(&flash_fs, &config_file, PITCH_CONFIG_FILENAME, LFS_O_RDWR | LFS_O_CREAT, &file_cfg);
                if(result < 0)  {
                    log_file_open_error(PITCH_CONFIG_FILENAME, result);
                }
            }
            if(result >= 0) {
                lfs_file_rewind(&flash_fs, &config_file);
                result = lfs_file_write(&flash_fs, &config_file, data, size);
                ret = result;
                if(result<0)    {
                    log_file_write_error(PITCH_CONFIG_FILENAME, result);
                }
                result = lfs_file_close(&flash_fs, &config_file);
                if(result >0)   {
                    log_file_close_error(PITCH_CONFIG_FILENAME, result);
                }
            }
            break;
        case AXIS_ROLL:
            if(result >= 0) {
                //result = lfs_file_open(&flash_fs, &config_file, ROLL_CONFIG_FILENAME, LFS_O_RDWR | LFS_O_CREAT);
                result = lfs_file_opencfg(&flash_fs, &config_file, ROLL_CONFIG_FILENAME, LFS_O_RDWR | LFS_O_CREAT, &file_cfg);
                if(result < 0)  {
                    log_file_open_error(ROLL_CONFIG_FILENAME, result);
                }
            }
            if(result >= 0) {
                lfs_file_rewind(&flash_fs, &config_file);
                result = lfs_file_write(&flash_fs, &config_file, data, size);
                ret = result;
                if(result<0)    {
                    log_file_write_error(ROLL_CONFIG_FILENAME, result);
                }
                result = lfs_file_close(&flash_fs, &config_file);
                if(result >0)   {
                    log_file_close_error(ROLL_CONFIG_FILENAME, result);
                }
            }
            break;
        case AXIS_THRUST:
            if(result >= 0) {
                //result = lfs_file_open(&flash_fs, &config_file, THRUST_CONFIG_FILENAME, LFS_O_RDWR | LFS_O_CREAT);
                result = lfs_file_opencfg(&flash_fs, &config_file, THRUST_CONFIG_FILENAME, LFS_O_RDWR | LFS_O_CREAT, &file_cfg);
                if(result < 0)  {
                    log_file_open_error(THRUST_CONFIG_FILENAME, result);
                }
            }
            if(result >= 0) {
                lfs_file_rewind(&flash_fs, &config_file);
                result = lfs_file_write(&flash_fs, &config_file, data, size);
                ret = result;
                if(result<0)    {
                    log_file_write_error(THRUST_CONFIG_FILENAME, result);
                }
                result = lfs_file_close(&flash_fs, &config_file);
                if(result >0)   {
                    log_file_close_error(THRUST_CONFIG_FILENAME, result);
                }
            }
            break;
        case AXIS_YAW:
            if(result >= 0) {
                //result = lfs_file_open(&flash_fs, &config_file, YAW_CONFIG_FILENAME, LFS_O_RDWR | LFS_O_CREAT);
                result = lfs_file_opencfg(&flash_fs, &config_file, YAW_CONFIG_FILENAME, LFS_O_RDWR | LFS_O_CREAT, &file_cfg);
                if(result < 0)  {
                    log_file_open_error(YAW_CONFIG_FILENAME, result);
                }
            }
            if(result >= 0) {
                lfs_file_rewind(&flash_fs, &config_file);
                result = lfs_file_write(&flash_fs, &config_file, data, size);
                ret = result;
                if(result<0)    {
                    log_file_write_error(YAW_CONFIG_FILENAME, result);
                }
                result = lfs_file_close(&flash_fs, &config_file);
                if(result >0)   {
                    log_file_close_error(YAW_CONFIG_FILENAME, result);
                }
            }
            break;
        case AXIS_L_BRAKE:
            if(result >= 0) {
                //result = lfs_file_open(&flash_fs, &config_file, L_BRAKE_CONFIG_FILENAME, LFS_O_RDWR | LFS_O_CREAT);
                result = lfs_file_opencfg(&flash_fs, &config_file, L_BRAKE_CONFIG_FILENAME, LFS_O_RDWR | LFS_O_CREAT, &file_cfg);
                if(result < 0)  {
                    log_file_open_error(L_BRAKE_CONFIG_FILENAME, result);
                }
            }
            if(result >= 0) {
                result = lfs_file_write(&flash_fs, &config_file, data, size);
                ret = result;
                if(result<0)    {
                    log_file_read_error(L_BRAKE_CONFIG_FILENAME, result);
                }
                result = lfs_file_close(&flash_fs, &config_file);
                if(result >0)   {
                    log_file_close_error(L_BRAKE_CONFIG_FILENAME, result);
                }
            }
            break;
        case AXIS_R_BRAKE:
            if(result >= 0) {
                //result = lfs_file_open(&flash_fs, &config_file, R_BRAKE_CONFIG_FILENAME, LFS_O_RDWR | LFS_O_CREAT);
                result = lfs_file_opencfg(&flash_fs, &config_file, R_BRAKE_CONFIG_FILENAME, LFS_O_RDWR | LFS_O_CREAT, &file_cfg);
                if(result < 0)  {
                    log_file_open_error(R_BRAKE_CONFIG_FILENAME, result);
                }
            }
            if(result >= 0) {
                result = lfs_file_write(&flash_fs, &config_file, data, size);
                ret = result;
                if(result<0)    {
                    log_file_read_error(R_BRAKE_CONFIG_FILENAME, result);
                }
                result = lfs_file_close(&flash_fs, &config_file);
                if(result >0)   {
                    log_file_close_error(R_BRAKE_CONFIG_FILENAME, result);
                }
            }
            break;
        default:
            result = FS_ERR_NO_AXIS; 
            break;
    }
    // todo: brake calibrations
    (void)lfs_unmount(&flash_fs); // Can safely ignore return, always returns 0
                                  // without threads
    if(result < 0)  {
        ret = result;
    }

    // Else returining bytes read 
    return ret;

}
