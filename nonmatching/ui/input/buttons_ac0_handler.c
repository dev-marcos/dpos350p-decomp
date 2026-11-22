/*
 * buttons_ac0_handler -- stock 0x40055F2C (340 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Buttons 0xAC0 (restore SYS_INFO and the generator state, reset and clear the screen, open windows 1 and 5), 0xACA (open window 0x25), 0xAD4 (save a screenshot), 0xADE / 0xAF2 (toggle bytes 0x42E22D28 / 0x42E1810C), 0xAFC (toggle byte 0x42E1810D, copied to 0x42E1810F).
 */
#include "hw/platform.h"

#include "hw/fpga.h"

#define TB(o) (*(volatile unsigned char *)(0x42E61FF8u + (o)))
void ui_window_close(u32 id);
void ui_window_open(u32 id);

void touch_group_0AC0(void);
void sys_info_restore(void);
void gen_state_restore(void);
void ui_windows_reset(void);
void fb_clear_current(void);
void ui_windows_open_1_5(void);
void list_a_save_new(void);
#define TT8(o) (*(volatile unsigned char *)(0x42E180C8u + (o)))

void buttons_ac0_handler(void)
{
    u32 v;
    touch_group_0AC0();
    if (TB(0xAC0) == 2) {
        TB(0xAC0) = 0;
        TB(0) = 0;
        sys_info_restore();
        gen_state_restore();
        ui_windows_reset();
        fb_clear_current();
        ui_windows_open_1_5();
    } else if (TB(0xACA) == 2) {
        TB(0xACA) = 0;
        TB(0) = 0;
        ui_window_open(0x25);
    } else if (TB(0xAD4) == 2) {
        TB(0xAD4) = 0;
        TB(0) = 0;
        list_a_save_new();
    } else if (TB(0xADE) == 2) {
        TB(0xADE) = 0;
        TB(0) = 0;
        TT8(0xAC60) = TT8(0xAC60) == 0;
    } else if (TB(0xAF2) == 2) {
        TB(0xAF2) = 0;
        TB(0) = 0;
        TT8(0x44) = TT8(0x44) == 0;
    } else if (TB(0xAFC) == 2) {
        TB(0xAFC) = 0;
        TB(0) = 0;
        v = TT8(0x45) == 0;
        TT8(0x45) = v;
        TT8(0x47) = v;
    }
}
