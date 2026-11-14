/*
 * frontend_apply_all -- stock 0x4004A510 (64 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Applies every front-end setting: offsets, vertical scales, calibration selector, V/div labels, FPGA selectors 0x62/0x28/0x2B/0x2F/0x29/0x30/0x2C, channel mode.
 */
#include "hw/platform.h"

#include "hw/fpga.h"

void offset_clamp(void);
void vscale_update(void);
void vdiv_labels_update(void);
void fpga_write_sel_0x62(void);
void ch1_frontend_apply(void);
void ch2_frontend_apply(void);
void fpga_write_sel_0x2f(void);
void fpga_write_sel_0x29(void);
void fpga_write_sel_0x30(void);
void fpga_write_sel_0x2c(void);
void fpga_channel_mode_apply(void);

void frontend_apply_all(void)
{
    offset_clamp();
    vscale_update();
    retained_inside_current_sc1_3_t6();
    vdiv_labels_update();
    fpga_write_sel_0x62();
    ch1_frontend_apply();
    ch2_frontend_apply();
    fpga_write_sel_0x2f();
    fpga_write_sel_0x29();
    fpga_write_sel_0x30();
    fpga_write_sel_0x2c();
    fpga_channel_mode_apply();
}
