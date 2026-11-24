/*
 * gen_fpga_settings_write -- stock 0x4004ED18 (504 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Generator settings to the FPGA, skipped while byte 0x42E68BC7 is set. Block G = 0x42E3F984: clock = min(word 0x42E428E0, G+0x2EFC * point count G+0x2EF6); below 2500 it is multiplied by 10 (at least 2500) with the x10 flag G+0x2F0C = 1, else the flag is 0; stored in G+0x2F40 and sent to the MS5351 clock 1. Then FPGA selectors 0x31 / 0x32 = bits 8..15 and 0..7 of G+0x2F38 / G+0x2F3C, 0x3C = the x10 flag, 0x3D = 1, 0x41 = count - 2 (two bytes), 0x44 = (G+0x2EFA != 0); for each point i: 0x3F = i, 0x40 = sample halfword G+0xFB6 + 2i, 0x3E = 0 then 1 (strobe); then fpga_sel42_43_fill(low byte of G+0x2F46) and 0x3D = 0.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "drivers/fpga_bus.h"
#include "drivers/ms5351.h"
#include "fpga.h"
#include "generator.h"
#include "state/gen_block.h"

void gen_fpga_settings_write(void)
{
    u32 f, c, i;
    volatile unsigned short *smp;
    if (g_touch->serviced)
        return;
    f = GENS(period_len) * GENS(frequency);
    c = GENS(f_2f5c);
    if (c > f)
        c = f;
    if (c < 2500) {
        GENS(f_2f0c) = 1;
        c *= 10;
        if (c < 2500)
            c = 2500;
    } else {
        GENS(f_2f0c) = 0;
    }
    GENS(f_2f40) = c;
    ms5351_set_clk1(c);
    fpga_selector_command_write(0x31);
    fpga_data_byte_write((GENS(f_2f38) >> 8) & 0xFF);
    fpga_data_byte_write(GENS(f_2f38) & 0xFF);
    fpga_selector_command_write(0x32);
    fpga_data_byte_write((GENS(f_2f3c) >> 8) & 0xFF);
    fpga_data_byte_write(GENS(f_2f3c) & 0xFF);
    fpga_selector_command_write(0x3C);
    fpga_data_byte_write(GENS(f_2f0c));
    fpga_selector_command_write(0x3D);
    fpga_data_byte_write(1);
    fpga_selector_command_write(0x41);
    fpga_data_byte_write(((GENS(period_len) - 2) >> 8) & 0xFF);
    fpga_data_byte_write((GENS(period_len) - 2) & 0xFF);
    fpga_selector_command_write(0x44);
    fpga_data_byte_write(GENS(waveform) != 0);
    smp = (volatile unsigned short *)(0x42E3F984u + 0xFB6);
    for (i = 0; i < GENS(period_len); i++) {
        fpga_selector_command_write(0x3F);
        fpga_data_byte_write((i >> 8) & 0xFF);
        fpga_data_byte_write(i & 0xFF);
        fpga_selector_command_write(0x40);
        fpga_data_byte_write(*smp >> 8);
        fpga_data_byte_write(*smp & 0xFF);
        smp++;
        fpga_selector_command_write(0x3E);
        fpga_data_byte_write(0);
        fpga_selector_command_write(0x3E);
        fpga_data_byte_write(1);
    }
    fpga_sel42_43_fill(GENS(f_2f46) & 0xFF);
    fpga_selector_command_write(0x3D);
    fpga_data_byte_write(0);
}
