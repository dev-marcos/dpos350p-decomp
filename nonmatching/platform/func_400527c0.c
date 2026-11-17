/*
 * buttons_20a_handler -- stock 0x400527C0 (552 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Window of button group 0x20A. While the slider byte 0x42E62203 is set: touch byte = 1 and SYS+0x654 = 100 * (x - lo) / (hi - lo) (sdiv), x = s16 0x42E68BB0 clamped to the halfwords lo = 0x42E62204, hi = 0x42E62208. Else buttons: 0x214 / 0x21E / 0x228 toggle SYS+0x653 / +0x652 / +0x655; 0x17E only clears; 0x232 closes windows 0xA, 5, 2, opens 0x1C and 5 and calls sys_3c_enter; 0x23C backs up the settings and generator state, reinitialises, and opens windows 0x22..0x24; 0x246 backs up the settings, opens windows 0x26..0x28 and sets SYS+0x54E = 1.
 */
#include "hw/platform.h"

#include "hw/fpga.h"

#define TB(o) (*(volatile unsigned char *)(0x42E61FF8u + (o)))
void ui_window_close(u32 id);
void ui_window_open(u32 id);
void ui_window_invalidate_from(u32 id, u32 r1);
void inv_left(u32 id) __asm__("ui_window_invalidate_from");

void touch_group_020A(void);
void sys_3c_enter(void);
void sys_info_backup(void);
void gen_state_backup(void);
void func_4003eb40(void);
void state_block_3f984_init(void);
void ui_windows_reset(void);
void fb_clear_current(void);
#define TH16(o) (*(volatile unsigned short *)(0x42E61FF8u + (o)))

void buttons_20a_handler(void)
{
    touch_group_020A();
    if (TB(0x20B)) {
        int x, lo, hi, d, n;
        TB(0) = 1;
        x = *(volatile short *)0x42E68BB0u;
        lo = TH16(0x20C);
        hi = TH16(0x210);
        if (x < lo)
            x = lo;
        else if (hi < x)
            x = hi;
        n = (int)((u32)(x - lo) * 100u);
        d = hi - lo;
        SYS8(0x654) = d == 0 ? 0 : d == -1 ? (int)(0u - (u32)n) : n / d;
        return;
    }
    if (TB(0x214) == 2) {
        TB(0) = 0;
        TB(0x214) = 0;
        SYS8(0x653) = SYS8(0x653) == 0;
    } else if (TB(0x21E) == 2) {
        TB(0) = 0;
        TB(0x21E) = 0;
        SYS8(0x652) = SYS8(0x652) == 0;
    } else if (TB(0x228) == 2) {
        TB(0) = 0;
        TB(0x228) = 0;
        SYS8(0x655) = SYS8(0x655) == 0;
    } else if (TB(0x17E) == 2) {
        TB(0x17E) = 0;
        TB(0) = 0;
    } else if (TB(0x232) == 2) {
        TB(0) = 0;
        TB(0x232) = 0;
        ui_window_close(0xA);
        ui_window_close(5);
        ui_window_close(2);
        ui_window_open(0x1C);
        ui_window_open(5);
        sys_3c_enter();
    } else if (TB(0x23C) == 2) {
        TB(0x23C) = 0;
        TB(0) = 0;
        sys_info_backup();
        gen_state_backup();
        func_4003eb40();
        state_block_3f984_init();
        ui_windows_reset();
        fb_clear_current();
        ui_window_open(0x22);
        ui_window_open(0x23);
        ui_window_open(0x24);
    } else if (TB(0x246) == 2) {
        TB(0x246) = 0;
        TB(0) = 0;
        sys_info_backup();
        func_4003eb40();
        ui_windows_reset();
        fb_clear_current();
        ui_window_open(0x26);
        ui_window_open(0x27);
        ui_window_open(0x28);
        SYS8(0x54E) = 1;
    }
}
