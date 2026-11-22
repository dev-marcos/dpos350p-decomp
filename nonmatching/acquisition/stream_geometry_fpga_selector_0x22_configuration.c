/*
 * stream_geometry_fpga_selector_0x22_configuration -- stock 0x400405E0 (88 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Stream geometry: csi_init_structs(SYS16+0x640 + 2 x SYS16+0x6F2), then selector 0x22 <- 0x14 + 2 x (SYS16+0x640 + SYS16+0x6F2), 16 bit, high byte first. (name: Hack)
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "acquisition.h"
#include "drivers/csi.h"
#include "drivers/fpga_bus.h"

void stream_geometry_fpga_selector_0x22_configuration(void)
{
    u32 w;
    csi_init_structs((SYS16(0x640) + 2u * SYS16(0x6F2)) & 0xFFFF);
    w = 0x14 + 2u * (SYS16(0x6F2) + SYS16(0x640));
    fpga_selector_command_write(0x22);
    fpga_data_byte_write((w << 16) >> 24);
    fpga_data_byte_write(w & 0xFF);
}
