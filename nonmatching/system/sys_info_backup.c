/*
 * sys_info_backup -- stock 0x4003F024 (16 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Copies SYS_INFO (0x3DD588 bytes) into 0x409223F0.
 */
#include "hw/platform.h"

#include "hw/fpga.h"

void sys_info_backup(void)
{
    memcpy(SYS_BACKUP, (void *)0x40544E68u, SYS_SIZE);
}
