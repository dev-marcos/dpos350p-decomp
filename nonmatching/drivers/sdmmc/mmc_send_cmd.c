/*
 * mmc_send_cmd -- stock 0x40067D70 (832 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Issue a command (CMD12 is skipped): response/CRC/init flags, data phase by CPU (<= 64 bytes) or IDMAC (0x42E5D010 descriptors), command-done and data-over waits (1 000 000), busy wait (2 000 000 us), responses; on error reset the controller and update the clock; always clear RINTSTS and reset FIFO/DMA; flush the cache after a read.
 */
#include "hw/platform.h"
#include "drivers/mmio.h"
#include "drivers/sdmmc.h"
#include "drivers/uart.h"
#include "shared/drivers.h"

typedef struct { unsigned short opcode, pad; u32 flags, arg, resp[4]; } mmc_cmd_t;   /* flags: 1 resp, 2 long, 4 crc, 8 busy */
typedef struct { u32 *buf; unsigned char flags, pad[3]; u32 blocks, blksz; } mmc_xfer_t; /* flags: 1 read, 2 write */
int mmc_trans_data_by_cpu(u32 n, mmc_xfer_t *d);
int mmc_trans_data_by_dma(u32 n, mmc_xfer_t *d, u32 *desc);
int mmc_send_cmd(u32 n, mmc_cmd_t *cmd, mmc_xfer_t *data);
int mmc_cmd(u32 n, mmc_cmd_t *cmd, mmc_xfer_t *data, u32 opcode, u32 arg, u32 flags);

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
