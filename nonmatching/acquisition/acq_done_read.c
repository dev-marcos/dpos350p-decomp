/*
 * acq_done_read -- stock 0x4003F9D0 (40 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Selector 0x0A bit 0 into SYS_INFO+0x38 (Hack: SYS_ACQ_DONE).
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "acquisition.h"
#include "drivers/fpga_bus.h"
#include "sys_info.h"

void acq_done_read(void)
{
    fpga_selector_command_write(0x0A);
    SI(acq_done) = fpga_data_byte_read() & 1;
}
