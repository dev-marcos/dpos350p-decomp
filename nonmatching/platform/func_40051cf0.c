/*
 * factory_steps_run -- stock 0x40051CF0 (140 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Unless the saved state word 0x42E68AE0 is 0x5A5555A5 (factory flow complete): runs each factory step whose done-word is not yet set (0x40050FE0 / +4 = 0xA5AA555A, 0x40051578 / +8 = 0x5A5AA5AA, 0x400519E4 / +0xC = 0xA5A5AA55, the last also setting the state word), saving after each (settings_e_save); then 0x400447C8, settings_load_all, state_blocks_init, outputs_apply and gen_update.
 */
#include "hw/platform.h"

#include "hw/fpga.h"

void factory_test_board(void);
void factory_test_tester(void);
void touch_targets_test(void);
void settings_e_save(void);
void func_400447c8(void);
void settings_load_all(void);
void state_blocks_init(void);
void outputs_apply(void);
void gen_update(void);
#define FS32(o) (*(volatile u32 *)(0x42E68AE0u + (o)))

void factory_steps_run(void)
{
    if (FS32(0) == 0x5A5555A5u)
        return;
    if (FS32(4) != 0xA5AA555Au) {
        factory_test_board();
        FS32(4) = 0xA5AA555Au;
        settings_e_save();
    }
    if (FS32(8) != 0x5A5AA5AAu) {
        factory_test_tester();
        FS32(8) = 0x5A5AA5AAu;
        settings_e_save();
    }
    if (FS32(0xC) != 0xA5A5AA55u) {
        touch_targets_test();
        FS32(0xC) = 0xA5A5AA55u;
        FS32(0) = 0x5A5555A5u;
        settings_e_save();
    }
    func_400447c8();
    settings_load_all();
    state_blocks_init();
    outputs_apply();
    gen_update();
}
