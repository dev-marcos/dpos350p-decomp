/*
 * selector_0x69_16_bit_timing_retry -- stock 0x40049FB8 (36 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Job timing, high byte then low byte (selector 0x69). (name: Hack SMEC.md)
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "drivers/fpga_bus.h"
#include "fpga.h"

void selector_0x69_16_bit_timing_retry(u32 v)
{
    fpga_selector_command_write(0x69);
    fpga_data_byte_write(v >> 8);
    fpga_data_byte_write(v & 0xFF);
}
