/*
 * one_runtime_capture_step -- stock 0x400417E4 (308 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Waits for the FPGA ready flag (selector 0x0B, giving up on a touch), clears SYS16+0x29C/0x29E, applies the time base, captures CH1 (SYS+0x3E) and CH2 (SYS+0x102) by the bank mode SYS+0x100, then copies SYS16+0x6F6 samples of each channel (from the channel buffers SYS+0x7B4 / 0x7B8 at offset SYS+0x704) into the display buffers 0x42AC7CF4 (+0x62250 for CH2) at SYS+0x6FC. (name: Hack)
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "acquisition.h"
#include "util.h"
#include "sys_info.h"

/* Same functions, declared the way the stock code calls them (see the definitions for the
 * full interface): the C must pass exactly these registers. */
void bank_unpack_0_3_raw(u32 buf, u32 ch) __asm__("bank_unpack_0_3");
void bank_unpack_4_7_raw(u32 buf, u32 ch) __asm__("bank_unpack_4_7");
void bank_unpack_all_raw(u32 buf, u32 ch) __asm__("bank_unpack_all");

void one_runtime_capture_step(void)
{
    u32 m;
    for (;;) {
        runtime_ready_read();
        if (SI(runtime_ready) != 0)
            break;
        runtime_event_service();
        if (g_touch->busy2 != 0)
            break;
    }
    SI(f_29c) = 0;
    SI(f_29e) = 0;
    timebase_apply_2b0();
    if (SI(ch1_enabled) != 0) {
        stream_geometry_fpga_selector_0x22_configuration();
        m = SI(bank_mode);
        if (m == 0) {
            capture_banks_0_3();
            bank_unpack_0_3_raw(SI(ch1_buffer), 0);
        } else if (m == 1) {
            capture_banks_all();
            bank_unpack_all_raw(SI(ch1_buffer), 0);
        }
    }
    if (SI(ch2_enabled) != 0) {
        stream_geometry_fpga_selector_0x22_configuration();
        m = SI(bank_mode);
        if (m == 0) {
            capture_banks_4_7();
            bank_unpack_4_7_raw(SI(ch2_buffer), 1);
        } else if (m == 2) {
            capture_banks_all();
            bank_unpack_all_raw(SI(ch2_buffer), 1);
        }
    }
    copy_u16((unsigned short *)(0x42AC7CF4u + SI(runtime_fine_progress) * 2), (const unsigned short *)(SI(ch1_buffer) + SI(runtime_field_704) * 2), SI(runtime_step));
    copy_u16((unsigned short *)(0x42AC7CF4u + SI(runtime_fine_progress) * 2 + 0x62250), (const unsigned short *)(SI(ch2_buffer) + SI(runtime_field_704) * 2), SI(runtime_step));
}
