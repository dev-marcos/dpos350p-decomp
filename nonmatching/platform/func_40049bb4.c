/*
 * disk_format -- stock 0x40049BB4 (48 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * f_mkfs("0:", FM_FAT, 16 KiB clusters, work 0x428DF874, 1000000).
 */
#include "hw/platform.h"

#include "hw/fpga.h"

int disk_format(void)
{
    return f_mkfs(STR_DRIVE0, 1, 0x4000, MKFS_WORK, 1000000);
}
