/*
 * plot_touch_drag -- stock 0x400545F8 (1664 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Drag and tap handler of the plot area (record 0x42E61FF8 + 0xF2 / + 0xF3, drag state U = 0x42E68998). Press (record + 0xF2 = 1): ui_window_invalidate_from(3), touch_drag_pick picks the dragged object (byte U), whose start values are saved as int64 at U+8 / +0x10 / +0x18: 1 trigger position (SYS+0x570, or the zoom one SYS+0x578 in split view), 2 zoom position, 3 / 4 position plus CH1 / CH2 offset (SYS+0x60 / +0x120) plus the trigger level SYS+0x27E when that channel is the trigger channel, 5 / 6 zoom position plus CH1 / CH2 offset, 7 .. 10 cursor words SYS+0x808 .. +0x814, 11 / 12 words SYS+0x3DA534 / +0x3DA538, 13 both offsets; the press time 0x42E68CD8 is copied to 0x42E68C78. Move (record + 0xF3 = 1, flag byte set to 1): touch_drag_delta gives dx / dy (words U+0x30 / U+0x34) and the object is moved: positions by dx x the pixel time (int64 SYS+0x590, zoom SYS+0x5A0; only when SYS+0x580 is set for 1 .. 4), offsets and trigger level by dy, cursors by dx or dy with their clamp functions. Release (record + 0xF2 = 2): a tap (start and end point equal, under 600 ticks) on the right half of the plot steps the time base down (zoom_index_sub in split view), on the left half up (zoom_index_add), then clears SYS+0x586, acquisition_apply and ui_window_invalidate_from(3).
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "acquisition.h"
#include "processing.h"
#include "storage/files.h"
#include "ui/input.h"
#include "ui/windows.h"
#include "sys_info.h"

/* Same functions, declared the way the stock code calls them (see the definitions for the
 * full interface): the C must pass exactly these registers. */
void ui_window_invalidate_from_raw(u32 n) __asm__("ui_window_invalidate_from");

#define SYS 0x40544E68u
#define BTN 0x42E61FF8u              /* touch button records; byte 0 is the pending-touch flag */
#define B8(a) (*(volatile unsigned char *)(a))
#define S16(a) (*(volatile short *)(a))
#define W32(a) (*(volatile u32 *)(a))

#define U 0x42E68998u                /* drag state */
#define TCH 0x42E68BB0u              /* touch point: +4 / +6 now, +8 / +0xA at press */
#define S64(a) (*(volatile long long *)(a))
#define SPLIT B8(SYS + 0x2AA)

static void press_save(void)
{
    u32 m = B8(U);

    switch (m) {
    case 1:
        S64(U + 8) = S64(SYS + (SPLIT ? 0x578 : 0x570));
        break;
    case 2:
        S64(U + 8) = SI(f_578);
        break;
    case 3:
    case 4:
        S64(U + 8) = S64(SYS + (SPLIT ? 0x578 : 0x570));
        S64(U + 0x10) = (int)W32(SYS + (m == 3 ? 0x60 : 0x120));
        if (SI(trigger_channel) == m - 3)
            S64(U + 0x18) = S16(SYS + 0x27E);
        break;
    case 5:
    case 6:
        S64(U + 8) = SI(f_578);
        S64(U + 0x10) = (int)W32(SYS + (m == 5 ? 0x60 : 0x120));
        break;
    case 7:
    case 8:
    case 9:
    case 10:
        S64(U + 8) = (int)W32(SYS + 0x808 + 4 * (m - 7));
        break;
    case 11:
    case 12:
        S64(U + 8) = (int)W32(SYS + 0x3DA534 + 4 * (m - 11));
        break;
    case 13:
        S64(U + 8) = (int)SI(ch1_offset);
        S64(U + 0x10) = (int)SI(ch2_offset);
        break;
    }
}

static void h_drag(int dx)
{
    long long d = (long long)dx * S64(SYS + 0x590);

    if (SPLIT)
        SI(f_578) = S64(U + 8) + d;
    else
        SI(f_570) = S64(U + 8) - d;
}

static void move(void)
{
    u32 m = B8(U), lo8 = (u32)S64(U + 8);
    int dx = (int)W32(U + 0x30), dy = (int)W32(U + 0x34);

    switch (m) {
    case 1:
        if (SI(f_580))
            h_drag(dx);
        break;
    case 2:
        if (SI(f_580))
            SI(f_578) = S64(U + 8) - (long long)dx * SI(f_5a0);
        break;
    case 3:
    case 4:
        if (SI(f_580))
            h_drag(dx);
        W32(SYS + (m == 3 ? 0x60 : 0x120)) = dy + (u32)S64(U + 0x10);
        if (SI(trigger_channel) == m - 3)
            SI(trigger_level) = dy + (u32)S64(U + 0x18);
        break;
    case 5:
    case 6:
        SI(f_578) = S64(U + 8) - (long long)dx * SI(f_5a0);
        W32(SYS + (m == 5 ? 0x60 : 0x120)) = dy + (u32)S64(U + 0x10);
        break;
    case 7:
    case 8:
        W32(SYS + 0x808 + 4 * (m - 7)) = lo8 - dx;
        cursor_808_clamp();
        break;
    case 9:
    case 10:
        W32(SYS + 0x810 + 4 * (m - 9)) = lo8 - dy;
        cursor_810_clamp();
        break;
    case 11:
    case 12:
        W32(SYS + 0x3DA534 + 4 * (m - 11)) = lo8 - dx;
        cursor_3da534_clamp(m - 11);
        break;
    case 13:
        SI(ch1_offset) = lo8 - dx;
        SI(ch2_offset) = dy + (u32)S64(U + 0x10);
        break;
    }
}

void plot_touch_drag(void)
{
    int x;

    if (B8(BTN + 0xF2) == 1) {
        B8(BTN + 0xF2) = 0;
        B8(BTN) = 0;
        ui_window_invalidate_from_raw(3);
        touch_drag_pick();
        press_save();
        W32(0x42E68C78u) = W32(0x42E68CD8u);
    }
    if (B8(BTN + 0xF3) == 1) {
        B8(BTN + 0xF3) = 0;
        B8(BTN) = 1;
        touch_drag_delta();
        move();
    }
    if (B8(BTN + 0xF2) != 2)
        return;
    B8(BTN + 0xF2) = 0;
    B8(BTN) = 0;
    x = S16(TCH + 8);
    if (S16(TCH + 4) != x || S16(TCH + 6) != S16(TCH + 0xA) || W32(0x42E68CD8u) - W32(0x42E68C78u) >= 600)
        return;
    if (x >= (int)(SI(plot_x0) + (SI(plot_width) >> 1))) {
        if (!SPLIT)
            timebase_index_step_down(1);
        else
            zoom_index_sub(1);
    } else {
        if (!SPLIT)
            timebase_index_step(1);
        else
            zoom_index_add(1);
    }
    SI(f_586) = 0;
    acquisition_apply();
    ui_window_invalidate_from_raw(3);
}
