/*
 * FPGA_Start -- stock 0x40049188 (48 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * CSI set-up, PE pins to FPGA-bus mode, MS5351M clocks. (name: Hack FPGA.md)
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "drivers/csi.h"
#include "drivers/ms5351.h"
#include "fpga.h"

void FPGA_Start(void)
{
    csi_full_init(0x1F40);
    g_pe_mode = 1;
    fpga_pe_mode_select(0);
    ms5351_start_clocks();
}
