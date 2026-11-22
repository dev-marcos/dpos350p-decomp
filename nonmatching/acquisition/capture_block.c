/*
 * capture_block -- stock 0x40040638 (108 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * One capture into buf: trigger position and selector 0x1F <- v, fpga_pe_mode_select(1), csi_set_buffer(buf), timer 0 stopped, CSI capture on, PE0 high until SYS_INFO+0x70E is set (the CSI interrupt), PE0 low, timer 0 restarted, fpga_pe_mode_select(0), then csi_clear_frame_flag.
 */
#include "hw/platform.h"

#include "hw/fpga.h"

void pe0_set(void);
void pe0_clr(void);
void fpga_write_sel_0x1f(u32 v);
void fpga_pe_mode_select(u32 v);
void csi_set_buffer(u32 buf);
void csi_clear_frame_flag(void);
void timer_start(int n);
void enable_csi_capture(void);

void capture_block(u32 v, u32 buf)
{
    trigger_pos_update();
    fpga_write_sel_0x1f(v);
    fpga_pe_mode_select(1);
    csi_set_buffer(buf);
    timer_stop(0);
    enable_csi_capture();
    pe0_set();
    while (SYS8(0x70E) == 0)
        ;
    pe0_clr();
    timer_start(0);
    fpga_pe_mode_select(0);
    csi_clear_frame_flag();
}
