# FPGA control protocol (PE-pin mode, clocks, identification), SMEC jobs, touch service, power
# (auto power-off, battery coefficients, charger status) and the FatFs disk glue. exec'd by gen_platform.py
FH = '#include "hw/fpga.h"'


def g(addr, size, name, desc, body, src="fpga"):
    f(addr, size, name, src, desc, FH + "\n\n" + body.strip("\n"))


# ---------------------------------------------------------------- FPGA start-up
g(0x40049094, 36, "fpga_pe_mode_select",
  "Shared PE pins: 0 = FPGA control bus (GPIO), other = CSI sample capture. Reprograms the pins only when the mode changes (0x42E68D19).", """
void fpga_pe_mode_select(u32 mode)
{
    if (mode == g_pe_mode)
        return;
    g_pe_mode = mode;
    if (mode == 0)
        fpga_bus_init();
    else
        csi_pins_init_0();
}
""")
g(0x400490B8, 36, "ms5351_start_clocks", "MS5351M outputs: CLK0 95 MHz, CLK1 10 MHz.", """
void ms5351_start_clocks(void)
{
    ms5351_pins_init();
    ms5351_set_clk0(95000000u);
    ms5351_set_clk1(10000000u);
}
""")
g(0x40049188, 48, "FPGA_Start", "CSI set-up, PE pins to FPGA-bus mode, MS5351M clocks. (name: Hack FPGA.md)", """
void fpga_pe_mode_select(u32 mode);
void ms5351_start_clocks(void);

void FPGA_Start(void)
{
    csi_full_init(0x1F40);
    g_pe_mode = 1;
    fpga_pe_mode_select(0);
    ms5351_start_clocks();
}
""", src="fpga; name: Hack FPGA.md")
g(0x400492E4, 72, "perform_fpga_identification_probe",
  "Selector 0x01 returns A5 A5 5A 5A on a working FPGA; result in SYS_INFO+0x7FC. (name: Hack FPGA.md)", """
void perform_fpga_identification_probe(void)
{
    u32 v;
    fpga_selector_command_write(1);
    v = fpga_data_byte_read();
    v = fpga_data_byte_read() | (v << 8);
    v = fpga_data_byte_read() | (v << 8);
    v = fpga_data_byte_read() | (v << 8);
    SYS8(0x7FC) = v == 0xA5A55A5Au;
}
""", src="fpga; name: Hack FPGA.md")

# ---------------------------------------------------------------- touch panel
g(0x400498E4, 340, "gt911_poll",
  "Every 15 ms: GT911 status 0x814E (low nibble = points), first point at 0x8150 (clamped to 1022x600), status cleared; press/release edges latched with their points.", """
void gt911_poll(void)
{
    unsigned char st[4], pt[4];
    u32 x, y, touched;
    volatile touch_t *t = g_touch;

    if (!(g_touch_last + 15 < g_tick_count))
        return;
    g_touch_last = g_tick_count;
    gt911_read16(0x814E, st, 1);
    {
        u32 status = st[0];
        st[0] = 0;
        touched = 0;
        if (status & 0xF) {
            gt911_read16(0x8150, pt, 4);
            gt911_write16(0x814E, st, 1);
            x = pt[0] + (pt[1] << 8);
            y = pt[2] + (pt[3] << 8);
            touched = 1;
            if (x > 0x3FE) x = 0x3FE;
            if (y > 0x258) y = 0x258;
        } else {
            gt911_write16(0x814E, st, 1);
            x = 0;
            y = 0;
        }
    }
    t->x = x;
    t->touched = touched;
    t->y = y;
    if (t->touched == 1 && t->prev == 0) {
        t->down_x = t->x;
        t->down_y = t->y;
        t->event = 1;
    } else if (t->touched == 0 && t->prev == 1) {
        t->up_x = t->prev_x;
        t->up_y = t->prev_y;
        t->event = 2;
    }
    t->prev = t->touched;
    t->prev_x = t->x;
    t->prev_y = t->y;
}
""")
g(0x40049A38, 36, "touch_state_clear", "Clears the touch state block (0x42E68BB0).", """
void touch_state_clear(void)
{
    volatile u32 *w = (volatile u32 *)g_touch;
    w[0] = 0;
    w[1] = 0;
    w[2] = 0;
    *(volatile unsigned short *)((u32)g_touch + 0x0C) = 0;
    *(volatile unsigned char *)((u32)g_touch + 0x0E) = 0;
    w[4] = 0;
}
""")
g(0x40049A5C, 24, "gt911_pins_init", "GT911 I2C and control pins.", """
void gt911_pins_init(void)
{
    gt911_i2c_pins_init();
    gt911_ctrl_pins_out();
}
""")
g(0x40049A74, 40, "touch_init", "Touch controller and state; enables the touch service.", """
void touch_init(void)
{
    gt911_pins_init();
    gt911_bus_init();
    touch_state_clear();
    g_touch->enabled = 1;
}
""")
g(0x40049A9C, 108, "touch_service",
  "UI-loop touch poll: a press/release edge, or a held touch when repeat is on (0x42E61FF8), marks an event for the UI.", """
void touch_service(void)
{
    volatile touch_t *t = g_touch;
    t->serviced = 0;
    if (t->busy != 0)
        return;
    gt911_poll();
    if (t->event != 0 || (t->touched != 0 && g_touch_hold != 0)) {
        t->busy = 1;
        t->busy2 = 1;
    }
    if (t->enabled == 0)
        t->busy2 = 0;
}
""")
g(0x40049B08, 96, "runtime_event_service",
  "Touch poll from the acquisition loop: same event rule as touch_service, also sets the serviced flag. (name: Hack ACQUISITION.md; the code services the touch panel)", """
void runtime_event_service(void)
{
    volatile touch_t *t = g_touch;
    if (t->enabled == 0 || t->busy != 0)
        return;
    gt911_poll();
    if (t->event != 0 || (t->touched != 0 && g_touch_hold != 0)) {
        t->busy = 1;
        t->busy2 = 1;
        t->serviced = 1;
    }
}
""", src="touch; name: Hack ACQUISITION.md")

# ---------------------------------------------------------------- disk glue
g(0x40049B80, 16, "disk_mount", "f_mount(&fs, \"0:\", 1).", """
int disk_mount(void)
{
    return f_mount(g_fatfs0, STR_DRIVE0, 1);
}
""", src="diskio")
g(0x40049B90, 36, "disk_remount", "Unmount \"0:\" and mount it again.", """
int disk_remount(void)
{
    f_mount(0, STR_DRIVE0, 1);
    return disk_mount();
}
""", src="diskio")
g(0x40049BB4, 48, "disk_format", "f_mkfs(\"0:\", FM_FAT, 16 KiB clusters, work 0x428DF874, 1000000).", """
int disk_format(void)
{
    return f_mkfs(STR_DRIVE0, 1, 0x4000, MKFS_WORK, 1000000);
}
""", src="diskio")
g(0x40049BE4, 84, "disk_free_bytes", "Free space of \"0:\" in bytes (free clusters x sectors per cluster x 512), 0 on error.", """
u32 disk_free_bytes(void)
{
    u32 n;
    void *fs = g_fatfs0;
    if (f_getfree(STR_DRIVE0, &n, &fs) != 0)
        return 0;
    return ((*(unsigned short *)((u32)fs + 0x0A) * n) >> 1) << 10;
}
""", src="diskio")
g(0x40049C38, 64, "disk_initialize", "Drive 0 only: card init when the SD driver is ready (returns 0, or 2 on failure), else STA_NOINIT.", """
int disk_initialize(u32 pdrv)
{
    if (pdrv != 0)
        return 1;
    if (g_sd_state != 1)
        return 1;
    return mmc_card_init(0) ? 2 : 0;
}
""", src="diskio")
g(0x40049C78, 112, "disk_read",
  "Reads through the bounce buffer 0x427EB634; copies it to the caller even when the read failed. A wrong drive or an unready card returns 0 (stock behaviour).", """
int disk_read(u32 pdrv, unsigned char *buff, u32 sector, u32 count)
{
    int res = 0;
    if (pdrv == 0 && g_sd_state == 1) {
        if (mmc_read_blocks(0, sector, count, SD_BOUNCE) != 0)
            res = 1;
        memcpy(buff, SD_BOUNCE, g_sector_size * count);
    }
    return res;
}
""", src="diskio")
g(0x40049CE8, 128, "disk_write", "Writes through the bounce buffer; wrong drive -> RES_PARERR, unready card -> 0.", """
int disk_write(u32 pdrv, const unsigned char *buff, u32 sector, u32 count)
{
    if (pdrv != 0)
        return 4;
    if (g_sd_state != 1)
        return 0;
    memcpy(SD_BOUNCE, buff, g_sector_size * count);
    return mmc_write_blocks(0, sector, count, SD_BOUNCE) ? 1 : 0;
}
""", src="diskio")
g(0x40049D68, 112, "disk_ioctl", "CTRL_SYNC, GET_SECTOR_COUNT (when ready), GET_SECTOR_SIZE, GET_BLOCK_SIZE (1).", """
int disk_ioctl(u32 pdrv, u32 cmd, void *buff)
{
    if (pdrv != 0)
        return 4;
    switch (cmd) {
    case 0:
        return 0;
    case 1:
        if (g_sd_state == 1)
            *(u32 *)buff = g_sd_info[1];
        return 0;
    case 2:
        *(unsigned short *)buff = g_sector_size;
        return 0;
    case 3:
        *(u32 *)buff = 1;
        return 0;
    default:
        return 4;
    }
}
""", src="diskio")
g(0x40049DD8, 8, "get_fattime", "Fixed timestamp 0x53756000 (2021-11-21 12:00:00).", """
u32 get_fattime(void)
{
    return 0x53756000u;
}
""", src="diskio")

# ---------------------------------------------------------------- SMEC jobs
g(0x40049E00, 48, "security_chip_check", "Commits job 0xB4, then reads the security-chip status (selector 0x60 bit 0).", """
void security_chip_check(void)
{
    g_smec->pending = 1;
    smec_commit(0xB4);
    fpga_selector_command_write(0x60);
    g_smec->secure_ok = fpga_data_byte_read() & 1;
}
""", src="smec")
g(0x40049E30, 148, "fpga_params_push", "Sends the acquisition parameters (SYS_INFO 0x1B8, 0x1C0, 0x268, 0x270, 0x26C, 0x08, 0x62C) as jobs 1..7.", """
void fpga_params_push(void)
{
    fpga_param_updater(1, 1, SYS32(0x1B8), SYS8(0x1B8));
    fpga_param_updater(2, 4, SYS32(0x1C0), SYS32(0x1C0));
    fpga_param_updater(3, 4, SYS32(0x268), SYS32(0x268));
    fpga_param_updater(4, 4, SYS32(0x270), SYS32(0x270));
    fpga_param_updater(5, 4, SYS32(0x26C), SYS32(0x26C));
    fpga_param_updater(6, 2, SYS32(0x08), SYS16(0x08));
    fpga_param_updater(7, 4, SYS32(0x62C), SYS32(0x62C));
}
""", src="smec")
g(0x40049EC4, 88, "fpga_param_updater",
  "Sends job idx with the value (the 32-bit one for size 4, else the narrow one) unless that value was already sent. (name: Hack SMEC.md)", """
void fpga_param_updater(u32 idx, u32 size, u32 v32, u32 vsmall)
{
    u32 v = size < 4 ? vsmall : v32;
    smec_state_t *s = g_smec;
    if (s->cache[idx] != v || s->valid[idx] == 0) {
        s->valid[idx] = 1;
        s->cache[idx] = v;
        smec_run_job(idx, v);
        s->pending = 1;
    }
}
""", src="smec; name: Hack SMEC.md")
g(0x40049F1C, 40, "smec_commit", "When jobs were sent since the last commit, sends job idx with value 0.", """
void smec_commit(u32 idx)
{
    if (g_smec->pending != 0) {
        smec_run_job(idx, 0);
        g_smec->pending = 0;
    }
}
""", src="smec")
g(0x40049F44, 28, "smec_timing_reset", "Per-index job timing back to 2000 (255 entries).", """
void smec_timing_reset(void)
{
    volatile u32 *p = g_smec_timing;
    int n = 0xFF;
    do {
        *p++ = 0x7D0;
    } while (--n);
}
""", src="smec")
g(0x40049F60, 28, "selector_0x6a_one_byte_job_index", "Job index (selector 0x6A). (name: Hack SMEC.md)", """
void selector_0x6a_one_byte_job_index(u32 v)
{
    fpga_selector_command_write(0x6A);
    fpga_data_byte_write(v);
}
""", src="smec; name: Hack SMEC.md")
g(0x40049F7C, 60, "selector_0x6b_32_bit_job_value", "Job value, MSB first (selector 0x6B). (name: Hack SMEC.md)", """
void selector_0x6b_32_bit_job_value(u32 v)
{
    fpga_selector_command_write(0x6B);
    fpga_data_byte_write(v >> 24);
    fpga_data_byte_write((v << 8) >> 24);
    fpga_data_byte_write((v << 16) >> 24);
    fpga_data_byte_write(v & 0xFF);
}
""", src="smec; name: Hack SMEC.md")
g(0x40049FB8, 36, "selector_0x69_16_bit_timing_retry", "Job timing, high byte then low byte (selector 0x69). (name: Hack SMEC.md)", """
void selector_0x69_16_bit_timing_retry(u32 v)
{
    fpga_selector_command_write(0x69);
    fpga_data_byte_write(v >> 8);
    fpga_data_byte_write(v & 0xFF);
}
""", src="smec; name: Hack SMEC.md")
g(0x40049FDC, 68, "smec_start_job", "Start (0x68 = 1), wait for selector 0x64 bit 0, release (0x68 = 0).", """
void smec_start_job(void)
{
    fpga_selector_command_write(0x68);
    fpga_data_byte_write(1);
    do {
        fpga_selector_command_write(0x64);
    } while (!(fpga_data_byte_read() & 1));
    fpga_selector_command_write(0x68);
    fpga_data_byte_write(0);
}
""", src="smec")
g(0x4004A020, 32, "smec_job_valid", "Result-valid flag (selector 0x67 bit 0).", """
u32 smec_job_valid(void)
{
    fpga_selector_command_write(0x67);
    return fpga_data_byte_read() & 1;
}
""", src="smec")
g(0x4004A040, 36, "smec_wait_ready", "Waits for selector 0x65 bit 0.", """
void smec_wait_ready(void)
{
    do {
        fpga_selector_command_write(0x65);
    } while (!(fpga_data_byte_read() & 1));
}
""", src="smec")
g(0x4004A064, 28, "smec_job_echo", "Job index echoed by the FPGA (selector 0x68).", """
u32 smec_job_echo(void)
{
    fpga_selector_command_write(0x68);
    return fpga_data_byte_read();
}
""", src="smec")
g(0x4004A080, 20, "smec_job_result", "32-bit job result, MSB first (selector 0x66). The stock body continues at 0x4004A094 (split by the analyser).", """
u32 smec_job_result(void)
{
    u32 v;
    fpga_selector_command_write(0x66);
    v = fpga_data_byte_read();
    v = fpga_data_byte_read() | (v << 8);
    v = fpga_data_byte_read() | (v << 8);
    return fpga_data_byte_read() | (v << 8);
}
""", src="smec")
g(0x4004A0B0, 140, "smec_run_job",
  "Sends job (idx, val) and returns its result. On a missing echo or invalid result the index timing grows by 100 up to 3000; 4 more tries at 3000, then 0.", """
u32 smec_run_job(u32 idx, u32 val)
{
    volatile u32 *t = g_smec_timing;
    int tries = 0;
    for (;;) {
        u32 echo, ok, v;
        selector_0x69_16_bit_timing_retry(t[idx] & 0xFFFF);
        selector_0x6a_one_byte_job_index(idx);
        selector_0x6b_32_bit_job_value(val);
        smec_start_job();
        smec_wait_ready();
        echo = smec_job_echo();
        ok = smec_job_valid();
        if (echo == idx && ok != 0)
            return smec_job_result();
        v = t[idx] + 100;
        t[idx] = v;
        if (v <= 3000)
            continue;
        tries++;
        t[idx] = 3000;
        if (tries >= 4)
            return 0;
    }
}
""", src="smec")
g(0x4004A13C, 32, "smec_reset_cmd", "Writes 8 to selector 0x67.", """
void smec_reset_cmd(void)
{
    fpga_selector_command_write(0x67);
    fpga_data_byte_write(8);
}
""", src="smec")
g(0x4004A15C, 48, "SMEC_Init", "Clears the job state, timing 2000, reset command, sends the acquisition parameters. (name: Hack SMEC.md)", """
void SMEC_Init(void)
{
    __aeabi_memclr4(g_smec, 0x8FC);
    smec_timing_reset();
    smec_reset_cmd();
    selector_0x69_16_bit_timing_retry(0x7D0);
    fpga_params_push();
}
""", src="smec; name: Hack SMEC.md")

# ---------------------------------------------------------------- power
g(0x4004A198, 28, "power_monitor_init", "LRADC (battery) set-up, auto power-off deadline, default battery coefficients.", """
void power_monitor_init(void)
{
    lradc_init();
    auto_off_arm();
    battery_coeffs_default();
}
""", src="power")
g(0x4004A1D0, 36, "auto_off_arm", "Auto power-off deadline = tick + minutes (SYS_INFO+0x658) x 60000 ms, kept in SYS_INFO+0x65C.", """
void auto_off_arm(void)
{
    SYS32(0x65C) = g_tick_count + 60000u * SYS32(0x658);
}
""", src="power")
g(0x4004A1F4, 132, "auto_off_check",
  "When auto power-off is on and its deadline passed: draws the shutdown image at the screen centre, waits 3 s and powers off.", """
void auto_off_check(void)
{
    u32 cx = SYS16(4) + (SYS16(8) >> 1);
    u32 cy = SYS16(6) + (SYS16(0x0A) >> 1);
    if (SYS32(0x658) != 0 && SYS32(0x65C) < g_tick_count) {
        unsigned char *p = (unsigned char *)(0x42C2FE74u + 0xDB60);
        func_40034134(cx + (signed char)p[0x194], cy + (signed char)p[0x195], *(u32 *)p, 0);
        delay_ms(3000);
        power_off();
    }
}
""", src="power")
g(0x4004A278, 40, "charge_status_read", "Charger status (selector 0x02 bit 0) into SYS_INFO+0x779.", """
void charge_status_read(void)
{
    fpga_selector_command_write(2);
    SYS8(0x779) = fpga_data_byte_read() & 1;
}
""", src="power")
g(0x4004A2A0, 48, "battery_coeffs_default",
  "Default battery conversion: offset 170.0, slope 32.947, base 2565.74 (float; mV = slope x LRADC + base + offset).", """
void battery_coeffs_default(void)
{
    *(volatile u32 *)0x42E68C9Cu = 0x432A0000u;   /* 170.0f */
    *(volatile u32 *)0x42E68C94u = 0x4203CA1Bu;   /* 32.947f */
    *(volatile u32 *)0x42E68C98u = 0x45205BCAu;   /* 2565.74f */
}
""", src="power")
