/*
 * csi_readl_t -- stock 0x40065890 (4 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * b csi_readl.
 */
#include "hw/platform.h"
#include "drivers/csi.h"
#include "stock_asm.h"
#include "shared/drivers.h"

u32 csi_readl_t(u32 addr) { return csi_readl(addr); }
