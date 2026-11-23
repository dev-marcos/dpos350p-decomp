/*
 * ui_window_40_draw -- stock 0x40062EE8 (92 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Draw function of window 40 (block 0x42E68668): +5 = 0; 0x400612FC; unless both +0 and +4 are set, 0x40060FE8, 0x40060DC8, 0x40061F10; then 0x400617AC, 0x400623F4, 0x40062448, 0x40062B1C, 0x40062A18, 0x40062C98, 0x40061194 and func_400606a0_unless_locked.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "processing.h"
#include "stock_asm.h"
#include "ui/draw.h"
#include "ui/input.h"
#include "ui/screens.h"
#include "ui/windows.h"
#include "state/levels_block.h"

#define CB8(o)  (*(volatile unsigned char *)(0x42E68668u + (o)))

void ui_window_40_draw(void)
{
    LEVELS(f_005) = 0;
    levels_view_layout();
    if (CB8(0) == 0 || LEVELS(f_004) == 0) {
        auto_level_search();
        fft_spur_cancel();
        func_40061f10();
    }
    measure_values_clamp();
    levels_update_chain();
    levels_graph_draw();
    levels_shade_draw();
    levels_max_panel_draw();
    level_columns_draw();
    level_columns_draw_b();
    func_400606a0_unless_locked();
}
