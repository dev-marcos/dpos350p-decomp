/*
 * USBC_Dev_ConfigTransferMode -- stock 0x4006C6A8 (136 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * ts_type 1..4 = CTRL/ISO/INT/BULK; speed 1 = LS, 2 = FS, 3 = HS.
 */
#include "hw/usb.h"
#include "usb/usbc.h"

void USBC_Dev_ConfigTransferMode(u32 ts_type, u32 speed_mode)
{
    switch (ts_type) {
    case 1: __USBC_Dev_TsType_Ctrl(); break;
    case 2: __USBC_Dev_TsType_Iso(); break;
    case 3: __USBC_Dev_TsType_Int(); break;
    case 4: __USBC_Dev_TsType_Bulk(); break;
    default: __USBC_Dev_TsType_default(); break;
    }
    switch (speed_mode) {
    case 3: __USBC_Dev_TsMode_Hs(); break;
    case 2: __USBC_Dev_TsMode_Fs(); break;
    case 1: __USBC_Dev_TsMode_Ls(); break;
    default: __USBC_Dev_TsMode_default(); break;
    }
}
