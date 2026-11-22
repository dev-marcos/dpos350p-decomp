/*
 * USBC_Dev_WriteDataStatus -- stock 0x4006CC3C (12 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * complete ? __USBC_Dev_WriteDataComplete : __USBC_Dev_WriteDataHalf.
 */
#include "hw/usb.h"
#include "usb/usbc.h"

int USBC_Dev_WriteDataStatus(u32 ep_type, u32 complete)
{
    if (complete) return __USBC_Dev_WriteDataComplete(ep_type);
    return __USBC_Dev_WriteDataHalf(ep_type);
}
