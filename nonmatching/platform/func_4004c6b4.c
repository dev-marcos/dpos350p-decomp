/*
 * system_calibration -- stock 0x4004C6B4 (212 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * System calibration (factory): clears the calibration object, calibrates each row, fills the aux arrays above the calibrated rows with the last value, runs the channel-multiplex check for V/div 0..10 and saves the calibration only when all pass. Returns the last check result.
 */
#include "hw/platform.h"

#include "hw/fpga.h"

int system_calibration(void)
{
    u32 i, r;
    int res;
    func_4003f0c0();
    memset_w((void *)CAL_OBJ, 0, 0x1A90);
    for (i = 0; i < SYS8(0x1B9); )
        func_4004bfc8(i++ & 0xFF);
    {
        u32 b = SYS8(0x1B9), a = SYS8(0x1BA);
        if (b < a) {
            volatile u32 *src = (volatile u32 *)((u32)CAL_OBJ + b * 4);
            volatile u32 *dst = (volatile u32 *)((u32)CAL_OBJ + b * 4 + 0x50);
            u32 n = a - b;
            do {
                dst[0] = src[0x4C / 4];
                dst[0xA0 / 4] = src[0xEC / 4];
                dst[0x50 / 4] = src[0x9C / 4];
                dst[0xF0 / 4] = src[0x13C / 4];
                dst++;
            } while (--n);
        }
    }
    func_4004b0a0();
    for (r = 0; ; ) {
        res = post_channel_mux_check(r & 0xFF);
        if (res != 1)
            break;
        if (++r >= 11) {
            calibration_save_wrapper();
            break;
        }
    }
    func_4003f0e0();
    g_touch->enabled = 1;
    return res;
}
