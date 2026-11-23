/*
 * capture_block -- stock 0x40040638 (108 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * One capture into buf: trigger position and selector 0x1F <- v, fpga_pe_mode_select(1), csi_set_buffer(buf), timer 0 stopped, CSI capture on, PE0 high until SYS_INFO+0x70E is set (the CSI interrupt), PE0 low, timer 0 restarted, fpga_pe_mode_select(0), then csi_clear_frame_flag.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "acquisition.h"
#include "drivers/csi.h"
#include "drivers/timer.h"
#include "fpga.h"
#include "sys_info.h"

void capture_block(u32 v, u32 buf)
{
    trigger_pos_update();
    fpga_write_sel_0x1f(v);
    fpga_pe_mode_select(1);
    csi_set_buffer(buf);
    timer_stop(0);
    enable_csi_capture();
    pe0_set();
    while (SI(f_70e) == 0)
        ;
    pe0_clr();
    timer_start(0);
    fpga_pe_mode_select(0);
    csi_clear_frame_flag();
}
