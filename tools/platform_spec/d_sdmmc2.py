# SD/MMC controller driver, part 2 (commands, card init, block I/O).  exec'd by gen_platform.py
M2H = """
typedef struct { unsigned short opcode, pad; u32 flags, arg, resp[4]; } mmc_cmd_t;   /* flags: 1 resp, 2 long, 4 crc, 8 busy */
typedef struct { u32 *buf; unsigned char flags, pad[3]; u32 blocks, blksz; } mmc_xfer_t; /* flags: 1 read, 2 write */
u32 readl_a(u32 addr);
void writel_a(u32 addr, u32 v);
u32 mmc_base(u32 n);
void mmc_delay_us(u32 us);
void mmc_delay_ms(u32 ms);
int mmc_update_clk(u32 n);
int mmc_wait_rint(u32 n, u32 timeout, u32 mask);
int mmc_idma_status(u32 n);
int mmc_trans_data_by_cpu(u32 n, mmc_xfer_t *d);
int mmc_trans_data_by_dma(u32 n, mmc_xfer_t *d, u32 *desc);
int mmc_send_cmd(u32 n, mmc_cmd_t *cmd, mmc_xfer_t *data);
int mmc_cmd(u32 n, mmc_cmd_t *cmd, mmc_xfer_t *data, u32 opcode, u32 arg, u32 flags);
int mmc_config_clock(u32 n, u32 hz);
int mmc_set_bus_width(u32 n, u32 ctype);
int mmc_check_range(u32 start, u32 count);
void dcache_flush_range(u32 addr, u32 len);
#define mmc_card_type (*(int *)0x42E68CB8u)   /* 0 none, 1 SDHC, 2 SD (byte addressed), 3 MMC */
#define mmc_rca       (*(u32 *)0x42E68CBCu)
#define mmc_dma_buf   ((unsigned char *)0x42E68B28u)
"""


def s2(addr, size, name, desc, body):
    f(addr, size, name, "SD/MMC", desc, M2H.strip("\n") + "\n\n" + body.strip("\n"))


s2(0x40067D70, 832, "mmc_send_cmd",
   "Issue a command (CMD12 is skipped): response/CRC/init flags, data phase by CPU (<= 64 bytes) or IDMAC (0x42E5D010 descriptors), command-done and data-over waits (1 000 000), busy wait (2 000 000 us), responses; on error reset the controller and update the clock; always clear RINTSTS and reset FIFO/DMA; flush the cache after a read.", """
int mmc_send_cmd(u32 n, mmc_cmd_t *cmd, mmc_xfer_t *data)
{
    u32 base = mmc_base(n), v = 0x80000000u, fl, t;
    int r, dma = 0;
    if (cmd->opcode == 12)
        return 0;
    fl = cmd->flags;
    if (cmd->opcode == 0) v |= 0x8000;
    if (fl & 1) v |= 0x40;
    if (fl & 2) v |= 0x80;
    if (fl & 4) v |= 0x100;
    if (data) {
        if ((u32)data->buf & 3) {
            r = -1;
            dbg_printf(STR(0x4008E76C));
            goto reset;
        }
        v |= 0x2200;
        if (data->flags & 2) v |= 0x400;
        if (data->blocks >= 2) v |= 0x1000;
        writel_a(base + 0x10, data->blksz);
        writel_a(base + 0x14, data->blocks * data->blksz);
    }
    writel_a(base + 0x1C, cmd->arg);
    if (data) {
        if (data->blksz * data->blocks > 0x40) {
            dma = 1;
            mmc_trans_data_by_dma(n, data, (u32 *)0x42E5D010u);
        } else {
            writel_a(base + 0x18, cmd->opcode | v);
            if (mmc_trans_data_by_cpu(n, data)) {
                r = -0x98;
                dbg_printf(STR(0x4008F84C), readl_a(base + 0x38) & 0xBFC2);
                goto reset;
            }
            goto wait;
        }
    }
    writel_a(base + 0x18, cmd->opcode | v);
wait:
    r = mmc_wait_rint(n, 1000000, 4);
    if (r) goto done;
    if (dma) {
        t = 1000000;
        do {
            if (mmc_idma_status(n)) break;
        } while (--t);
    }
    if (data) {
        r = mmc_wait_rint(n, 1000000, data->blocks >= 2 ? 0x4000 : 8);
        if (r) goto done;
    }
    if (*(unsigned char *)&cmd->flags & 8) {
        t = 2000000;
        for (;;) {
            u32 st = readl_a(base + 0x3C);
            if (t == 0) {
                r = -0x98;
                dbg_printf(STR(0x4008F85C));
                goto reset;
            }
            t--;
            mmc_delay_us(1);
            if (!(st & 0x200)) break;
        }
    }
    if (*(unsigned char *)&cmd->flags & 2) {
        cmd->resp[0] = readl_a(base + 0x20);
        cmd->resp[1] = readl_a(base + 0x24);
        cmd->resp[2] = readl_a(base + 0x28);
        cmd->resp[3] = readl_a(base + 0x2C);
    } else {
        cmd->resp[0] = readl_a(base + 0x20);
        cmd->resp[1] = readl_a(base + 0x24);
    }
done:
    if (r >= 0) goto cleanup;
reset:
    dbg_printf(STR(0x4008F86C));
    writel_a(base, 7);
    mmc_update_clk(n);
cleanup:
    if (data && dma) {
        writel_a(base + 0x88, readl_a(base + 0x88));
        writel_a(base + 0x8C, 0);
        writel_a(base + 0x80, 0);
    }
    writel_a(base + 0x38, readl_a(base + 0x38));
    writel_a(base, readl_a(base) | 6);
    if (data && (data->flags & 1))
        dcache_flush_range((u32)data->buf, data->blocks * data->blksz);
    return r;
}
""")
s2(0x400680B0, 92, "mmc_hw_reset", "HWRST (+0x78): pulse bit 1 low for 5 ms, then wait 1 ms.", """
void mmc_hw_reset(u32 n)
{
    u32 r = mmc_base(n) + 0x78;
    writel_a(r, readl_a(r) | 2);
    writel_a(r, readl_a(r) & ~2u);
    mmc_delay_ms(5);
    writel_a(r, readl_a(r) | 2);
    mmc_delay_ms(1);
}
""")
s2(0x4006810C, 40, "mmc_cmd", "Fill opcode/arg/flags and send.", """
int mmc_cmd(u32 n, mmc_cmd_t *cmd, mmc_xfer_t *data, u32 opcode, u32 arg, u32 flags)
{
    cmd->opcode = (unsigned short)opcode;
    cmd->arg = arg;
    cmd->flags = flags;
    return mmc_send_cmd(n, cmd, data);
}
""")
s2(0x400682E0, 276, "mmc_switch_hs", "CMD6 check (0x00FFFF01) then switch (0x80FFFF01) to high speed, 64-byte status in 0x42E68B28. 1 = not supported/busy, 0 = switched, else the error.", """
int mmc_switch_hs(u32 n, mmc_cmd_t *cmd)
{
    mmc_xfer_t d;
    int r;
    d.buf = (u32 *)mmc_dma_buf; *(u32 *)&d.flags = 1; d.blocks = 1; d.blksz = 0x40;
    r = mmc_cmd(n, cmd, &d, 6, 0x00FFFF01u, 5);
    if (r) return r;
    if (!(mmc_dma_buf[1] | mmc_dma_buf[0] << 8)) return 1;
    if (mmc_dma_buf[0x1D] | mmc_dma_buf[0x1C] << 8) return 1;
    d.buf = (u32 *)mmc_dma_buf; *(u32 *)&d.flags = 1; d.blocks = 1; d.blksz = 0x40;
    r = mmc_cmd(n, cmd, &d, 6, 0x80FFFF01u, 5);
    if (r) return r;
    if (!(mmc_dma_buf[1] | mmc_dma_buf[0] << 8)) return 1;
    if (!(mmc_dma_buf[0x1D] | mmc_dma_buf[0x1C] << 8))
        dbg_printf(STR(0x4008D8F4));
    return 0;
}
""")
s2(0x4006840C, 576, "mmc_select_setup", "Select the card (CMD7); SDHC: SCR, high-speed switch then 25 MHz; SD: 50 MHz; optional 4-bit bus (ACMD6); block length 512 (CMD16); deselect. 0 or the first command error.", """
int mmc_switch_hs(u32 n, mmc_cmd_t *cmd);

int mmc_select_setup(u32 n, mmc_cmd_t *cmd, int width4)
{
    mmc_xfer_t d;
    int r;
    dbg_printf(STR(0x4008F87C));
    r = mmc_cmd(n, cmd, 0, 7, mmc_rca, 5);
    if (r) return r;
    if (mmc_card_type == 1) {
        dbg_printf(STR(0x4008E784));
        r = mmc_cmd(n, cmd, 0, 0x37, mmc_rca, 5);
        if (r) return r;
        d.buf = (u32 *)mmc_dma_buf; *(u32 *)&d.flags = 1; d.blocks = 1; d.blksz = 8;
        r = mmc_cmd(n, cmd, &d, 0x33, 0, 5);
        if (r) return r;
        if ((mmc_dma_buf[0] & 0xF) == 2 && mmc_switch_hs(n, cmd) == 0) {
            dbg_printf(STR(0x4008F88C));
            mmc_config_clock(n, 25000000);
            mmc_delay_ms(10);
        }
    } else if (mmc_card_type == 2) {
        dbg_printf(STR(0x4008EE24));
        mmc_config_clock(n, 50000000);
        mmc_delay_ms(10);
    } else {
        goto blk;
    }
    if (width4 == 1) {
        dbg_printf(STR(0x4008EE10));
        r = mmc_cmd(n, cmd, 0, 0x37, mmc_rca, 5);
        if (r) return r;
        r = mmc_cmd(n, cmd, 0, 6, 2, 5);
        if (r) return r;
        mmc_set_bus_width(n, 1);
    }
blk:
    dbg_printf(STR(0x4008E79C));
    mmc_cmd(n, cmd, 0, 0x10, 0x200, 0xD);
    mmc_cmd(n, cmd, 0, 7, 0, 0);
    return 0;
}
""")
for a, name, rd, ops, flag, align in ((0x4006866C, "mmc_read_blocks", True, (0x11, 0x12), 1, "0x4008D918"),
                                      (0x40068794, "mmc_write_blocks", False, (0x18, 0x19), 2, "0x4008E76C")):
    s2(a, 284 if rd else 288, name,
       f"{'Read' if rd else 'Write'} count sectors: range check, non-NULL word-aligned buffer, select (CMD7), CMD{ops[0]}/{ops[1]} (byte address for non-SDHC), deselect. 0 or an error code." + (" The data command's result is ignored." if rd else ""), f"""
int {name}(u32 n, u32 sector, u32 count, u32 *buf)
{{
    mmc_cmd_t cmd;
    mmc_xfer_t d;
    u32 op;
    int r = mmc_check_range(sector, count);
    if (r < 0) return r;
    if (!buf) {{
        dbg_printf(STR(0x4008D1DC));
        return (int)0xFFF4A025u;
    }}
    if ((u32)buf & 3) {{
        dbg_printf(STR({align}){', (u32)buf' if rd else ''});
        return (int)0xFFF8C2EFu;
    }}
    if (mmc_card_type != 1) sector <<= 9;
    op = count == 1 ? 0x{ops[0]:X} : 0x{ops[1]:X};
    r = mmc_cmd(n, &cmd, 0, 7, mmc_rca, 0xD);
    if (r) return r;
    d.buf = buf; *(u32 *)&d.flags = {flag}; d.blocks = count; d.blksz = 0x200;
    {"mmc_cmd(n, &cmd, &d, op, sector, 0xD);" if rd else "r = mmc_cmd(n, &cmd, &d, op, sector, 0xD);"}
    {"" if rd else "if (r) return r;"}
    mmc_cmd(n, &cmd, 0, 7, 0, 0);
    return 0;
}}
""")
s2(0x40068918, 1220, "mmc_card_init",
   "Pins, bus gate/reset (bit n+8), 400 kHz, controller resets, CMD0; CMD8 decides SD 2.0 (ACMD41 0x40FF8000: SDHC or SD) or SD 1.1 (ACMD41 0x00FF8000) / MMC (CMD1, unreachable: the code compares the result with +0x98); CID, RCA, CSD/CID decode (0x42E68A18), select and 4-bit setup.", """
void sdmmc_pins_init(u32 n);
int mmc_set_mod_clk(u32 n, u32 hz);
void mmc_hw_reset(u32 n);
int mmc_reset(u32 n);
int mmc_gctrl_clr_bit8(u32 n);
int mmc_clk_lowpower_on(u32 n);
int mmc_clk_enable(u32 n);
void mmc_read_csd_cid(u32 n, mmc_cmd_t *cmd, void *info);
int mmc_select_setup(u32 n, mmc_cmd_t *cmd, int width4);
unsigned long long __aeabi_llsl(unsigned long long v, int sh);

int mmc_card_init(u32 n)
{
    mmc_cmd_t cmd;
    u32 base = mmc_base(n), gate, bit, *cid = (u32 *)0x42E68C00u;
    const char *s;
    int r;
    sdmmc_pins_init(n);
    gate = readl_a(0x01C20060u);
    bit = (u32)__aeabi_llsl(1, (int)n + 8);
    writel_a(0x01C20060u, bit | gate);
    writel_a(0x01C202C0u, bit | readl_a(0x01C202C0u));
    mmc_delay_ms(20);
    mmc_set_mod_clk(n, 400000);
    mmc_hw_reset(n);
    mmc_reset(n);
    mmc_gctrl_clr_bit8(n);
    mmc_clk_lowpower_on(n);
    mmc_set_bus_width(n, 0);
    writel_a(base + 8, 0xFFFFFFFFu);
    mmc_clk_enable(n);
    mmc_delay_ms(2);
    mmc_cmd(n, &cmd, 0, 0, 0, 0);
    mmc_delay_ms(20);
    r = mmc_cmd(n, &cmd, 0, 8, 0x155, 5);
    mmc_delay_ms(30);
    if (r == 0) {
        dbg_printf(STR(0x4008FF54));
        mmc_cmd(n, &cmd, 0, 0x37, 0, 5);
        mmc_cmd(n, &cmd, 0, 0x29, 0x40FF8000u, 7);
        while (!(cmd.resp[3] & 0x80000000u)) {
            mmc_cmd(n, &cmd, 0, 0x37, 0, 5);
            if (mmc_cmd(n, &cmd, 0, 0x29, 0x40FF8000u, 7)) break;
        }
        if (cmd.resp[3] & 0x40000000u) {
            mmc_card_type = 1;
            s = STR(0x4008EE38);
        } else {
            mmc_card_type = 2;
            s = STR(0x4008EE4C);
        }
    } else {
        dbg_printf(STR(0x4008FF60));
        mmc_cmd(n, &cmd, 0, 0, 0, 0);
        mmc_delay_ms(20);
        r = mmc_cmd(n, &cmd, 0, 0x37, 0, 5);
        if (r == 0x98) {
            mmc_cmd(n, &cmd, 0, 0, 0, 0);
            mmc_delay_ms(2);
            if (mmc_cmd(n, &cmd, 0, 1, 0x80FF8000u, 5) == 0x98) {
                mmc_card_type = 0;
                dbg_printf(STR(0x4008F8AC));
                return -1;
            }
            while (!(cmd.resp[3] & 0x80000000u))
                if (mmc_cmd(n, &cmd, 0, 1, 0x80FF8000u, 5)) break;
            mmc_card_type = 3;
            s = STR(0x4008F89C);
        } else if (r != 0) {
            mmc_card_type = 0;
            dbg_printf(STR(0x4008F8BC));
            return -1;
        } else {
            mmc_cmd(n, &cmd, 0, 0x29, 0x00FF8000u, 7);
            while (!(cmd.resp[3] & 0x80000000u)) {
                mmc_cmd(n, &cmd, 0, 0x37, 0, 5);
                if (mmc_cmd(n, &cmd, 0, 0x29, 0x00FF8000u, 7)) break;
            }
            mmc_card_type = 2;
            s = STR(0x4008EE4C);
        }
    }
    dbg_printf(s);
    if (mmc_card_type != 0) {
        mmc_cmd(n, &cmd, 0, 2, 0, 7);
        cid[0] = cmd.resp[3];
        cid[1] = cmd.resp[2];
        cid[2] = cmd.resp[1];
        cid[3] = cmd.resp[0];
        if (mmc_card_type == 3) {
            if (mmc_cmd(n, &cmd, 0, 3, 0x10000, 7)) return -1;
            mmc_rca = 0x10000;
        } else {
            if (mmc_cmd(n, &cmd, 0, 3, 0, 7)) return -1;
            mmc_rca = cmd.resp[3];
        }
    }
    mmc_read_csd_cid(n, &cmd, (void *)0x42E68A18u);
    mmc_select_setup(n, &cmd, 1);
    return 0;
}
""")
f(0x4006A530, 32, "__aeabi_llsl", "AEABI", "64-bit shift left (shift taken mod 256; ARM register-shift semantics).", """
unsigned long long __aeabi_llsl(unsigned long long v, int sh)
{
    u32 lo = (u32)v, hi = (u32)(v >> 32), n = (u32)sh & 0xFF, s;
    if (n >= 32) {
        s = n - 32;
        hi = s < 32 ? lo << s : 0;
    } else {
        hi = (hi << n) | (n ? lo >> (32 - n) : 0);
    }
    lo = n < 32 ? lo << n : 0;
    return (unsigned long long)hi << 32 | lo;
}
""")
