# Per-trace processing chain after a capture (traces 0/1 = CH1/CH2, 2/3 = their second traces when
# SYS_INFO+0x2AA is set), the runtime re-arm, and the rest of the window-stack helpers (close,
# invalidate, reset, per-frame update). exec'd by gen_platform.py
#
# The callees of the trace chain (0x4005E10C, 0x4003C0AC, 0x4003C158, 0x4003C314, 0x4003BB6C,
# 0x4005E12C, 0x4005DDD4, ...) are still assembly; the names here only describe the call structure.


def t(addr, size, name, desc, body, src="trace"):
    f(addr, size, name, src, desc, '#include "hw/fpga.h"' + "\n\n" + body.strip("\n"))


# ---------------------------------------------------------------- re-arm
t(0x4003C520, 48, "runtime_rearm_setup", "Re-arm after a settings change: 0x4003BEA8, 0x4003B7B8, 0x40024E14, 0x4003C008, fb_present, write_sys_info_0x6f0_to_fpga, acquisition_prepare, then pulse_fpga_selector_0x08_0_1. (name: Hack)", """
void runtime_state_commit_force(void);
void acquisition_rearm(void);
void func_40024e14(void);
void acquisition_geometry_setup(void);
void fb_present(void);
void write_sys_info_0x6f0_to_fpga(void);
void pulse_fpga_selector_0x08_0_1(void);

void runtime_rearm_setup(void)
{
    runtime_state_commit_force();
    acquisition_rearm();
    func_40024e14();
    acquisition_geometry_setup();
    fb_present();
    write_sys_info_0x6f0_to_fpga();
    acquisition_prepare();
    pulse_fpga_selector_0x08_0_1();
}
""", src="trace; name: Hack")

# ---------------------------------------------------------------- trace chain
t(0x4003C550, 112, "traces_e10c_enabled", "Runs 0x4005E10C for every enabled trace: 0 (CH1 on, SYS+0x3E), then 2 when SYS+0x2AA is set; 1 (CH2 on, SYS+0x102), then 3. When SYS+0x652 is set it only runs 0x4005E10C(0).", """
void layer_clear(u32 tr);

void traces_e10c_enabled(void)
{
    if (SYS8(0x652)) {
        layer_clear(0);
        return;
    }
    if (SYS8(0x3E)) {
        layer_clear(0);
        if (SYS8(0x2AA))
            layer_clear(2);
    }
    if (SYS8(0x102)) {
        layer_clear(1);
        if (SYS8(0x2AA))
            layer_clear(3);
    }
}
""")
t(0x4003C790, 16, "trace_c5cc_full", "0x4003C5CC(tr, 0, SYS16+8): the whole record.", """
void func_4003c5cc(u32 tr, u32 start, u32 n);

void trace_c5cc_full(u32 tr)
{
    func_4003c5cc(tr, 0, SYS16(8));
}
""")
t(0x4003C7A0, 68, "trace_c5cc_window", "When SYS+0x6FC >= SYS+0x708: 0x4003C5CC(tr, (u16)SYS+0x700, SYS16+0x6F8).", """
void func_4003c5cc(u32 tr, u32 start, u32 n);

void trace_c5cc_window(u32 tr)
{
    if (SYS32(0x6FC) < SYS32(0x708))
        return;
    func_4003c5cc(tr, SYS32(0x700) & 0xFFFF, SYS16(0x6F8));
}
""")
t(0x4003C7E4, 120, "trace_process_full", "Full processing of trace tr, run only when SYS+0x101 is set and, for a 0x4003BF78(tr) result of 1, SYS+0x2AA is set too: 0x4005F90C, 0x4003C0AC, 0x4003C158, 0x4003C314, 0x4003BB6C, trace_c5cc_full, 0x4005E12C, 0x4005DDD4.", """
u32 mode_not_0_or_1(u32 tr);
void trace_span_full_veneer(u32 tr);
void scaled_pair_update(u32 tr);
void func_4003c158(u32 tr);
void func_4003c314(u32 tr);
void func_4003bb6c(u32 tr);
void trace_c5cc_full(u32 tr);
void func_4005e12c(u32 tr);
void func_4005ddd4(u32 tr);

void trace_process_full(u32 tr)
{
    u32 k = mode_not_0_or_1(tr);
    if (SYS8(0x101) == 0)
        return;
    if (k == 1 && SYS8(0x2AA) == 0)
        return;
    trace_span_full_veneer(tr);
    scaled_pair_update(tr);
    func_4003c158(tr);
    func_4003c314(tr);
    func_4003bb6c(tr);
    trace_c5cc_full(tr);
    func_4005e12c(tr);
    func_4005ddd4(tr);
}
""")
t(0x4003C85C, 116, "channel_traces_process_full", "trace_process_full for channel ch (0 or 1) when it is on, then for its second trace (ch + 2) when SYS+0x2AA is set.", """
void trace_process_full(u32 tr);

void channel_traces_process_full(u32 ch)
{
    if (ch == 0) {
        if (SYS8(0x3E) == 0)
            return;
        trace_process_full(0);
        if (SYS8(0x2AA))
            trace_process_full(2);
    } else if (ch == 1) {
        if (SYS8(0x102) == 0)
            return;
        trace_process_full(1);
        if (SYS8(0x2AA))
            trace_process_full(3);
    }
}
""")
t(0x4003C8D4, 60, "channels_process_full_ordered", "Both channels through channel_traces_process_full; CH2 first when the byte p+0x664 is 0, else CH1 first.", """
void channel_traces_process_full(u32 ch);

void channels_process_full_ordered(volatile unsigned char *p)
{
    if (p[0x664] == 0) {
        channel_traces_process_full(1);
        channel_traces_process_full(0);
    } else {
        channel_traces_process_full(0);
        channel_traces_process_full(1);
    }
}
""")
t(0x4003C8D0, 4, "channels_process_full", "channels_process_full_ordered(SYS_INFO). The stock entry is one instruction that falls through into 0x4003C8D4.", """
void channels_process_full_ordered(volatile unsigned char *p);

void channels_process_full(void)
{
    channels_process_full_ordered((volatile unsigned char *)0x40544E68u);
}
""")
t(0x4003C910, 96, "traces_process_single", "Processing used when SYS+0x652 is set: 0x4005F90C(0/1), 0x4003C0AC(0), 0x4003C158(0/1), 0x4003C314(0/1), 0x4003BB6C(0), 0x4005EDC0(0), 0x4005DDD4(0).", """
void trace_span_full_veneer(u32 tr);
void scaled_pair_update(u32 tr);
void func_4003c158(u32 tr);
void func_4003c314(u32 tr);
void func_4003bb6c(u32 tr);
void func_4005edc0(u32 tr);
void func_4005ddd4(u32 tr);

void traces_process_single(void)
{
    trace_span_full_veneer(0);
    trace_span_full_veneer(1);
    scaled_pair_update(0);
    func_4003c158(0);
    func_4003c158(1);
    func_4003c314(0);
    func_4003c314(1);
    func_4003bb6c(0);
    func_4005edc0(0);
    func_4005ddd4(0);
}
""")
t(0x4003C970, 76, "traces_update", "Unless the byte 0x42E68BC7 is set, and unless SYS+0x27A is 1 with SYS+0x3C and SYS+0x38 both 0: channels_process_full, or traces_process_single when SYS+0x652 is set.", """
void channels_process_full(void);
void traces_process_single(void);

void traces_update(void)
{
    if (*(volatile unsigned char *)0x42E68BC7u)
        return;
    if (SYS8(0x27A) == 1 && SYS8(0x3C) == 0 && SYS8(0x38) == 0)
        return;
    if (SYS8(0x652) == 0)
        channels_process_full();
    else
        traces_process_single();
}
""")
t(0x4003C9BC, 56, "trace_process_window", "Processing of trace tr over the capture window: 0x4005F910, 0x4003C0AC, 0x4003BB6C, trace_c5cc_window, 0x4005E838, 0x4005DF5C.", """
void trace_span_window_veneer(u32 tr);
void scaled_pair_update(u32 tr);
void func_4003bb6c(u32 tr);
void trace_c5cc_window(u32 tr);
void func_4005e838(u32 tr);
void func_4005df5c(u32 tr);

void trace_process_window(u32 tr)
{
    trace_span_window_veneer(tr);
    scaled_pair_update(tr);
    func_4003bb6c(tr);
    trace_c5cc_window(tr);
    func_4005e838(tr);
    func_4005df5c(tr);
}
""")
t(0x4003C9F4, 52, "per_channel_processing", "trace_process_window for CH1 and CH2 when they are on. (name: Hack)", """
void trace_process_window(u32 tr);

void per_channel_processing(void)
{
    if (SYS8(0x3E))
        trace_process_window(0);
    if (SYS8(0x102))
        trace_process_window(1);
}
""", src="trace; name: Hack")
t(0x4003CA38, 72, "table_scatter_42cf64b4", "Clears 0x3AAC bytes at 0x42CF64B4, then for the 0x3AAC (value, index) halfword pairs at 0x40090444 stores value at 0x42CF64B4 + 2 x index.", """
void table_scatter_42cf64b4(void)
{
    volatile unsigned short *d = (volatile unsigned short *)0x42CF64B4u;
    const volatile unsigned short *s = (const volatile unsigned short *)0x40090444u;
    u32 n;
    memset_w((void *)d, 0, 0x3AAC);
    for (n = 0x3AAC; n; n--) {
        d[s[1]] = s[0];
        s += 2;
    }
}
""")

# ---------------------------------------------------------------- window stack (see p_ui_windows.py)
W = """#define WIN(id)  ((volatile unsigned char *)(0x42E64520u + (id) * 14u))
#define WSTK     ((volatile unsigned char *)0x42E68BC8u)
void ui_window_stack_top_update(void);
"""
t(0x4003CDC0, 60, "ui_windows_mark_dirty", "Marks dirty (+1) every window on the stack, from slot 0 up to the first empty slot.", W + """
void ui_windows_mark_dirty(void)
{
    u32 i, id;
    for (i = 0; i < 20; i++) {
        id = WSTK[2 + i];
        if (id == 0)
            break;
        WIN(id)[1] = 1;
    }
}
""", src="ui")
t(0x4003CD44, 124, "ui_window_invalidate_from", "Marks dirty the window id and every window above it on the stack, up to the first empty slot. Stock quirk: when id is not on the stack the start slot is whatever the caller left in r1 (low byte); the C takes it as the second argument.", W + """
void ui_window_invalidate_from(u32 id, u32 r1)
{
    u32 i, slot = r1, w;
    for (i = 0; (i & 0xFF) < 20; i++) {
        if (WSTK[2 + i] == id) {
            slot = i;
            break;
        }
    }
    slot &= 0xFF;
    for (; slot < 20; slot++) {
        w = WSTK[2 + slot];
        if (w == 0)
            break;
        WIN(w)[1] = 1;
    }
}
""", src="ui")
t(0x4003CE50, 188, "ui_window_close", "Closes window id when it is open: clears +0, +1, +3, +5, removes it from the stack by moving the slots above down one (slot 19 keeps its value), marks every stacked window dirty and updates the stack top. Stock quirk: when id is not on the stack the slot used is whatever the caller left in r2; the C takes it as the third argument.", W + """
void ui_window_close(u32 id, u32 r1, u32 r2)
{
    volatile unsigned char *w = WIN(id);
    u32 i, slot = r2, k;
    (void)r1;
    if (w[0] == 0)
        return;
    w[0] = 0;
    w[1] = 0;
    w[3] = 0;
    w[5] = 0;
    for (i = 0; i <= 19; i++) {
        if (WSTK[2 + i] == id) {
            slot = i;
            break;
        }
    }
    if (slot < 19) {
        for (k = 19 - slot; k; k--, slot++)
            WSTK[2 + slot] = WSTK[3 + slot];
    }
    for (i = 0; i < 20; i++) {
        k = WSTK[2 + i];
        if (k)
            WIN(k)[1] = 1;
    }
    ui_window_stack_top_update();
}
""", src="ui")
t(0x4003CF0C, 76, "ui_windows_reset", "Clears +0, +1, +3, +5 of the first 100 window records, clears the 20 stack slots (0x4005F914) and the stack top halfword.", W + """
void mem_clear(volatile void *p, u32 n);

void ui_windows_reset(void)
{
    volatile unsigned char *w = WIN(0);
    u32 n;
    for (n = 100; n; n--) {
        w[0] = 0;
        w[1] = 0;
        w[3] = 0;
        w[5] = 0;
        w += 14;
    }
    mem_clear(WSTK + 2, 20);
    *(volatile unsigned short *)WSTK = 0;
}
""", src="ui")
t(0x4003CF58, 48, "ui_frame_update", "ui_window_redraw_dirty, battery_service, then, unless the byte 0x42E68BC7 is set, fb_present and the halfword 0x42E68BC4 = 0.", """
void ui_window_redraw_dirty(void);
void battery_service(void);
void fb_present(void);

void ui_frame_update(void)
{
    ui_window_redraw_dirty();
    battery_service();
    if (*(volatile unsigned char *)0x42E68BC7u)
        return;
    fb_present();
    *(volatile unsigned short *)0x42E68BC4u = 0;
}
""", src="ui")
