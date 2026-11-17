# Acquisition control: state backup/restore around calibration, FPGA acquisition selectors
# (0x08/0x09/0x0A/0x0B/0x0E/0x0F/0x1F/0x20/0x21), trigger-position arithmetic, the acquisition wait
# and the per-channel measurement classifier. exec'd by gen_platform.py


def q(addr, size, name, desc, body, src="acquisition"):
    f(addr, size, name, src, desc, '#include "hw/fpga.h"' +"\n\n" + body.strip("\n"))


# ---------------------------------------------------------------- state backup / restore
for (a, n, d, s, sz, what) in [
        (0x4003F024, "sys_info_backup", "SYS_BACKUP", "(void *)0x40544E68u", "SYS_SIZE", "SYS_INFO (0x3DD588 bytes) into 0x409223F0"),
        (0x4003F034, "gen_state_backup", "(void *)0x42E428F4u", "(void *)GEN_G", "0x2F70", "generator state (0x2F70 bytes) into 0x42E428F4"),
        (0x4003F044, "state_c_backup", "(void *)0x42E22DE8u", "(void *)0x42E180C8u", "0xAD20", "the block 0x42E180C8 (0xAD20 bytes) into 0x42E22DE8"),
        (0x4003F054, "state_d_backup", "(void *)0x42E68800u", "(void *)0x42E68668u", "0x198", "the block 0x42E68668 (0x198 bytes) into 0x42E68800"),
        (0x4003F088, "gen_state_restore", "(void *)GEN_G", "(void *)0x42E428F4u", "0x2F70", "generator state back from 0x42E428F4"),
        (0x4003F098, "state_c_restore", "(void *)0x42E180C8u", "(void *)0x42E22DE8u", "0xAD20", "the block 0x42E180C8 back"),
        (0x4003F0A8, "state_d_restore", "(void *)0x42E68668u", "(void *)0x42E68800u", "0x198", "the block 0x42E68668 back")]:
    q(a, 16, n, f"Copies {what}.", f"""
void {n}(void)
{{
    memcpy({d}, {s}, {sz});
}}
""")
q(0x4003F064, 36, "sys_info_restore", "SYS_INFO back from 0x409223F0, then 0x4003EF1C.", """
void sys_info_restore(void)
{
    memcpy((void *)0x40544E68u, SYS_BACKUP, SYS_SIZE);
    applied_state_invalidate();
}
""")
q(0x4003F0C0, 32, "state_backup_all", "Saves SYS_INFO, the generator state and two other blocks (around calibration).", """
void sys_info_backup(void); void gen_state_backup(void); void state_c_backup(void); void state_d_backup(void);

void state_backup_all(void)
{
    sys_info_backup();
    gen_state_backup();
    state_c_backup();
    state_d_backup();
}
""")
q(0x4003F0E0, 32, "state_restore_all", "Restores what state_backup_all saved.", """
void sys_info_restore(void); void gen_state_restore(void); void state_c_restore(void); void state_d_restore(void);

void state_restore_all(void)
{
    sys_info_restore();
    gen_state_restore();
    state_c_restore();
    state_d_restore();
}
""")

# ---------------------------------------------------------------- FPGA acquisition selectors
q(0x4003F894, 36, "timebase_apply", "Time-base set-up (0x4003F2F4, 0x4003F160) and the three 0x40063xxx helpers.", """
void timebase_apply(void)
{
    func_4003f2f4();
    func_4003f160();
    func_40063f04();
    func_40063cd0();
    func_40063c34_all8();
}
""")
q(0x4003F8B8, 40, "runtime_ready_read", "Selector 0x0B bit 0 into SYS_INFO+0x6EE (Hack: SYS_RUNTIME_READY).", """
void runtime_ready_read(void)
{
    fpga_selector_command_write(0x0B);
    SYS8(0x6EE) = fpga_data_byte_read() & 1;
}
""")
q(0x4003F8E0, 48, "pulse_fpga_selector_0x08_0_1", "Selector 0x08: 0 then 1 (runtime re-arm). (name: Hack ACQUISITION.md)", """
void pulse_fpga_selector_0x08_0_1(void)
{
    fpga_selector_command_write(8);
    fpga_data_byte_write(0);
    fpga_selector_command_write(8);
    fpga_data_byte_write(1);
}
""", src="acquisition; name: Hack ACQUISITION.md")
q(0x4003F910, 56, "write_sys_info_0x6f0_to_fpga", "Selector 0x09 <- SYS_INFO+0x6F0 (16 bit, high byte first). (name: Hack ACQUISITION.md)", """
void write_sys_info_0x6f0_to_fpga(void)
{
    fpga_selector_command_write(9);
    fpga_data_byte_write(SYS16(0x6F0) >> 8);
    fpga_data_byte_write(SYS16(0x6F0) & 0xFF);
}
""", src="acquisition; name: Hack ACQUISITION.md")
q(0x4003F948, 64, "fpga_sample_count_write", "Selector 0x0E <- n - 2 (32 bit, MSB first).", """
void fpga_sample_count_write(u32 n)
{
    u32 v = n - 2;
    fpga_selector_command_write(0x0E);
    fpga_data_byte_write((u32)((int)v >> 24) & 0xFF);
    fpga_data_byte_write((v << 8) >> 24);
    fpga_data_byte_write((v << 16) >> 24);
    fpga_data_byte_write(v & 0xFF);
}
""")
q(0x4003F988, 72, "fpga_trigger_pos_write", "Selector 0x20 <- position (16 bit, high byte first), then strobe 0x21: 0, 1.", """
void fpga_trigger_pos_write(u32 v)
{
    fpga_selector_command_write(0x20);
    fpga_data_byte_write((v << 16) >> 24);
    fpga_data_byte_write(v & 0xFF);
    fpga_selector_command_write(0x21);
    fpga_data_byte_write(0);
    fpga_selector_command_write(0x21);
    fpga_data_byte_write(1);
}
""")
q(0x4003F9D0, 40, "acq_done_read", "Selector 0x0A bit 0 into SYS_INFO+0x38 (Hack: SYS_ACQ_DONE).", """
void acq_done_read(void)
{
    fpga_selector_command_write(0x0A);
    SYS8(0x38) = fpga_data_byte_read() & 1;
}
""")
q(0x4003F9F8, 12, "pe0_set", "PE0 high (FPGA bus strobe line).", """
void pe0_set(void)
{
    pio_set_bit((void *)0x01C20890u, 1);
}
""")
q(0x4003FA04, 12, "pe0_clr", "PE0 low.", """
void pe0_clr(void)
{
    pio_clr_bit((void *)0x01C20890u, 1);
}
""")
q(0x4003FA10, 132, "acquisition_prepare",
  "Acquisition start sequence: 0x0F=0, 0x01: 1 then 0, 0x11: 0 then 1, 0x02 <- SYS_INFO+0xBB3, 0x0F=1. (name: Hack STOCK_LIKE_MINIMAL_RUNTIME.md)", """
void acquisition_prepare(void)
{
    fpga_selector_command_write(0x0F); fpga_data_byte_write(0);
    fpga_selector_command_write(0x01); fpga_data_byte_write(1);
    fpga_selector_command_write(0x01); fpga_data_byte_write(0);
    fpga_selector_command_write(0x11); fpga_data_byte_write(0);
    fpga_selector_command_write(0x11); fpga_data_byte_write(1);
    fpga_selector_command_write(0x02); fpga_data_byte_write(SYS8(0xBB3));
    fpga_selector_command_write(0x0F); fpga_data_byte_write(1);
}
""", src="acquisition; name: Hack STOCK_LIKE_MINIMAL_RUNTIME.md")
q(0x4003FA94, 28, "fpga_write_sel_0x1f", "Selector 0x1F <- v.", """
void fpga_write_sel_0x1f(u32 v)
{
    fpga_selector_command_write(0x1F);
    fpga_data_byte_write(v);
}
""")
q(0x4003FAB0, 4, "veneer_400636bc", "Branch to 0x400636BC.", "void veneer_400636bc(void)\n{\n    fpga_cmd1b_write();\n}\n")
q(0x4003FAB4, 4, "veneer_40063714", "Branch to 0x40063714.", "void veneer_40063714(void)\n{\n    fpga_cmd1c_write();\n}\n")
q(0x4003FAE4, 4, "veneer_40063798", "Branch to 0x40063798.", "void veneer_40063798(void)\n{\n    fpga_position_290_update();\n}\n")
q(0x4003FAB8, 44, "trigger_pos_update", "Trigger position = (L + SYS_INFO+0x63C - SYS_INFO+0x6F2) mod L, L = SYS_INFO+0x520, to the FPGA.", """
void trigger_pos_update(void)
{
    u32 L = SYS32(0x520);
    u32 v = L + SYS32(0x63C) - SYS16(0x6F2);
    fpga_trigger_pos_write(v - (v / L) * L);
}
""")
q(0x4003FAE8, 168, "trigger_offsets_update",
  "Trigger offsets: k = (SYS64+0x570 / SYS64+0x590 / SYS+0x564) x SYS+0x560 x SYS+0x558 / SYS+0x1B1 (signed 64-bit divisions); a = SYS+0x2BC / 2 + k; SYS+0x288 = a mod L, SYS+0x28C = (SYS+0x2BC - a) mod L; then 0x400636BC, 0x40063714.", """
void trigger_offsets_update(void)
{
    u32 r4 = SYS32(0x2BC), L, a, b;
    long long t;
    u32 half = (u32)__aeabi_ldivmod((long long)r4, 2);
    t = __aeabi_ldivmod((long long)SYS64(0x570), (long long)SYS64(0x590));
    t = __aeabi_ldivmod(t, (long long)SYS32(0x564));
    t = (long long)((unsigned long long)t * SYS32(0x560));
    t = (long long)((unsigned long long)t * SYS32(0x558));
    t = __aeabi_ldivmod(t, (long long)SYS8(0x1B1));
    L = SYS32(0x520);
    a = (u32)t + half;
    b = r4 - a;
    SYS32(0x288) = a - (a / L) * L;
    SYS32(0x28C) = b - (b / L) * L;
    veneer_400636bc();
    veneer_40063714();
}
""")
q(0x4003FB90, 304, "consumer_translator_of_the_16_bit",
  "Trigger delay in samples (SYS_INFO+0x63C): SYS+0x290 - SYS+0x2BC/2 - k (k as in trigger_offsets_update), raised by whole records L until >= 1; 1 when SYS+0x274 or +0x275 is 0. Then the trigger position to the FPGA and SYS+0x2A0 = SYS+0x2B4/2 + low-word product. (name: Hack FPGA.md)", """
void consumer_translator_of_the_16_bit(void)
{
    long long t, d;
    u32 L, h2, k2;
    if (SYS8(0x274) == 0 || SYS8(0x275) == 0) {
        SYS32(0x63C) = 1;
        return;
    }
    t = __aeabi_ldivmod((long long)SYS32(0x2BC), 2);
    d = (long long)SYS32(0x290) - t;
    t = __aeabi_ldivmod((long long)SYS64(0x570), (long long)SYS64(0x590));
    t = __aeabi_ldivmod(t, (long long)SYS32(0x564));
    t = (long long)((unsigned long long)t * SYS32(0x560));
    t = (long long)((unsigned long long)t * SYS32(0x558));
    t = __aeabi_ldivmod(t, (long long)SYS8(0x1B1));
    d = d - t;
    L = SYS32(0x520);
    while (d < 1)
        d += L;
    SYS32(0x63C) = (u32)d;
    trigger_pos_update();
    h2 = (u32)__aeabi_ldivmod((long long)SYS32(0x2B4), 2);
    t = __aeabi_ldivmod((long long)SYS64(0x570), (long long)SYS64(0x590));
    t = __aeabi_ldivmod(t, (long long)SYS32(0x564));
    k2 = (u32)t * SYS32(0x560);
    k2 = k2 * SYS32(0x558);
    SYS32(0x2A0) = h2 + k2;
}
""", src="acquisition; name: Hack FPGA.md")
q(0x4003FCC0, 80, "acq_wait_done",
  "Polls the acquisition-done flag (selector 0x0A); after 2500 polls each further poll also services the touch panel and gives up on a touch event.", """
void acq_wait_done(void)
{
    int n = 0;
    SYS8(0x38) = 0;
    for (;;) {
        acq_done_read();
        if (SYS8(0x38) != 0)
            return;
        if (++n < 2500)
            continue;
        runtime_event_service();
        if (g_touch->busy2 != 0)
            return;
    }
}
""")
q(0x4003FD10, 260, "amplitude_statistical_normalization",
  "Measurement class for a channel (SYS_INFO+0x51C / +0x51D): 0 when the sample span is below 22 MHz / rate x SYS+0x1B1 or the amplitude / SYS+0x1C0 is below 25, else 2 when the span reaches 250 MHz / rate x SYS+0x1B1 and SYS+0x55C x scale / SYS+0x554 >= 23, else 1. (name: Hack FPGA.md; the code classifies rather than normalises)", """
void amplitude_statistical_normalization(u32 ch)
{
    volatile unsigned char *dst;
    u32 amp, lo, hi, rate, r5, res;
    float s0, s2;
    unsigned long long span, p;
    if (ch == 0) {
        dst = (volatile unsigned char *)(0x40544E68u + 0x51C);
        amp = SYS32(0xB68);
        lo = SYS32(0xB78); hi = SYS32(0xB7C);
        s0 = *(volatile float *)(0x40544E68u + 0xB70);
    } else {
        dst = (volatile unsigned char *)(0x40544E68u + 0x51D);
        amp = SYS32(0xB90);
        s0 = *(volatile float *)(0x40544E68u + 0xB98);
        lo = SYS32(0xBA0); hi = SYS32(0xBA4);
    }
    s0 = (float)SYS32(0x55C) * s0;
    rate = SYS32(0x524);
    s2 = (float)SYS32(0x554);
    p = (unsigned long long)(22000000u / rate) * SYS8(0x1B1);
    s2 = s0 / s2;
    r5 = (u32)(int)s2;
    span = ((unsigned long long)hi << 32) | lo;
    if (span < p)
        res = 0;
    else {
        p = (unsigned long long)(250000000u / rate) * SYS8(0x1B1);
        if (span >= p && (int)r5 >= 23)
            res = 2;
        else
            res = 1;
    }
    *dst = res;
    if ((int)(amp / SYS32(0x1C0)) < 25)
        *dst = 0;
}
""", src="acquisition; name: Hack FPGA.md")
