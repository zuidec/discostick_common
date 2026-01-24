#include <stdint.h>
#include "dev_id_strings.h"

const char* str_F0 = "STM32F0\0";
const char* str_F1 = "STM32F1\0";
const char* str_F2 = "STM32F2\0";
const char* str_F3 = "STM32F3\0";
const char* str_F4 = "STM32F4\0";
const char* str_F7 = "STM32F7\0";
const char* str_H5 = "STM32H5\0";
const char* str_H7 = "STM32H7\0";
const char* str_G0 = "STM32G0\0";
const char* str_G4 = "STM32G4\0";
const char* str_L0 = "STM32L0\0";
const char* str_L1 = "STM32L1\0";
const char* str_L4 = "STM32L4\0";
const char* str_L5 = "STM32L5\0";
const char* str_U5 = "STM32U5\0";
const char* str_WL = "STM32WL\0";
const char* str_WB = "STM32WB\0";
const char* str_MP1= "STM32MP1\0";
const char* str_unknown = "unknown\0";

const char* get_stm32_family_str(uint16_t devid)  {
    const char* devstr = str_unknown;

    switch (devid)
    {
        /* ---------- F0 ---------- */
        case STM32_DEV_ID_F0:
            devstr = str_F0;
            break;

        /* ---------- F1 ---------- */
        case STM32_DEV_ID_F1_LD:
        case STM32_DEV_ID_F1_MD:
        case STM32_DEV_ID_F1_HD:
        case STM32_DEV_ID_F1_XL:
        case STM32_DEV_ID_F1_CL:
            devstr = str_F1;
            break;

        /* ---------- F2 ---------- */
        case STM32_DEV_ID_F2:
            devstr = str_F2;
            break;

        /* ---------- F3 ---------- */
        case STM32_DEV_ID_F3:
        case STM32_DEV_ID_F3_ALT:
            devstr = str_F3;
            break;

        /* ---------- F4 ---------- */
        case STM32_DEV_ID_F4:
        case STM32_DEV_ID_F4_ALT:
            devstr = str_F4;
            break;

        /* ---------- F7 ---------- */
        case STM32_DEV_ID_F7:
            devstr = str_F7;
            break;

        /* ---------- G0 ---------- */
        case STM32_DEV_ID_G0:
            devstr = str_G0;
            break;

        /* ---------- G4 ---------- */
        case STM32_DEV_ID_G4:
            devstr = str_G4;
            break;

        /* ---------- H5 ---------- */
        case STM32_DEV_ID_H5:
            devstr = str_H5;
            break;

        /* ---------- H7 ---------- */
        case STM32_DEV_ID_H7:
            devstr = str_H7;
            break;

        /* ---------- L0 ---------- */
        case STM32_DEV_ID_L0:
            devstr = str_L0;
            break;

        /* ---------- L1 ---------- */
        case STM32_DEV_ID_L1:
            devstr = str_L1;
            break;

        /* ---------- L4 / L4+ ---------- */
        case STM32_DEV_ID_L4:
        case STM32_DEV_ID_L4_PLUS:
            devstr = str_L4;
            break;

        /* ---------- L5 ---------- */
        case STM32_DEV_ID_L5:
            devstr = str_L5;
            break;

        /* ---------- U5 ---------- */
        case STM32_DEV_ID_U5:
            devstr = str_U5;
            break;

        /* ---------- WB ---------- */
        case STM32_DEV_ID_WB:
            devstr = str_WB;
            break;

        /* ---------- WL ---------- */
        case STM32_DEV_ID_WL:
            devstr = str_WL;
            break;

        /* ---------- MP1 ---------- */
        case STM32_DEV_ID_MP1:
            devstr = str_MP1;
            break;

        default:
            break;
    }

    return devstr;
}
