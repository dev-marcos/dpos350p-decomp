/*
 * runtime_ready_read -- stock 0x4003F8B8 (40 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Selector 0x0B bit 0 into SYS_INFO+0x6EE (Hack: SYS_RUNTIME_READY).
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "acquisition.h"
#include "drivers/fpga_bus.h"

void runtime_ready_read(void)
{
    fpga_selector_command_write(0x0B);
    SYS8(0x6EE) = fpga_data_byte_read() & 1;
}
