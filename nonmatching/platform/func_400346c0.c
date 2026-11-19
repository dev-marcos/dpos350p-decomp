/*
 * ui_frame_chain -- stock 0x400346C0 (64 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * outputs_apply, sys_rect_redraw_unless_t17, acquisition_dispatcher, acquisition_rearm, traces_update, zoom_views_update, 0x40042520, func_40023ea4_or_40024c60, 0x4003117C, 0x40030F8C, 0x40031794, then func_40028364_unless_t17. Most of them do nothing while touch byte +0x17 is set.
 */
#include "hw/platform.h"

#include "hw/fpga.h"

void sys_rect_redraw_unless_t17(void);
void acquisition_rearm(void);
void traces_update(void);
void zoom_views_update(void);
void split_window_marker_draw(void);
void func_40023ea4_or_40024c60(void);
void manual_cursors_draw(void);
void measure_panel_draw(void);
void func_40031794(void);
void func_40028364_unless_t17(void);

void ui_frame_chain(void)
{
    outputs_apply();
    sys_rect_redraw_unless_t17();
    acquisition_dispatcher();
    acquisition_rearm();
    traces_update();
    zoom_views_update();
    split_window_marker_draw();
    func_40023ea4_or_40024c60();
    manual_cursors_draw();
    measure_panel_draw();
    func_40031794();
    func_40028364_unless_t17();
}
