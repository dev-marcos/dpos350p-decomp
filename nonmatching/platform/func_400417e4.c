/*
 * one_runtime_capture_step -- stock 0x400417E4 (308 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Waits for the FPGA ready flag (selector 0x0B, giving up on a touch), clears SYS16+0x29C/0x29E, applies the time base, captures CH1 (SYS+0x3E) and CH2 (SYS+0x102) by the bank mode SYS+0x100, then copies SYS16+0x6F6 samples of each channel (from the channel buffers SYS+0x7B4 / 0x7B8 at offset SYS+0x704) into the display buffers 0x42AC7CF4 (+0x62250 for CH2) at SYS+0x6FC. (name: Hack)
 */
#include "hw/platform.h"

#include "hw/fpga.h"

void runtime_ready_read(void);
void timebase_apply_2b0(void);
void stream_geometry_fpga_selector_0x22_configuration(void);
void capture_banks_all(void);
void capture_banks_0_3(void);
void capture_banks_4_7(void);
void func_40040d74(u32 buf, u32 ch);
void func_40040e44(u32 buf, u32 ch);
void func_40040bec(u32 buf, u32 ch);

void one_runtime_capture_step(void)
{
    u32 m;
    for (;;) {
        runtime_ready_read();
        if (SYS8(0x6EE) != 0)
            break;
        runtime_event_service();
        if (g_touch->busy2 != 0)
            break;
    }
    SYS16(0x29C) = 0;
    SYS16(0x29E) = 0;
    timebase_apply_2b0();
    if (SYS8(0x3E) != 0) {
        stream_geometry_fpga_selector_0x22_configuration();
        m = SYS8(0x100);
        if (m == 0) {
            capture_banks_0_3();
            func_40040d74(SYS32(0x7B4), 0);
        } else if (m == 1) {
            capture_banks_all();
            func_40040bec(SYS32(0x7B4), 0);
        }
    }
    if (SYS8(0x102) != 0) {
        stream_geometry_fpga_selector_0x22_configuration();
        m = SYS8(0x100);
        if (m == 0) {
            capture_banks_4_7();
            func_40040e44(SYS32(0x7B8), 1);
        } else if (m == 2) {
            capture_banks_all();
            func_40040bec(SYS32(0x7B8), 1);
        }
    }
    copy_u16((unsigned short *)(0x42AC7CF4u + SYS32(0x6FC) * 2), (const unsigned short *)(SYS32(0x7B4) + SYS32(0x704) * 2), SYS16(0x6F6));
    copy_u16((unsigned short *)(0x42AC7CF4u + SYS32(0x6FC) * 2 + 0x62250), (const unsigned short *)(SYS32(0x7B8) + SYS32(0x704) * 2), SYS16(0x6F6));
}
