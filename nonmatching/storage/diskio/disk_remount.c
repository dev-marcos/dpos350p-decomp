/*
 * disk_remount -- stock 0x40049B90 (36 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Unmount "0:" and mount it again.
 */
#include "hw/platform.h"

#include "hw/fpga.h"

int disk_remount(void)
{
    f_mount(0, STR_DRIVE0, 1);
    return disk_mount();
}
