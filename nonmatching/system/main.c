/*
 * main -- stock 0x40034700 (148 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Boot sequence, then the main loop. Boot: sys_clock_init(0x44AA2000), sys_mmu_init, timers_init, word 0x40095490 = 1, factory_defaults, spi0_init(0), settings_load_all, ui_text_init, FPGA_Start, display_start, touch_init, ui_cf88_then_579c, colour_table_3dd530_init, disk_mount, SMEC_Init, power_monitor_init, acquisition_reconfigure, rate_table_init_entry, applied_state_invalidate, Gen_Init, sd_update_check, factory_steps_run, outputs_apply, ui_windows_open_1_5. Loop forever: touch_service, window_touch_poll, ui_frame_update, then the pass counter SI(f_64c) += 1.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "acquisition.h"
#include "drivers/ccu.h"
#include "drivers/display.h"
#include "drivers/spi.h"
#include "drivers/timer.h"
#include "fpga.h"
#include "frontend.h"
#include "generator.h"
#include "power.h"
#include "selftest.h"
#include "stock_asm.h"
#include "storage/diskio.h"
#include "storage/settings.h"
#include "system.h"
#include "ui/draw.h"
#include "ui/input.h"
#include "ui/screens.h"
#include "ui/text.h"
#include "ui/windows.h"
#include "sys_info.h"

int main(void)
{
    sys_clock_init(0x44AA2000u);
    sys_mmu_init();
    timers_init();
    *(volatile u32 *)0x40095490u = 1;
    factory_defaults();
    spi0_init(0);
    settings_load_all();
    ui_text_init();
    FPGA_Start();
    display_start();
    touch_init();
    ui_cf88_then_579c();
    colour_table_3dd530_init();
    disk_mount();
    SMEC_Init();
    power_monitor_init();
    acquisition_reconfigure();
    rate_table_init_entry();
    applied_state_invalidate();
    Gen_Init();
    sd_update_check();
    factory_steps_run();
    outputs_apply();
    ui_windows_open_1_5();
    for (;;) {
        touch_service();
        window_touch_poll();
        ui_frame_update();
        SI(f_64c) += 1;
    }
}
