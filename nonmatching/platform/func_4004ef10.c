/*
 * gen_output_calibrate -- stock 0x4004EF10 (468 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Generator output calibration: backs up the state, re-initialises the generator block, sets frequency 1000 and period 2000, fills the second table (G+0xFB6) with 0x1FFE, time-base index 0x18, then measures three DAC settings (G+0x2F38 / +0x2F3C = 0x206C / 0x22C4, 0x206C / 0x1068 and 1000 / 0xE42, range byte SYS+0x41 = 4, 4, 8): outputs_apply, gen_fpga_settings_write, 100 ms, acquisition_dispatcher, then the 64-bit SYS+0x8F8 / 1000 (a, b, c). Coefficients k0 = (4700 (b - c) + 550 (b - a)) / 34310000, k1 = (a - b + 0 x k0) / 4700, k2 = a - 8300 k0 - 8900 k1, stored as floats at G+0x2F64..+0x2F6C and in the backup copy 0x42E428F4 + 0x2F64.., then settings_g_save, state_restore_all, gen_update; touch byte +0x16 is 0 during the run and 1 after. What the constants mean (resistor values?) is OPEN.
 */
#include "hw/platform.h"

#include "hw/fpga.h"

#define SYS 0x40544E68u
#define GEN 0x42E3F984u
#define TOUCH 0x42E68BB0u
#define B8(a) (*(volatile unsigned char *)(a))
#define H16(a) (*(volatile unsigned short *)(a))
#define W32(a) (*(volatile u32 *)(a))
#define D64(a) (*(volatile unsigned long long *)(a))

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
