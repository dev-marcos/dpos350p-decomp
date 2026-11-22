/*
 * state_backup_all -- stock 0x4003F0C0 (32 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Saves SYS_INFO, the generator state and two other blocks (around calibration).
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "generator.h"
#include "system.h"

void state_backup_all(void)
{
    sys_info_backup();
    gen_state_backup();
    state_c_backup();
    state_d_backup();
}
