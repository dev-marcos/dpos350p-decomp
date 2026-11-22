/*
 * USBC_Dev_ReadDataStatus -- stock 0x4006CC48 (12 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * complete ? __USBC_Dev_ReadDataComplete : __USBC_Dev_ReadDataHalf.
 */
#include "hw/usb.h"
#include "usb/usbc.h"

int USBC_Dev_ReadDataStatus(u32 ep_type, u32 complete)
{
    if (complete) return __USBC_Dev_ReadDataComplete(ep_type);
    return __USBC_Dev_ReadDataHalf(ep_type);
}
