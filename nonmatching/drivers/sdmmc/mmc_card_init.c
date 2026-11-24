/*
 * mmc_card_init -- stock 0x40068918 (1220 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Pins, bus gate/reset (bit n+8), 400 kHz, controller resets, CMD0; CMD8 decides SD 2.0 (ACMD41 0x40FF8000: SDHC or SD) or SD 1.1 (ACMD41 0x00FF8000) / MMC (CMD1, unreachable: the code compares the result with +0x98); CID, RCA, CSD/CID decode (0x42E68A18), select and 4-bit setup.
 */
#include "hw/platform.h"
#include "drivers/mmio.h"
#include "drivers/sdmmc.h"
#include "drivers/uart.h"
#include "libc.h"
#include "shared/drivers.h"

typedef struct { unsigned short opcode, pad; u32 flags, arg, resp[4]; } mmc_cmd_t;   /* flags: 1 resp, 2 long, 4 crc, 8 busy */
typedef struct { u32 *buf; unsigned char flags, pad[3]; u32 blocks, blksz; } mmc_xfer_t; /* flags: 1 read, 2 write */
int mmc_trans_data_by_cpu(u32 n, mmc_xfer_t *d);
int mmc_trans_data_by_dma(u32 n, mmc_xfer_t *d, u32 *desc);
int mmc_send_cmd(u32 n, mmc_cmd_t *cmd, mmc_xfer_t *data);
int mmc_cmd(u32 n, mmc_cmd_t *cmd, mmc_xfer_t *data, u32 opcode, u32 arg, u32 flags);

void mmc_read_csd_cid(u32 n, mmc_cmd_t *cmd, void *info);
int mmc_select_setup(u32 n, mmc_cmd_t *cmd, int width4);

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
