# main and the factory reset that keeps the menu language.
# exec'd by gen_platform.py (do not define HDR, SPEC, OUT or f here)

f(0x40034700, 148, "main", "init",
  "Boot sequence, then the main loop. Boot: sys_clock_init(0x44AA2000), sys_mmu_init, timers_init, word "
  "0x40095490 = 1, factory_defaults, spi0_init(0), settings_load_all, ui_text_init, FPGA_Start, display_start, "
  "touch_init, ui_cf88_then_579c, colour_table_3dd530_init, disk_mount, SMEC_Init, power_monitor_init, "
  "acquisition_reconfigure, rate_table_init_entry, applied_state_invalidate, Gen_Init, sd_update_check, "
  "factory_steps_run, outputs_apply, ui_windows_open_1_5. Loop forever: touch_service, window_touch_poll, "
  "ui_frame_update, then the pass counter SYS32(0x64C) += 1.", r'''#include "hw/fpga.h"

void sys_clock_init(u32 a);
void sys_mmu_init(void);
void timers_init(void);
void factory_defaults(void);
void spi0_init(u32 a);
void settings_load_all(void);
void ui_text_init(void);
void FPGA_Start(void);
void display_start(void);
void touch_init(void);
void ui_cf88_then_579c(void);
void colour_table_3dd530_init(void);
int disk_mount(void);
void SMEC_Init(void);
void power_monitor_init(void);
void acquisition_reconfigure(void);
void rate_table_init_entry(void);
void applied_state_invalidate(void);
void Gen_Init(void);
void sd_update_check(void);
void factory_steps_run(void);
void outputs_apply(void);
void ui_windows_open_1_5(void);
void touch_service(void);
void window_touch_poll(void);
void ui_frame_update(void);

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
        SYS32(0x64C) += 1;
    }
}''')

f(0x400447C8, 64, "factory_reset_keep_language", "init",
  "factory_defaults with the menu language SYS+0x7AC kept, then for slot SYS+0x7C0 = 5, 4, .. 1: "
  "settings_f_save and settings_slot_save; then settings_b_save, wdt_system_reset and for (;;) until the "
  "watchdog fires. factory_steps_run calls it after the factory steps.", r'''#include "hw/fpga.h"

void factory_defaults(void);
void settings_f_save(void);
void settings_slot_save(void);
void settings_b_save(void);
void wdt_system_reset(void);

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
}''')
