/*
 * system_defaults_init -- stock 0x4003E738 (1032 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Power-on defaults: 135 constant stores into SYS_INFO (screen 950 x 503 at u16 +8 / +0xA, trigger level u16 +0xE = 251, sample shift word +0x1BC = 6, the marker word SYS+0x77C = 0xA55A5AA5, ...), into the generator settings at 0x42E3F984 + 0x2F10..+0x2F5C (limits such as 50000000 at +0x2F14, DAC values 0x206C / 0x1F40), the Bode plot block 0x42E180C8 (x 70, y 48, 867 x 480, decade 289 px, 3 decades, rows of 80 px, 6 rows), the UI block 0x42E68998 and the backup block 0x42E68668 (double 10.0 at +0x50). Nothing is read except the values just written. The list was taken from an emulator trace of the stock code and is checked by the EQUIVALENT test.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "system.h"
#include "sys_info.h"
#include "state/gen_block.h"

#define SYS 0x40544E68u
#define H16(a) (*(volatile unsigned short *)(a))
#define W32(a) (*(volatile u32 *)(a))
#define D64(a) (*(volatile unsigned long long *)(a))

void system_defaults_init(void)
{
    SI(f_01c) = 0x5F;
    SI(plot_x0) = 0x0;
    SI(plot_width) = 0x3B6;
    SI(f_01e) = 0xA;
    SI(plot_height) = 0x1F7;
    SI(grid_step) = 0x3F;
    SI(f_022) = 0x8;
    SI(timebase_index_min) = 0xC;
    SI(level_default) = 0xFB;
    SI(slow_timebase_index) = 0x21;
    SI(sample_rate) = 0x8;
    SI(f_554) = 0x2710;
    SI(f_000) = 0x1EE893;
    SI(plot_x_mid) = 0x1DB;
    SI(plot_y0) = 0x30;
    SI(f_628) = 0x2;
    SI(f_1b2) = 0x0;
    SI(f_1b6) = 0x0;
    SI(f_1b4) = 0x2710;
    SI(f_1b9) = 0xC;
    SI(f_1ba) = 0xD;
    SI(f_1b8) = 0x14;
    SI(acq_geometry_62c) = 0xC8;
    SI(f_2ac) = 0x19;
    SI(f_2a5) = 0x2A;
    SI(timebase_index_max) = 0x2A;
    SI(f_2a6) = 0xB;
    SI(f_2a4) = 0x20;
    SI(acq_geometry_1c0) = 0x40;
    SI(acq_geometry_268) = 0x40;
    SI(f_630) = 0x10;
    SI(f_632) = 0x9;
    SI(f_77c) = 0xA55A5AA5;
    SI(f_754) = 0x64;
    SI(record_length) = 0x17FC;
    SI(runtime_position_base) = 0x3;
    SI(fft_size) = 0x800;
    SI(f_756) = 0x12C;
    SI(capture_slot_count) = 0x1F4;
    SI(f_1ae) = 0x4;
    SI(f_540) = 0x32;
    SI(f_642) = 0x1F40;
    SI(sample_shift) = 0x6;
    SI(raw_shift) = 0x6;
    SI(f_048) = 0x2;
    SI(f_10c) = 0x2;
    GENS(f_2f34) = 0x7D0;
    GENS(f_2f24) = 0xBB8;
    GENS(f_2f28) = 0xFFFFF448;
    GENS(f_2f10) = 0xE;
    GENS(f_2f14) = 0x2FAF080;
    GENS(f_2f18) = 0x1388;
    GENS(f_2f1c) = 0x9C4;
    GENS(f_2f20) = 0x3E7;
    GENS(f_fae) = 0x230;
    GENS(f_fb0) = 0x5F;
    GENS(f_2f48) = 0x2000;
    GENS(f_2f46) = 0x32;
    GENS(f_2f50) = 0x14A;
    GENS(f_2f54) = 0x206C;
    GENS(f_2f58) = 0x1F40;
    GENS(f_2f4c) = 0x38DA740E;
    GENS(f_2f5c) = 0x7735940;
    GENS(f_fb4) = 0x5;
    SI(acq_geometry_26c) = 0x64;
    SI(cursor_gap) = 0x3E;
    SI(f_6ec) = 0xB;
    SI(f_6ed) = 0x1F;
    SI(acq_geometry_270) = 0x3E8;
    SI(f_8e4) = 0x18;
    SI(f_868) = 0x50;
    SI(f_87c) = 0xC8;
    SI(capture_cells) = 0x3B6;
    SI(list_page_size) = 0xF;
    W32(0x42E6899Cu) = 0x32;
    W32(0x42E689F8u) = 0x46;
    W32(0x42E68A0Cu) = 0x46;
    H16(0x42E180C8u) = 0x46;
    H16(0x42E180CCu) = 0x363;
    H16(0x42E180D0u) = 0x121;
    H16(0x42E180CAu) = 0x30;
    H16(0x42E180CEu) = 0x1E0;
    H16(0x42E180D2u) = 0x3;
    H16(0x42E180D4u) = 0x50;
    H16(0x42E180D6u) = 0x6;
    W32(0x42E6871Cu) = 0x30;
    W32(0x42E68728u) = 0x1E0;
    W32(0x42E68718u) = 0x48;
    W32(0x42E68720u) = 0x3B3;
    W32(0x42E6872Cu) = 0x9D;
    W32(0x42E68730u) = 0x6;
    W32(0x42E68734u) = 0x50;
    W32(0x42E68738u) = 0x6;
    W32(0x42E687ECu) = 0x2;
    D64(0x42E686B8u) = 0x4024000000000000ULL;
    SI(settings_slot) = 0x1;
    SI(calibration_flash_addr) = 0x1FB000;
    SI(f_7c8) = 0x1FA000;
    SI(f_7cc) = 0x1F9000;
    SI(f_7d0) = 0x1F8000;
    SI(f_7d4) = 0x1F7000;
    SI(f_7d8) = 0x1F6000;
    SI(rec_e_flash_addr) = 0x1F5000;
    SI(rec_b_flash_addr) = 0x1F4000;
    SI(rec_f_flash_addr) = 0x1F3000;
    SI(rec_g_flash_addr) = 0x1F2000;
    SI(rec_d_flash_addr) = 0x1F0000;
    SI(rec_c_flash_addr) = 0x1EC000;
    SI(rec_a_flash_addr) = 0x1EB000;
    SI(f_1c8) = 0x8F0;
    SI(ch1_gain_table) = 0x8F0;
    SI(f_1f4) = 0x896;
    SI(f_1f0) = 0x896;
    SI(f_1ec) = 0x8D9;
    SI(f_1e8) = 0x8B5;
    SI(f_1e4) = 0x8BB;
    SI(f_1e0) = 0x8F6;
    SI(f_1dc) = 0x8F6;
    SI(f_1d8) = 0x8A2;
    SI(f_1d4) = 0x8DA;
    SI(f_1d0) = 0x8BB;
    SI(f_1cc) = 0x8B6;
    SI(f_244) = 0x896;
    SI(f_240) = 0x896;
    SI(f_23c) = 0x8D9;
    SI(f_238) = 0x8B5;
    SI(f_234) = 0x8BB;
    SI(f_230) = 0x8F6;
    SI(f_22c) = 0x8F6;
    SI(f_228) = 0x8A2;
    SI(f_224) = 0x8DA;
    SI(f_220) = 0x8BB;
    SI(f_21c) = 0x8B6;
    SI(f_218) = 0x8F0;
    SI(ch2_gain_table) = 0x8F0;
}
