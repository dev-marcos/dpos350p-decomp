/*
 * state_restore_all -- stock 0x4003F0E0 (32 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Restores what state_backup_all saved.
 */
#include "hw/platform.h"

#include "hw/fpga.h"

void sys_info_restore(void); void gen_state_restore(void); void state_c_restore(void); void state_d_restore(void);

void state_restore_all(void)
{
    sys_info_restore();
    gen_state_restore();
    state_c_restore();
    state_d_restore();
}
