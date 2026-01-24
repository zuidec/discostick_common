/*
 *	dev_id_strings.h
 *	(enter file description here)
 *
 *	Created by zuidec on 01/15/26
 */

#ifndef DEV_ID_STRINGS_H
#define DEV_ID_STRINGS_H	// BEGIN DEV_ID_STRINGS_H

/*
 *	Includes
 */
#include <stdint.h>


/*
 *	Defines
 */
/* =============================== */
/* STM32 DBGMCU DEV_ID DEFINITIONS */
/* =============================== */

/* STM32F0 */
#define STM32_DEV_ID_F0              0x440

/* STM32F1 */
#define STM32_DEV_ID_F1_LD           0x412
#define STM32_DEV_ID_F1_MD           0x410
#define STM32_DEV_ID_F1_HD           0x414
#define STM32_DEV_ID_F1_XL           0x430
#define STM32_DEV_ID_F1_CL           0x418

/* STM32F2 */
#define STM32_DEV_ID_F2              0x411

/* STM32F3 */
#define STM32_DEV_ID_F3              0x422
#define STM32_DEV_ID_F3_ALT          0x432

/* STM32F4 */
#define STM32_DEV_ID_F4              0x431
#define STM32_DEV_ID_F4_ALT          0x413

/* STM32F7 */
#define STM32_DEV_ID_F7              0x449

/* STM32G0 */
#define STM32_DEV_ID_G0              0x466

/* STM32G4 */
#define STM32_DEV_ID_G4              0x468

/* STM32H5 */
#define STM32_DEV_ID_H5              0x483

/* STM32H7 */
#define STM32_DEV_ID_H7              0x450

/* STM32L0 */
#define STM32_DEV_ID_L0              0x417

/* STM32L1 */
#define STM32_DEV_ID_L1              0x416

/* STM32L4 / L4+ */
#define STM32_DEV_ID_L4              0x415
#define STM32_DEV_ID_L4_PLUS         0x470

/* STM32L5 */
#define STM32_DEV_ID_L5              0x472

/* STM32U5 */
#define STM32_DEV_ID_U5              0x482

/* STM32WB */
#define STM32_DEV_ID_WB              0x495

/* STM32WL */
#define STM32_DEV_ID_WL              0x496

/* STM32MP1 */
#define STM32_DEV_ID_MP1             0x500



/*
 *	Structs, unions, etc...
 */
typedef enum
{
    STM32_FAMILY_UNKNOWN = 0,

    STM32_FAMILY_F0,
    STM32_FAMILY_F1,
    STM32_FAMILY_F2,
    STM32_FAMILY_F3,
    STM32_FAMILY_F4,
    STM32_FAMILY_F7,

    STM32_FAMILY_G0,
    STM32_FAMILY_G4,

    STM32_FAMILY_H5,
    STM32_FAMILY_H7,

    STM32_FAMILY_L0,
    STM32_FAMILY_L1,
    STM32_FAMILY_L4,
    STM32_FAMILY_L5,

    STM32_FAMILY_U5,

    STM32_FAMILY_WB,
    STM32_FAMILY_WL,

    STM32_FAMILY_MP1

} stm32_family_t;


extern const char* get_stm32_family_str(uint16_t); 

/*
 *	Function prototypes
 */




#endif	// END DEV_ID_STRINGS_H
