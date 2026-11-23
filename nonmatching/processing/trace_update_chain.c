/*
 * trace_update_chain -- stock 0x40037AB8 (144 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * trace_cursor_limits and trace_defaults_37b48, then (depending on the bytes 0x42E1810C/0D/0F) bode_sweep_run, bode_grid_draw, bode_autoscale + bode_curves_zoom, and bode_curves_draw unless +0x44 is set while +0x47 is clear; when +0x44 is 1 and +0x47 is set, +0x45 = 1 and window 0x22 is invalidated. Ends with trace_cursor_values and func_40037c3c_unless_t17.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "bode.h"
#include "processing.h"
#include "ui/screens.h"
#include "ui/windows.h"
#include "state/bode_block.h"


void trace_update_chain(void)
{
    trace_cursor_limits();
    trace_defaults_37b48();
    if (BODES(running) == 0)
        bode_sweep_run();
    bode_grid_draw();
    if (BODES(f_047)) {
        if (BODES(running))
            goto redraw;
        bode_autoscale();
        bode_curves_zoom();
    }
    if (BODES(loop) && BODES(f_047) == 0)
        goto done;
redraw:
    bode_curves_draw();
    if (BODES(loop) == 1) {
        u32 k = BODES(f_047);
        if (k) {
            BODES(running) = 1;
            ui_window_invalidate_from(0x22, k);
        }
    }
done:
    trace_cursor_values();
    func_40037c3c_unless_t17();
}
