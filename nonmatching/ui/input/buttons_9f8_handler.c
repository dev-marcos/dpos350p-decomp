/*
 * buttons_9f8_handler -- stock 0x40055634 (228 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Buttons 0x9F8 and 0xA02 (the first also runs reload_chain_400373e0) leave mode 0x3C and swap windows 0x1C/5 for 2/5; buttons 0xA0C and 0xA16 set byte 0x4091F398 to 0 or 1 and invalidate from window 5.
 */
#include "hw/platform.h"

#include "hw/fpga.h"

#define TB(o) (*(volatile unsigned char *)(0x42E61FF8u + (o)))
void ui_window_close(u32 id);
void ui_window_open(u32 id);

void touch_group_09F8(void);
void reload_chain_400373e0(void);
void sys_3c_leave(void);
void ui_window_invalidate_from(u32 id, u32 r1);
#define FLAG (*(volatile unsigned char *)0x4091F398u)

void buttons_9f8_handler(void)
{
    touch_group_09F8();
    if (TB(0x9F8) == 2) {
        TB(0x9F8) = 0;
        TB(0) = 0;
        reload_chain_400373e0();
    } else if (TB(0xA02) == 2) {
        TB(0xA02) = 0;
        TB(0) = 0;
    } else {
        u32 r1;                      /* the stock leaves r1 = flag address or flag value */
        if (TB(0xA0C) == 2) {
            TB(0xA0C) = 0;
            TB(0) = 0;
            FLAG = 0;
            r1 = 0x4091F398u;
        } else if (TB(0xA16) == 2) {
            TB(0xA16) = 0;
            TB(0) = 0;
            FLAG = 1;
            r1 = 1;
        } else {
            return;
        }
        ui_window_invalidate_from(5, r1);
        return;
    }
    sys_3c_leave();
    ui_window_close(0x1C);
    ui_window_close(5);
    ui_window_open(2);
    ui_window_open(5);
}
