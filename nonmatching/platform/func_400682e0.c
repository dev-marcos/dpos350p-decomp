/*
 * mmc_switch_hs -- stock 0x400682E0 (276 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * CMD6 check (0x00FFFF01) then switch (0x80FFFF01) to high speed, 64-byte status in 0x42E68B28. 1 = not supported/busy, 0 = switched, else the error.
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
