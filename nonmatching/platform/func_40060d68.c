/*
 * calib_run_4_channels -- stock 0x40060D68 (96 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Block 0x42E68668: +0x190 = 1, +5 = 1; for i = 0..3: +0xF4 = i, then 0x400612FC, 0x40060FE8, 0x40060DC8, 0x40060BA4; then +0x190 = 0, +0xF4 = its old low byte, and settings_c_save.
 */
#include "hw/platform.h"

#include "hw/fpga.h"

#define CB8(o)  (*(volatile unsigned char *)(0x42E68668u + (o)))
#define CB32(o) (*(volatile u32 *)(0x42E68668u + (o)))
void func_400612fc(void);
void func_40060fe8(void);
void func_40060dc8(void);
void func_40060ba4(void);
void settings_c_save(void);

void calib_run_4_channels(void)
{
    u32 old, i;
    CB8(0x190) = 1;
    old = CB32(0xF4);
    CB8(5) = 1;
    for (i = 0; i < 4; i++) {
        CB32(0xF4) = i;
        func_400612fc();
        func_40060fe8();
        func_40060dc8();
        func_40060ba4();
    }
    CB8(0x190) = 0;
    CB32(0xF4) = old & 0xFF;
    settings_c_save();
}
