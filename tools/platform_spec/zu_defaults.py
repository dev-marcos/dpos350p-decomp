# Defaults and calibration: the acquisition defaults reset (17 callers), the power-on defaults of
# SYS_INFO and of the plot / UI blocks, the sync of five FPGA parameters with their cached copies,
# and the generator output calibration (three DAC settings measured, three float coefficients).
# exec'd by gen_platform.py (do not define HDR, SPEC, OUT or f here)


def wu(addr, size, name, desc, body, src="init"):
    f(addr, size, name, src, desc, '#include "hw/fpga.h"' + "\n\n" + body.strip("\n"))


COM = """
#define SYS 0x40544E68u
#define GEN 0x42E3F984u
#define TOUCH 0x42E68BB0u
#define B8(a) (*(volatile unsigned char *)(a))
#define H16(a) (*(volatile unsigned short *)(a))
#define W32(a) (*(volatile u32 *)(a))
#define D64(a) (*(volatile unsigned long long *)(a))
"""

wu(0x4003EB40, 488, "acq_defaults_reset",
   "Resets the acquisition state in SYS_INFO to its defaults and ends with runtime_state_commit_force (tail call). Notable values: time-base index SYS+0x588 = 0x17 and +0x598 = 0x16, the 64-bit words +0x570 / +0x578 = 0, "
   "range bytes +0x41 / +0x105 = 6, coupling bytes +0x46 / +0x10A = 0, the trigger level copies +0x27E, +0x60 and +0x120 = u16 +0xE, the cursor words +0x808 = u16 +8 / 5, +0x80C = 4 x (u16 +8) / 5, +0x810 = u16 +0xA / 4, +0x814 = 3 x (u16 +0xA) / 4, "
   "the saved-trace state SYS+0x3DA52C (byte 0, u16 +4 = 0, +8 = u16 +8 / 3, +0xC = 2 x (u16 +8) / 3), key repeat counters +0x86A..+0x870 = 0, 0, 1, 1; touch byte +0x16 = 1.", COM + """
void runtime_state_commit_force(void);

void acq_defaults_reset(void)
{
    u32 lvl = H16(SYS + 0xE), w8, wa, t;
    W32(SYS + 0x5F0) = 0;
    B8(SYS + 0x588) = 0x17;
    B8(SYS + 0x598) = 0x16;
    W32(SYS + 0x570) = 0;
    W32(SYS + 0x574) = 0;
    D64(SYS + 0x578) = 0;
    B8(SYS + 0x3C) = 0;
    B8(SYS + 0x2AA) = 0;
    B8(SYS + 0x3E) = 1;
    H16(SYS + 0x27E) = (unsigned short)lvl;
    B8(SYS + 0x42) = 0;
    B8(SYS + 0x46) = 0;
    B8(SYS + 0x3F) = 0;
    B8(SYS + 0x41) = 6;
    W32(SYS + 0x60) = lvl;
    B8(SYS + 0x102) = 0;
    B8(SYS + 0x106) = 0;
    B8(SYS + 0x10A) = 0;
    B8(SYS + 0x103) = 0;
    B8(SYS + 0x105) = 6;
    W32(SYS + 0x120) = lvl;
    B8(SYS + 0x274) = 1;
    B8(SYS + 0x275) = 1;
    B8(SYS + 0x27A) = 0;
    B8(SYS + 0x27C) = 0;
    B8(SYS + 0x27B) = 0;
    B8(SYS + 0x284) = 0;
    W32(SYS + 0x64C) = 0;
    B8(SYS + 0x70E) = 0;
    B8(SYS + 0x54F) = 0;
    B8(SYS + 0x54E) = 0;
    B8(SYS + 0x654) = 0x32;
    B8(SYS + 0x672) = 0x14;
    B8(SYS + 0x656) = 0x1E;
    w8 = H16(SYS + 8);
    B8(SYS + 0x673) = 2;
    B8(SYS + 0x3DA52C) = 0;
    H16(SYS + 0x3DA530) = 0;
    W32(SYS + 0x3DA534) = (w8 * 0xAAABu) >> 17;
    W32(SYS + 0x3DA538) = (u32)(((long long)(int)(w8 * 2) * 0x55555556LL) >> 32);
    B8(SYS + 0x2A) = 0;
    B8(SYS + 0x28) = 0;
    B8(SYS + 0x101) = 1;
    H16(SYS + 0x3A) = 0;
    W32(SYS + 0x808) = (w8 * 0xCCCDu) >> 18;
    wa = H16(SYS + 0xA);
    W32(SYS + 0x710) = 0;
    B8(SYS + 0x653) = 0;
    B8(SYS + 0x655) = 1;
    B8(SYS + 0x660) = 0;
    W32(SYS + 0x810) = wa >> 2;
    B8(SYS + 0x650) = 1;
    B8(SYS + 0x651) = 1;
    t = 3 * wa;
    H16(SYS + 0x86A) = 0;
    H16(SYS + 0x86C) = 0;
    H16(SYS + 0x86E) = 1;
    H16(SYS + 0x870) = 1;
    W32(SYS + 0x874) = 0;
    B8(SYS + 0x580) = 1;
    B8(SYS + 0x70D) = 1;
    H16(SYS + 0x586) = 0;
    H16(SYS + 0x664) = 0;
    H16(SYS + 0x666) = 0;
    B8(SYS + 0x671) = 0;
    B8(SYS + 0x652) = 0;
    B8(SYS + 0x77A) = 1;
    W32(SYS + 0x658) = 0;
    W32(SYS + 0x780) = 0;
    W32(SYS + 0x784) = 0;
    W32(SYS + 0x788) = 0;
    B8(SYS + 0x802) = 0;
    B8(SYS + 0x803) = 0;
    W32(SYS + 0x80C) = (u32)(((long long)(int)(w8 * 4) * 0x66666667LL) >> 32) >> 1;
    W32(SYS + 0x814) = (t + ((t >> 1) >> 30)) >> 2;
    B8(SYS + 0x804) = 0;
    W32(SYS + 0x878) = 0;
    B8(TOUCH + 0x16) = 1;
    runtime_state_commit_force();
}
""", src="acquisition")

wu(0x4003E738, 1032, "system_defaults_init",
   "Power-on defaults: 135 constant stores into SYS_INFO (screen 950 x 503 at u16 +8 / +0xA, trigger level u16 +0xE = 251, sample shift word +0x1BC = 6, the marker word SYS+0x77C = 0xA55A5AA5, ...), "
   "into the generator settings at 0x42E3F984 + 0x2F10..+0x2F5C (limits such as 50000000 at +0x2F14, DAC values 0x206C / 0x1F40), the Bode plot block 0x42E180C8 (x 70, y 48, 867 x 480, decade 289 px, 3 decades, rows of 80 px, 6 rows), the UI block 0x42E68998 and the backup block 0x42E68668 "
   "(double 10.0 at +0x50). Nothing is read except the values just written. The list was taken from an emulator trace of the stock code and is checked by the EQUIVALENT test.", COM + """
void system_defaults_init(void)
{
    H16(SYS + 0x1C) = 0x5F;
    H16(SYS + 0x4) = 0x0;
    H16(SYS + 0x8) = 0x3B6;
    H16(SYS + 0x1E) = 0xA;
    H16(SYS + 0xA) = 0x1F7;
    H16(SYS + 0x20) = 0x3F;
    H16(SYS + 0x22) = 0x8;
    B8(SYS + 0x2A8) = 0xC;
    H16(SYS + 0xE) = 0xFB;
    B8(SYS + 0x2A9) = 0x21;
    W32(SYS + 0x524) = 0x8;
    W32(SYS + 0x554) = 0x2710;
    W32(SYS + 0x0) = 0x1EE893;
    H16(SYS + 0xC) = 0x1DB;
    H16(SYS + 0x6) = 0x30;
    B8(SYS + 0x628) = 0x2;
    B8(SYS + 0x1B2) = 0x0;
    H16(SYS + 0x1B6) = 0x0;
    H16(SYS + 0x1B4) = 0x2710;
    B8(SYS + 0x1B9) = 0xC;
    B8(SYS + 0x1BA) = 0xD;
    B8(SYS + 0x1B8) = 0x14;
    W32(SYS + 0x62C) = 0xC8;
    H16(SYS + 0x2AC) = 0x19;
    B8(SYS + 0x2A5) = 0x2A;
    B8(SYS + 0x2A7) = 0x2A;
    B8(SYS + 0x2A6) = 0xB;
    B8(SYS + 0x2A4) = 0x20;
    W32(SYS + 0x1C0) = 0x40;
    W32(SYS + 0x268) = 0x40;
    H16(SYS + 0x630) = 0x10;
    H16(SYS + 0x632) = 0x9;
    W32(SYS + 0x77C) = 0xA55A5AA5;
    H16(SYS + 0x754) = 0x64;
    W32(SYS + 0x520) = 0x17FC;
    H16(SYS + 0x6F2) = 0x3;
    W32(SYS + 0x7F8) = 0x800;
    H16(SYS + 0x756) = 0x12C;
    H16(SYS + 0x758) = 0x1F4;
    H16(SYS + 0x1AE) = 0x4;
    W32(SYS + 0x540) = 0x32;
    H16(SYS + 0x642) = 0x1F40;
    W32(SYS + 0x1BC) = 0x6;
    W32(SYS + 0x264) = 0x6;
    B8(SYS + 0x48) = 0x2;
    B8(SYS + 0x10C) = 0x2;
    W32(GEN + 0x2F34) = 0x7D0;
    W32(GEN + 0x2F24) = 0xBB8;
    W32(GEN + 0x2F28) = 0xFFFFF448;
    W32(GEN + 0x2F10) = 0xE;
    W32(GEN + 0x2F14) = 0x2FAF080;
    W32(GEN + 0x2F18) = 0x1388;
    W32(GEN + 0x2F1C) = 0x9C4;
    W32(GEN + 0x2F20) = 0x3E7;
    H16(GEN + 0xFAE) = 0x230;
    H16(GEN + 0xFB0) = 0x5F;
    W32(GEN + 0x2F48) = 0x2000;
    H16(GEN + 0x2F46) = 0x32;
    W32(GEN + 0x2F50) = 0x14A;
    W32(GEN + 0x2F54) = 0x206C;
    W32(GEN + 0x2F58) = 0x1F40;
    W32(GEN + 0x2F4C) = 0x38DA740E;
    W32(GEN + 0x2F5C) = 0x7735940;
    H16(GEN + 0xFB4) = 0x5;
    W32(SYS + 0x26C) = 0x64;
    W32(SYS + 0x3DA540) = 0x3E;
    B8(SYS + 0x6EC) = 0xB;
    B8(SYS + 0x6ED) = 0x1F;
    W32(SYS + 0x270) = 0x3E8;
    B8(SYS + 0x8E4) = 0x18;
    H16(SYS + 0x868) = 0x50;
    W32(SYS + 0x87C) = 0xC8;
    W32(SYS + 0x3DA544) = 0x3B6;
    H16(SYS + 0xF9A) = 0xF;
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
    B8(SYS + 0x7C0) = 0x1;
    W32(SYS + 0x7E0) = 0x1FB000;
    W32(SYS + 0x7C8) = 0x1FA000;
    W32(SYS + 0x7CC) = 0x1F9000;
    W32(SYS + 0x7D0) = 0x1F8000;
    W32(SYS + 0x7D4) = 0x1F7000;
    W32(SYS + 0x7D8) = 0x1F6000;
    W32(SYS + 0x7DC) = 0x1F5000;
    W32(SYS + 0x7E4) = 0x1F4000;
    W32(SYS + 0x7C4) = 0x1F3000;
    W32(SYS + 0x7E8) = 0x1F2000;
    W32(SYS + 0x7EC) = 0x1F0000;
    W32(SYS + 0x7F0) = 0x1EC000;
    W32(SYS + 0x7F4) = 0x1EB000;
    W32(SYS + 0x1C8) = 0x8F0;
    W32(SYS + 0x1C4) = 0x8F0;
    W32(SYS + 0x1F4) = 0x896;
    W32(SYS + 0x1F0) = 0x896;
    W32(SYS + 0x1EC) = 0x8D9;
    W32(SYS + 0x1E8) = 0x8B5;
    W32(SYS + 0x1E4) = 0x8BB;
    W32(SYS + 0x1E0) = 0x8F6;
    W32(SYS + 0x1DC) = 0x8F6;
    W32(SYS + 0x1D8) = 0x8A2;
    W32(SYS + 0x1D4) = 0x8DA;
    W32(SYS + 0x1D0) = 0x8BB;
    W32(SYS + 0x1CC) = 0x8B6;
    W32(SYS + 0x244) = 0x896;
    W32(SYS + 0x240) = 0x896;
    W32(SYS + 0x23C) = 0x8D9;
    W32(SYS + 0x238) = 0x8B5;
    W32(SYS + 0x234) = 0x8BB;
    W32(SYS + 0x230) = 0x8F6;
    W32(SYS + 0x22C) = 0x8F6;
    W32(SYS + 0x228) = 0x8A2;
    W32(SYS + 0x224) = 0x8DA;
    W32(SYS + 0x220) = 0x8BB;
    W32(SYS + 0x21C) = 0x8B6;
    W32(SYS + 0x218) = 0x8F0;
    W32(SYS + 0x214) = 0x8F0;
}
""")

wu(0x4003F160, 404, "fpga_params_6e4_sync",
   "Pushes the FPGA parameters SYS+0x6E4..+0x6E8 that differ from their cached copies SYS+0xBB0..+0xBB4, updating the copy first: bytes +0x6E4 / +0x6E5 / +0x6E6 each go through fpga_param_updater(0x21 / 0x22 / 0x23, 1, the unaligned word at the copy, the byte), smec_commit(0xC1 / 0xC2 / 0xC3) "
   "and the FPGA selector 4 / 5 / 6 with the byte; byte +0x6E7 only through selector 2; the word +0x6E8 through selector 0xD as four bytes, high first. When anything was sent: selector 7 with 1, then 0 (a strobe), then delay_ms(2). The word change alone sends the strobe too.", COM + """
void smec_commit(u32 id);
void delay_ms(u32 ms);
typedef u32 u32_unaligned __attribute__((aligned(1)));
#define WU(a) (*(volatile u32_unaligned *)(a))

void fpga_params_6e4_sync(void)
{
    int sent = 0, i;
    u32 v;
    for (i = 0; i < 3; i++) {
        v = B8(SYS + 0x6E4 + i);
        if (B8(SYS + 0xBB0 + i) != v) {
            B8(SYS + 0xBB0 + i) = (unsigned char)v;
            sent = 1;
            fpga_param_updater(0x21 + i, 1, WU(SYS + 0xBB0 + i), v);
            smec_commit(0xC1 + i);
            fpga_selector_command_write(4 + i);
            fpga_data_byte_write(B8(SYS + 0xBB0 + i));
        }
    }
    v = B8(SYS + 0x6E7);
    if (B8(SYS + 0xBB3) != v) {
        B8(SYS + 0xBB3) = (unsigned char)v;
        fpga_selector_command_write(2);
        sent = 1;
        fpga_data_byte_write(B8(SYS + 0xBB3));
    }
    v = W32(SYS + 0x6E8);
    if (W32(SYS + 0xBB4) != v) {
        W32(SYS + 0xBB4) = v;
        fpga_selector_command_write(0xD);
        fpga_data_byte_write(W32(SYS + 0xBB4) >> 24);
        fpga_data_byte_write((W32(SYS + 0xBB4) << 8) >> 24);
        fpga_data_byte_write((W32(SYS + 0xBB4) << 16) >> 24);
        fpga_data_byte_write(W32(SYS + 0xBB4) & 0xFF);
    } else if (!sent)
        return;
    fpga_selector_command_write(7);
    fpga_data_byte_write(1);
    fpga_selector_command_write(7);
    fpga_data_byte_write(0);
    delay_ms(2);
}
""", src="acquisition")

wu(0x4004EF10, 468, "gen_output_calibrate",
   "Generator output calibration: backs up the state, re-initialises the generator block, sets frequency 1000 and period 2000, fills the second table (G+0xFB6) with 0x1FFE, time-base index 0x18, then measures three DAC settings (G+0x2F38 / +0x2F3C = 0x206C / 0x22C4, 0x206C / 0x1068 and 1000 / 0xE42, "
   "range byte SYS+0x41 = 4, 4, 8): outputs_apply, gen_fpga_settings_write, 100 ms, acquisition_dispatcher, then the 64-bit SYS+0x8F8 / 1000 (a, b, c). Coefficients k0 = (4700 (b - c) + 550 (b - a)) / 34310000, k1 = (a - b + 0 x k0) / 4700, k2 = a - 8300 k0 - 8900 k1, "
   "stored as floats at G+0x2F64..+0x2F6C and in the backup copy 0x42E428F4 + 0x2F64.., then settings_g_save, state_restore_all, gen_update; touch byte +0x16 is 0 during the run and 1 after. What the constants mean (resistor values?) is OPEN.", COM + """
void state_backup_all(void);
void state_block_3f984_init(void);
void fpga_cmd63_write(void);
void fill_u16(unsigned short *d, unsigned short v, u32 n);
void outputs_apply(void);
void gen_fpga_settings_write(void);
void delay_ms(u32 ms);
void acquisition_dispatcher(void);
void settings_g_save(void);
void state_restore_all(void);
void gen_update(void);
#define F32(a) (*(volatile float *)(a))

static double measure(u32 a, u32 b, u32 range)
{
    W32(GEN + 0x2F38) = a;
    W32(GEN + 0x2F3C) = b;
    B8(SYS + 0x41) = (unsigned char)range;
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
    state_block_3f984_init();
    W32(GEN + 0x2EFC) = 1000;
    H16(GEN + 0x2EF6) = 2000;
    B8(SYS + 0x588) = 0x18;
    B8(SYS + 0x54F) = 1;
    B8(GEN + 0x2F60) = 1;
    B8(TOUCH + 0x16) = 0;
    fpga_cmd63_write();
    fill_u16((unsigned short *)(GEN + 0xFB6), 0x1FFE, H16(GEN + 0x2EF6));
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
    F32(GEN + 0x2F6C) = f2;
    F32(GEN + 0x2F68) = f1;
    F32(GEN + 0x2F64) = f0;
    settings_g_save();
    state_restore_all();
    gen_update();
    B8(TOUCH + 0x16) = 1;
}
""", src="acquisition")

wu(0x40050898, 788, "gen_selftest_50898",
   "Self-test with five generator / capture settings (DERIVED from what it writes): acq_defaults_reset, channels on (SYS+0x3E = 1, +0x102 = 1), coupling bytes +0x46 / +0x10A = 2, SYS+0x660 = 5, touch byte +0x16 = 0. For each step i: waveform byte 0x42E3F984+0x2EFA = 1, "
   "frequency +0x2EFC = 25 / 100 / 1000 / 10000 / 10000, amplitude +0x2F00 = 1200 (3000 in the last step), offset +0x2F04 = 0, time-base SYS+0x588 = 0x20 / 0x1E / 0x1B / 0x18 / 0x18, ranges SYS+0x41 = SYS+0x105 = 6 (5 in the last step); outputs_apply, gen_update, acquisition_dispatcher. "
   "Per channel the peak-to-peak of the capture window (u16 samples at 0x42AC7CF4 / +0x62250 from 0x2E68 + 5 x step for 0x924 - 10 x step samples, step = SYS+0x558) passes when it is at most 10 x word SYS+0x1C0. "
   "The AND of the five results goes to byte 0x42E68AE0+0x35 (A) and +0x36 (B), both together to +0x34.", COM + """
void acq_defaults_reset(void);
void outputs_apply(void);
void gen_update(void);
void acquisition_dispatcher(void);

void gen_selftest_50898(void)
{
    static const u32 freq[5] = {25, 100, 1000, 10000, 10000};
    static const u32 amp[5] = {1200, 1200, 1200, 1200, 3000};
    static const unsigned char tb[5] = {0x20, 0x1E, 0x1B, 0x18, 0x18};
    static const unsigned char rng[5] = {6, 6, 6, 6, 5};
    u32 i, j, step, start, n, lim, ok_a = 1, ok_b = 1;
    int mx, mn, v;
    volatile unsigned short *s;
    acq_defaults_reset();
    B8(SYS + 0x3E) = 1;
    B8(SYS + 0x102) = 1;
    B8(SYS + 0x46) = 2;
    B8(SYS + 0x10A) = 2;
    B8(SYS + 0x660) = 5;
    B8(TOUCH + 0x16) = 0;
    step = W32(SYS + 0x558);
    start = 0x2E68 + 5 * step;
    n = 0x924 - 10 * step;
    for (i = 0; i < 5; i++) {
        B8(GEN + 0x2EFA) = 1;
        W32(GEN + 0x2EFC) = freq[i];
        W32(GEN + 0x2F00) = amp[i];
        W32(GEN + 0x2F04) = 0;
        B8(SYS + 0x588) = tb[i];
        B8(SYS + 0x41) = rng[i];
        B8(SYS + 0x105) = rng[i];
        outputs_apply();
        gen_update();
        acquisition_dispatcher();
        mx = 0;
        mn = 0x0FFFFFFF;
        if ((int)start < (int)(start + n)) {
            s = (volatile unsigned short *)(0x42AC7CF4u + 2 * start);
            for (j = 0; j < n; j++) {
                v = s[j];
                if (v >= mx)
                    mx = v;
                if (v <= mn)
                    mn = v;
            }
        }
        lim = W32(SYS + 0x1C0) * 10;
        ok_a &= lim >= (u32)(mx - mn) ? 1u : 0u;
        mx = 0;
        mn = 0x0FFFFFFF;
        if ((int)start < (int)(start + n)) {
            s = (volatile unsigned short *)(0x42AC7CF4u + 0x62250u + 2 * start);
            for (j = 0; j < n; j++) {
                v = s[j];
                if (v >= mx)
                    mx = v;
                if (v < mn)
                    mn = v;
            }
        }
        lim = W32(SYS + 0x1C0) * 10;
        ok_b &= lim >= (u32)(mx - mn) ? 1u : 0u;
    }
    B8(0x42E68AE0u + 0x35) = (unsigned char)ok_a;
    B8(0x42E68AE0u + 0x36) = (unsigned char)ok_b;
    B8(0x42E68AE0u + 0x34) = (unsigned char)(ok_a & ok_b);
}
""", src="acquisition")
