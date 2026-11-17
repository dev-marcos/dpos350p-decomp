/*
 * selftest_4f5e4 -- stock 0x4004F5E4 (668 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Self-test of the eight capture byte lanes (DERIVED): channel 1 only (SYS+0x3E = 1, SYS+0x102 = 0, SYS+0x3A = 1), range 0x18, level 6; s64 SYS+0x30 = 10, SYS+0x640 = 10, SYS+0x644 = 1, stream geometry sent. Then 900 steps of the FPGA 0x2A value SYS+0x44 = (int)(3000.0f + i * 7.7777777f): each sends it, prepares and runs one capture and copies byte 0x42DF76D4 + k * 0x2134 of the eight lanes k to 0x42E51EB4 + k * 1000 + i. A lane passes (byte 0x42E68AF9 + k = 1) when, over its bytes 10..889, every bit is seen both set and clear and every pair of neighbouring bits is seen different; byte 0x42E68AF8 = 1 when all eight pass.
 */
#include "hw/platform.h"

#include "hw/fpga.h"

void func_4003eb40(void);
void outputs_apply(void);
void stream_geometry_fpga_selector_0x22_configuration(void);
void fpga_write_sel_0x2a(void);
void delay_ms(u32 ms);
void acquisition_prepare(void);
void trigger_pos_update(void);
void capture_banks_all(void);
#define RS8(o) (*(volatile unsigned char *)(0x42E68AE0u + (o)))
#define LANE8(o) (*(volatile unsigned char *)(0x42E51EB4u + (o)))

void selftest_4f5e4(void)
{
    unsigned char set[8], clr[8], edge[8];
    u32 k, i, b, ok;
    int s;
    for (k = 0; k < 8; k++) {
        set[k] = clr[k] = edge[k] = 0;
        RS8(0x19 + k) = 0;
    }
    func_4003eb40();
    SYS8(0x588) = 0x18;
    SYS8(0x3E) = 1;
    SYS8(0x102) = 0;
    SYS8(0x41) = 6;
    SYS8(0x3A) = 1;
    *(volatile unsigned char *)(0x42E68BB0u + 0x16) = 0;
    outputs_apply();
    *(volatile long long *)(0x40544E68u + 0x30) = 10;
    SYS16(0x640) = 10;
    SYS16(0x644) = 1;
    stream_geometry_fpga_selector_0x22_configuration();
    SYS16(0x44) = 0xBB8;
    fpga_write_sel_0x2a();
    delay_ms(200);
    for (s = 0; s < 900; s++) {
        SYS16(0x44) = (int)(3000.0f + (float)s * 7.7777777f);
        fpga_write_sel_0x2a();
        acquisition_prepare();
        SYS32(0x63C) = 0;
        trigger_pos_update();
        capture_banks_all();
        for (k = 0; k < 8; k++)
            LANE8(k * 1000 + s) = *(volatile unsigned char *)(0x42DF76D4u + k * 0x2134);
    }
    for (k = 0; k < 8; k++)
        for (b = 0; b < 8; b++)
            for (i = 0; i < 880; i++) {
                if (LANE8(k * 1000 + 10 + i) & (1u << b))
                    set[k] |= 1u << b;
                else
                    clr[k] |= 1u << b;
            }
    for (k = 0; k < 8; k++)
        for (i = 0; i < 880; i++) {
            u32 x = LANE8(k * 1000 + 10 + i);
            for (b = 0; b <= 6; b++) {
                u32 m = (1u << b) & 0xFF, n = (1u << (b + 1)) & 0xFF;
                if (((x & m) != 0) != ((x & n) != 0)) {
                    u32 e = edge[k] | m;
                    if (b == 6)
                        e |= n;
                    edge[k] = e;
                }
            }
        }
    for (k = 0; k < 8; k++)
        if ((unsigned char)(edge[k] & set[k] & clr[k]) == 0xFF)
            RS8(0x19 + k) = 1;
    ok = 1;
    for (k = 0; k < 8; k++) {
        RS8(0x18) = ok;
        ok &= RS8(0x19 + k);
    }
    RS8(0x18) = ok;
}
