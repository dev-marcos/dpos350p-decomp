/*
 * mmc_select_setup -- stock 0x4006840C (576 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Select the card (CMD7); SDHC: SCR, high-speed switch then 25 MHz; SD: 50 MHz; optional 4-bit bus (ACMD6); block length 512 (CMD16); deselect. 0 or the first command error.
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

int mmc_switch_hs(u32 n, mmc_cmd_t *cmd);

int mmc_select_setup(u32 n, mmc_cmd_t *cmd, int width4)
{
    mmc_xfer_t d;
    int r;
    dbg_printf(STR(4008f87c));
    r = mmc_cmd(n, cmd, 0, 7, mmc_rca, 5);
    if (r) return r;
    if (mmc_card_type == 1) {
        dbg_printf(STR(4008e784));
        r = mmc_cmd(n, cmd, 0, 0x37, mmc_rca, 5);
        if (r) return r;
        d.buf = (u32 *)mmc_dma_buf; *(u32 *)&d.flags = 1; d.blocks = 1; d.blksz = 8;
        r = mmc_cmd(n, cmd, &d, 0x33, 0, 5);
        if (r) return r;
        if ((mmc_dma_buf[0] & 0xF) == 2 && mmc_switch_hs(n, cmd) == 0) {
            dbg_printf(STR(4008f88c));
            mmc_config_clock(n, 25000000);
            mmc_delay_ms(10);
        }
    } else if (mmc_card_type == 2) {
        dbg_printf(STR(4008ee24));
        mmc_config_clock(n, 50000000);
        mmc_delay_ms(10);
    } else {
        goto blk;
    }
    if (width4 == 1) {
        dbg_printf(STR(4008ee10));
        r = mmc_cmd(n, cmd, 0, 0x37, mmc_rca, 5);
        if (r) return r;
        r = mmc_cmd(n, cmd, 0, 6, 2, 5);
        if (r) return r;
        mmc_set_bus_width(n, 1);
    }
blk:
    dbg_printf(STR(4008e79c));
    mmc_cmd(n, cmd, 0, 0x10, 0x200, 0xD);
    mmc_cmd(n, cmd, 0, 7, 0, 0);
    return 0;
}
