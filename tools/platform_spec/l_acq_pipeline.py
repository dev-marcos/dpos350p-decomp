# Acquisition pipeline: capture blocks over the CSI banks, the runtime capture step and sweep,
# the dispatcher and the per-channel post-processing chain. exec'd by gen_platform.py


def p(addr, size, name, desc, body, src="acquisition"):
    f(addr, size, name, src, desc, '#include "hw/fpga.h"' + "\n\n" + body.strip("\n"))


EXT = """void func_4003ffe4(u32 v);                   /* fragment of the time-base switch (OPEN) */
"""

p(0x40040030, 12, "timebase_apply_2b4", "Time-base step with SYS_INFO+0x2B4 (0x4003FFE4).", EXT + """
void timebase_apply_2b4(void)
{
    func_4003ffe4(SYS32(0x2B4));
}
""")
p(0x4004003C, 12, "timebase_apply_2b0", "Time-base step with SYS_INFO+0x2B0 (0x4003FFE4).", EXT + """
void timebase_apply_2b0(void)
{
    func_4003ffe4(SYS32(0x2B0));
}
""")
p(0x40040048, 36, "timebase_apply_2b0_x1_3", "Time-base step with SYS_INFO+0x2B0 x 1.3 (double, truncated).", EXT + """
void timebase_apply_2b0_x1_3(void)
{
    func_4003ffe4((u32)((double)SYS32(0x2B0) * 1.3));
}
""")
p(0x4004006C, 164, "channel_post_process",
  "Post-processing of one channel (0 = CH1, needs SYS+0x3E; 1 = CH2, needs SYS+0x102) while SYS+0x554 < SYS+0x55C: time-base step x 1.3, measurement class, resampling of the channel buffer (SYS+0x7B4 + 4 ch) and the final normalisation.", """
void timebase_apply_2b0_x1_3(void);
void amplitude_statistical_normalization(u32 ch);
void workspace_copy_and_resampling(u32 ch, u32 src, u32 half, u32 len, u32 n);
void final_per_channel_normalization(u32 ch);

void channel_post_process(u32 ch)
{
    u32 w;
    if (ch == 0) {
        if (SYS8(0x3E) == 0)
            return;
    } else if (ch == 1) {
        if (SYS8(0x102) == 0)
            return;
    }
    if (SYS32(0x554) >= SYS32(0x55C))
        return;
    timebase_apply_2b0_x1_3();
    w = SYS32(0x7BC);
    amplitude_statistical_normalization(ch);
    workspace_copy_and_resampling(ch, SYS32(0x7B4 + 4 * ch), w >> 1, w & 0xFFFF, SYS32(0x55C));
    final_per_channel_normalization(ch);
}
""")
p(0x400405E0, 88, "stream_geometry_fpga_selector_0x22_configuration",
  "Stream geometry: csi_init_structs(SYS16+0x640 + 2 x SYS16+0x6F2), then selector 0x22 <- 0x14 + 2 x (SYS16+0x640 + SYS16+0x6F2), 16 bit, high byte first. (name: Hack)", """
void csi_init_structs(u32 v);

void stream_geometry_fpga_selector_0x22_configuration(void)
{
    u32 w;
    csi_init_structs((SYS16(0x640) + 2u * SYS16(0x6F2)) & 0xFFFF);
    w = 0x14 + 2u * (SYS16(0x6F2) + SYS16(0x640));
    fpga_selector_command_write(0x22);
    fpga_data_byte_write((w << 16) >> 24);
    fpga_data_byte_write(w & 0xFF);
}
""", src="acquisition; name: Hack")
p(0x40040638, 108, "capture_block",
  "One capture into buf: trigger position and selector 0x1F <- v, fpga_pe_mode_select(1), csi_set_buffer(buf), timer 0 stopped, CSI capture on, PE0 high until SYS_INFO+0x70E is set (the CSI interrupt), PE0 low, timer 0 restarted, fpga_pe_mode_select(0), then csi_clear_frame_flag.", """
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
""")
p(0x400406A4, 36, "capture_with_length", "SYS_INFO+0x63C = 20, SYS16+0x640 = len + 20, then capture_block(v, buf).", """
void capture_block(u32 v, u32 buf);

void capture_with_length(u32 v, u32 len, u32 buf)
{
    SYS32(0x63C) = 0x14;
    SYS16(0x640) = len + 0x14;
    capture_block(v, buf);
}
""")
p(0x400406C8, 20, "capture_bank", "capture_block(i, 0x42DF76D4 + i x 0x2134) (signed 16-bit product).", """
void capture_block(u32 v, u32 buf);

void capture_bank(u32 i)
{
    capture_block(i, 0x42DF76D4u + (int)(short)i * 0x2134);
}
""")
for (a, n, lo, hi) in [(0x400406DC, "capture_banks_all", 0, 8), (0x400406FC, "capture_banks_0_3", 0, 4), (0x4004071C, "capture_banks_4_7", 4, 8)]:
    p(a, 32, n, f"capture_bank({lo}..{hi - 1}).", f"""
void capture_bank(u32 i);

void {n}(void)
{{
    u32 i;
    for (i = {lo}; i < {hi}; i++)
        capture_bank(i & 0xFF);
}}
""")
p(0x400417E4, 308, "one_runtime_capture_step",
  "Waits for the FPGA ready flag (selector 0x0B, giving up on a touch), clears SYS16+0x29C/0x29E, applies the time base, captures CH1 (SYS+0x3E) and CH2 (SYS+0x102) by the bank mode SYS+0x100, then copies SYS16+0x6F6 samples of each channel (from the channel buffers SYS+0x7B4 / 0x7B8 at offset SYS+0x704) into the display buffers 0x42AC7CF4 (+0x62250 for CH2) at SYS+0x6FC. (name: Hack)", """
void runtime_ready_read(void);
void timebase_apply_2b0(void);
void stream_geometry_fpga_selector_0x22_configuration(void);
void capture_banks_all(void);
void capture_banks_0_3(void);
void capture_banks_4_7(void);
void bank_unpack_0_3(u32 buf, u32 ch);
void bank_unpack_4_7(u32 buf, u32 ch);
void bank_unpack_all(u32 buf, u32 ch);

void one_runtime_capture_step(void)
{
    u32 m;
    for (;;) {
        runtime_ready_read();
        if (SYS8(0x6EE) != 0)
            break;
        runtime_event_service();
        if (g_touch->busy2 != 0)
            break;
    }
    SYS16(0x29C) = 0;
    SYS16(0x29E) = 0;
    timebase_apply_2b0();
    if (SYS8(0x3E) != 0) {
        stream_geometry_fpga_selector_0x22_configuration();
        m = SYS8(0x100);
        if (m == 0) {
            capture_banks_0_3();
            bank_unpack_0_3(SYS32(0x7B4), 0);
        } else if (m == 1) {
            capture_banks_all();
            bank_unpack_all(SYS32(0x7B4), 0);
        }
    }
    if (SYS8(0x102) != 0) {
        stream_geometry_fpga_selector_0x22_configuration();
        m = SYS8(0x100);
        if (m == 0) {
            capture_banks_4_7();
            bank_unpack_4_7(SYS32(0x7B8), 1);
        } else if (m == 2) {
            capture_banks_all();
            bank_unpack_all(SYS32(0x7B8), 1);
        }
    }
    copy_u16((unsigned short *)(0x42AC7CF4u + SYS32(0x6FC) * 2), (const unsigned short *)(SYS32(0x7B4) + SYS32(0x704) * 2), SYS16(0x6F6));
    copy_u16((unsigned short *)(0x42AC7CF4u + SYS32(0x6FC) * 2 + 0x62250), (const unsigned short *)(SYS32(0x7B8) + SYS32(0x704) * 2), SYS16(0x6F6));
}
""", src="acquisition; name: Hack")
p(0x40041918, 4, "full_acquisition_veneer", "Branch to full_acquisition_entry.", """
void full_acquisition_entry(void);

void full_acquisition_veneer(void)
{
    full_acquisition_entry();
}
""")
p(0x4004191C, 176, "runtime_sweep_path",
  "Roll/sweep acquisition: only for the run modes 1, 2, 3, 5 and 0x13 (byte 0x42E68BC8) with no touch activity; re-arms, then captures and processes record by record while SYS+0x700 < SYS16+8, stopping (SYS+0x38 = 1) on a touch. (name: Hack)", """
void runtime_rearm_setup(void);
void one_runtime_capture_step(void);
void runtime_state_commit_copy(void);
void per_channel_processing(void);
void runtime_counter_progress_advance(void);
void write_sys_info_0x6f0_to_fpga(void);
void pulse_fpga_selector_0x08_0_1(void);

void runtime_sweep_path(void)
{
    volatile unsigned char *t = (volatile unsigned char *)0x42E68BB0u;
    u32 mode = t[0x18];
    if (mode != 1 && mode != 2 && mode != 3 && mode != 5 && mode != 0x13)
        return;
    if (t[0x17] != 0 || t[0x15] != 0 || t[0x0D] != 0)
        return;
    runtime_rearm_setup();
    while (SYS32(0x700) < SYS16(8)) {
        one_runtime_capture_step();
        runtime_state_commit_copy();
        per_channel_processing();
        runtime_counter_progress_advance();
        write_sys_info_0x6f0_to_fpga();
        pulse_fpga_selector_0x08_0_1();
        runtime_event_service();
        if (t[0x15] != 0) {
            SYS8(0x38) = 1;
            return;
        }
    }
}
""", src="acquisition; name: Hack")
p(0x400419CC, 92, "acquisition_dispatcher",
  "Acquisition entry per main-loop pass: nothing while touch byte +0x17 is set; SYS+0x38 = 0 when SYS+0x3C (stop) is set; otherwise the full acquisition (SYS+0x28 = 0) or the sweep path, then runtime_state_commit_copy. (name: Hack)", """
void full_acquisition_veneer(void);
void runtime_sweep_path(void);
void runtime_state_commit_copy(void);

void acquisition_dispatcher(void)
{
    volatile unsigned char *t = (volatile unsigned char *)0x42E68BB0u;
    if (t[0x17] != 0)
        return;
    if (SYS8(0x3C) != 0) {
        SYS8(0x38) = 0;
        return;
    }
    if (SYS8(0x28) == 0)
        full_acquisition_veneer();
    else
        runtime_sweep_path();
    runtime_state_commit_copy();
}
""", src="acquisition; name: Hack")
