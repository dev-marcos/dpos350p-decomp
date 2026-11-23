/*
 * touch_drag_pick -- stock 0x40069B0C (492 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Picks the object a plot press drags and stores it in byte 0x42E68998 (codes of plot_touch_drag): 0 when the touch point 0x42E68BB0 + 4 / + 6 is (0, 0); 13 when SYS+0x652 is set; else the nearest cursor within the radius word 0x42E68998 + 4: SYS+0x802 -> touch_hit_cursor_x, words + 0x48 / + 0x4C give 7 / 8; SYS+0x803 -> touch_hit_cursor_y, + 0x50 / + 0x54 give 9 / 10; SYS+0x3DA52C -> touch_hit_x_b, + 0x58 / + 0x5C give 11 / 12 (unsigned, a later equal distance does not win). Without a cursor, the nearest trace: CH2 (SYS+0x102) 4, its zoom part 6 in split view, CH1 (SYS+0x3E) 3, its zoom part 5. When both channels are on and the chosen trace and the other one of the same part are both under 10 px away, the result is 3 when word SYS+0xB30 < SYS+0xB4C, else 4.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "ui/input.h"
#include "sys_info.h"

#define SYS 0x40544E68u
#define U 0x42E68998u                /* drag state */
#define TCH 0x42E68BB0u              /* touch point: +4 / +6 now */
#define B8(a) (*(volatile unsigned char *)(a))
#define S16(a) (*(volatile short *)(a))
#define W32(a) (*(volatile u32 *)(a))

#define SPLIT B8(SYS + 0x2AA)

static u32 pick(void)
{
    u32 m = 0, best = W32(U + 4), o;

    if (SI(cursor_x_on)) {
        touch_hit_cursor_x();
        for (o = 0; o < 2; o++)
            if (W32(U + 0x48 + 4 * o) < best) {
                best = W32(U + 0x48 + 4 * o);
                m = 7 + o;
            }
    }
    if (SI(cursor_y_on)) {
        touch_hit_cursor_y();
        for (o = 0; o < 2; o++)
            if (W32(U + 0x50 + 4 * o) < best) {
                best = W32(U + 0x50 + 4 * o);
                m = 9 + o;
            }
    }
    if (SI(f_3da52c)) {
        touch_hit_x_b();
        for (o = 0; o < 2; o++)
            if (W32(U + 0x58 + 4 * o) < best) {
                best = W32(U + 0x58 + 4 * o);
                m = 11 + o;
            }
    }
    if (m)
        return m;
    best = 0x7FFFFFFF;
    if (SI(ch2_enabled)) {
        trace_hit_ch2();
        if (W32(U + 0x3C) < best) {
            best = W32(U + 0x3C);
            m = 4;
        }
        if (SPLIT) {
            trace_hit_zoom_ch2();
            if (W32(U + 0x44) < best) {
                best = W32(U + 0x44);
                m = 6;
            }
        }
    }
    if (!SI(ch1_enabled))
        return m;
    trace_hit_ch1();
    if (W32(U + 0x38) < best) {
        best = W32(U + 0x38);
        m = 3;
    }
    if (SPLIT) {
        trace_hit_zoom_ch1();
        if (W32(U + 0x40) < best)
            m = 5;
    }
    if (!SI(ch1_enabled) || !SI(ch2_enabled))
        return m;
    if (m == 3 || m == 4) {
        if (W32(U + 0x38) >= 10 || W32(U + 0x3C) >= 10)
            return m;
    } else if (m == 5 || m == 6) {
        if (W32(U + 0x40) >= 10 || W32(U + 0x44) >= 10)
            return m;
    } else
        return m;
    return SI(f_b30) < SI(f_b4c) ? 3 : 4;
}

void touch_drag_pick(void)
{
    if (S16(TCH + 4) == 0 && S16(TCH + 6) == 0) {
        B8(U) = 0;
        return;
    }
    B8(U) = SI(f_652) ? 13 : pick();
}
