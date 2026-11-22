/*
 * main -- stock 0x40034700 (148 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Boot sequence, then the main loop. Boot: sys_clock_init(0x44AA2000), sys_mmu_init, timers_init, word 0x40095490 = 1, factory_defaults, spi0_init(0), settings_load_all, ui_text_init, FPGA_Start, display_start, touch_init, ui_cf88_then_579c, colour_table_3dd530_init, disk_mount, SMEC_Init, power_monitor_init, acquisition_reconfigure, rate_table_init_entry, applied_state_invalidate, Gen_Init, sd_update_check, factory_steps_run, outputs_apply, ui_windows_open_1_5. Loop forever: touch_service, window_touch_poll, ui_frame_update, then the pass counter SYS32(0x64C) += 1.
 */
#include "hw/platform.h"

#include "hw/fpga.h"

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
}
