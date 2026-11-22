/*
 * factory_reset_keep_language -- stock 0x400447C8 (64 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * factory_defaults with the menu language SYS+0x7AC kept, then for slot SYS+0x7C0 = 5, 4, .. 1: settings_f_save and settings_slot_save; then settings_b_save, wdt_system_reset and for (;;) until the watchdog fires. factory_steps_run calls it after the factory steps.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "drivers/wdt.h"
#include "storage/settings.h"
#include "system.h"

void factory_reset_keep_language(void)
{
    u32 lang = SYS8(0x7AC);
    int k;
    factory_defaults();
    SYS8(0x7AC) = lang;
    for (k = 5; k >= 1; k--) {
        SYS8(0x7C0) = k;
        settings_f_save();
        settings_slot_save();
    }
    settings_b_save();
    wdt_system_reset();
    for (;;)
        ;
}
