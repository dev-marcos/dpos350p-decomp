/*
 * fpga_pe_mode_select -- stock 0x40049094 (36 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Shared PE pins: 0 = FPGA control bus (GPIO), other = CSI sample capture. Reprograms the pins only when the mode changes (0x42E68D19).
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "drivers/csi.h"
#include "drivers/fpga_bus.h"
#include "fpga.h"

void fpga_pe_mode_select(u32 mode)
{
    if (mode == g_pe_mode)
        return;
    g_pe_mode = mode;
    if (mode == 0)
        fpga_bus_init();
    else
        csi_pins_init_0();
}
