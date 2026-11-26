/*
 * gen_output_calibrate -- stock 0x4004EF10 (468 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Generator output calibration: backs up the state, re-initialises the generator block, sets frequency 1000 and period 2000, fills the second table (G+0xFB6) with 0x1FFE, time-base index 0x18, then measures three DAC settings (G+0x2F38 / +0x2F3C = 0x206C / 0x22C4, 0x206C / 0x1068 and 1000 / 0xE42, range byte SYS+0x41 = 4, 4, 8): outputs_apply, gen_fpga_settings_write, 100 ms, acquisition_dispatcher, then the 64-bit SYS+0x8F8 / 1000 (a, b, c). Coefficients k0 = (4700 (b - c) + 550 (b - a)) / 34310000, k1 = (a - b + 0 x k0) / 4700, k2 = a - 8300 k0 - 8900 k1, stored as floats at G+0x2F64..+0x2F6C and in the backup copy 0x42E428F4 + 0x2F64.., then settings_g_save, state_restore_all, gen_update; touch byte +0x16 is 0 during the run and 1 after. What the constants mean (resistor values?) is OPEN.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "acquisition.h"
#include "drivers/timer.h"
#include "fpga.h"
#include "generator.h"
#include "storage/settings.h"
#include "system.h"
#include "util.h"
#include "sys_info.h"
#include "state/gen_block.h"
#include "shared/access.h"

#define SYS 0x40544E68u
#define GEN 0x42E3F984u

static double measure(u32 a, u32 b, u32 range)
{
    GENS(f_2f38) = a;
    GENS(f_2f3c) = b;
    SI(ch1_vdiv_index) = (unsigned char)range;
    outputs_apply();
    gen_fpga_settings_write();
    delay_ms(100);
    acquisition_dispatcher();
    return (double)(int)(*(volatile long long *)(SYS + 0x8F8) / 1000);
}

void gen_output_calibrate(void)
{
    double a, b, c, k0, k1, k2, d;
    float f0, f1, f2;
    state_backup_all();
    generator_state_init();
    GENS(frequency) = 1000;
    GENS(period_len) = 2000;
    SI(timebase_index) = 0x18;
    SI(f_54f) = 1;
    GENS(f_2f60) = 1;
    g_touch->enabled = 0;
    fpga_cmd63_write();
    fill_u16((unsigned short *)(GEN + 0xFB6), 0x1FFE, GENS(period_len));
    a = measure(0x206C, 0x22C4, 4);
    b = measure(0x206C, 0x1068, 4);
    c = measure(1000, 0xE42, 8);
    k0 = ((b - c) * 4700.0 + (b - a) * 550.0) / 34310000.0;
    d = (a - b) + k0 * 0.0;
    k1 = d / 4700.0;
    k2 = a - k0 * 8300.0;
    k2 = k2 - k1 * 8900.0;
    f0 = (float)k0;
    f1 = (float)k1;
    f2 = (float)k2;
    F32(0x42E428F4u + 0x2F64) = f0;
    F32(0x42E428F4u + 0x2F68) = f1;
    F32(0x42E428F4u + 0x2F6C) = f2;
    GENS(rec_g_2) = f2;
    GENS(rec_g_1) = f1;
    GENS(rec_g_0) = f0;
    settings_g_save();
    state_restore_all();
    gen_update();
    g_touch->enabled = 1;
}
