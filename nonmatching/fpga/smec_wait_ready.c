/*
 * smec_wait_ready -- stock 0x4004A040 (36 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Waits for selector 0x65 bit 0.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "drivers/fpga_bus.h"
#include "fpga.h"

void smec_wait_ready(void)
{
    do {
        fpga_selector_command_write(0x65);
    } while (!(fpga_data_byte_read() & 1));
}
