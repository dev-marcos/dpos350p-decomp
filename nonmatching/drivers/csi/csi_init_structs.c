/*
 * csi_init_structs -- stock 0x40065FA8 (136 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Fill the interface (0x42E5549C) and format (0x42E18074) descriptors, flush the D-cache, configure, then set the channel sizes (0x40065D04).
 */
#include "hw/platform.h"
#include "drivers/csi.h"
#include "stock_asm.h"
#include "shared/drivers.h"

int csi_init_structs(u32 rate)
{
    unsigned char *ifc = (unsigned char *)0x42E5549Cu, *fmt = (unsigned char *)0x42E18074u;
    *(unsigned short *)(ifc + 6) = 1;
    ifc[0] = 0; ifc[4] = 0; ifc[1] = 1; ifc[2] = 1; ifc[3] = 0;
    *(u32 *)(ifc + 0x10) = 1;
    *(u32 *)(fmt + 4) = rate;
    fmt[0] = 1; fmt[1] = 1;
    *(u32 *)(fmt + 8) = 1;
    *(u32 *)(fmt + 0x24) = 0;
    *(u32 *)(fmt + 0x28) = 0;
    *(unsigned short *)(fmt + 0x44) = 0xFFFF;
    fmt[0x4C] = 0;
    dcache_clean_inv_all();
    csi_init_cfg(0, (const signed char *)ifc);
    return csi_set_fmt(0, ifc, fmt);
}
