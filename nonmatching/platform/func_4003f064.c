/*
 * sys_info_restore -- stock 0x4003F064 (36 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * SYS_INFO back from 0x409223F0, then 0x4003EF1C.
 */
#include "hw/platform.h"

#include "hw/fpga.h"

void sys_info_restore(void)
{
    memcpy((void *)0x40544E68u, SYS_BACKUP, SYS_SIZE);
    func_4003ef1c();
}
