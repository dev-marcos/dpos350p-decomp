/*
 * trace_update_chain -- stock 0x40037AB8 (144 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * trace_cursor_limits and trace_defaults_37b48, then (depending on the bytes 0x42E1810C/0D/0F) func_4003a300, func_40038374, func_4003963c + func_40039918, and func_40039254 unless +0x44 is set while +0x47 is clear; when +0x44 is 1 and +0x47 is set, +0x45 = 1 and window 0x22 is invalidated. Ends with trace_cursor_values and func_40037c3c_unless_t17.
 */
#include "hw/platform.h"

#include "hw/fpga.h"

#define G8(o) (*(volatile unsigned char *)(0x42E180C8u + (o)))
void trace_cursor_limits(void);
void trace_defaults_37b48(void);
void func_4003a300(void);
void func_40038374(void);
void func_4003963c(void);
void func_40039918(void);
void func_40039254(void);
void ui_window_invalidate_from(u32 id, u32 r1);
void trace_cursor_values(void);
void func_40037c3c_unless_t17(void);

void trace_update_chain(void)
{
    trace_cursor_limits();
    trace_defaults_37b48();
    if (G8(0x45) == 0)
        func_4003a300();
    func_40038374();
    if (G8(0x47)) {
        if (G8(0x45))
            goto redraw;
        func_4003963c();
        func_40039918();
    }
    if (G8(0x44) && G8(0x47) == 0)
        goto done;
redraw:
    func_40039254();
    if (G8(0x44) == 1) {
        u32 k = G8(0x47);
        if (k) {
            G8(0x45) = 1;
            ui_window_invalidate_from(0x22, k);
        }
    }
done:
    trace_cursor_values();
    func_40037c3c_unless_t17();
}
