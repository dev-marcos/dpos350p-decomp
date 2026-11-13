/*
 * security_chip_check -- stock 0x40049E00 (48 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Commits job 0xB4, then reads the security-chip status (selector 0x60 bit 0).
 */
#include "hw/platform.h"

#include "hw/fpga.h"

void security_chip_check(void)
{
    g_smec->pending = 1;
    smec_commit(0xB4);
    fpga_selector_command_write(0x60);
    g_smec->secure_ok = fpga_data_byte_read() & 1;
}
