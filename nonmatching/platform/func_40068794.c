/*
 * mmc_write_blocks -- stock 0x40068794 (288 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Write count sectors: range check, non-NULL word-aligned buffer, select (CMD7), CMD24/25 (byte address for non-SDHC), deselect. 0 or an error code.
 */
#include "hw/platform.h"

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

int mmc_write_blocks(u32 n, u32 sector, u32 count, u32 *buf)
{
    mmc_cmd_t cmd;
    mmc_xfer_t d;
    u32 op;
    int r = mmc_check_range(sector, count);
    if (r < 0) return r;
    if (!buf) {
        dbg_printf(STR(0x4008D1DC));
        return (int)0xFFF4A025u;
    }
    if ((u32)buf & 3) {
        dbg_printf(STR(0x4008E76C));
        return (int)0xFFF8C2EFu;
    }
    if (mmc_card_type != 1) sector <<= 9;
    op = count == 1 ? 0x18 : 0x19;
    r = mmc_cmd(n, &cmd, 0, 7, mmc_rca, 0xD);
    if (r) return r;
    d.buf = buf; *(u32 *)&d.flags = 2; d.blocks = count; d.blksz = 0x200;
    r = mmc_cmd(n, &cmd, &d, op, sector, 0xD);
    if (r) return r;
    mmc_cmd(n, &cmd, 0, 7, 0, 0);
    return 0;
}
