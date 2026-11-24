/*
 * usb_disk_sd_write -- stock 0x4006A868 (252 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * WRITE(10) data stage to the SD card, one 512-byte packet per call. The first call (state byte 0x42E68D1B = 0) stores lba (0x42E68C88) and the block count (0x42E68C8C). Each call copies the received packet (pointer word 0x42E68CE0) to 0x42E08074 + 512 x n (n = word 0x42E68C90, then n + 1) and takes half 0x42E68D16 from the residue word 0x42E68C10 + 8. Once all blocks are in, or when 0x42E68D1D is 4, mmc_write_blocks(0, lba, count, 0x42E08074) writes them (a failure sets bit 2 of 0x42E68D1C), n = 0, usb_mass_send_csw(0, 1) and the state resets; the result is then the one of usb_mass_send_csw, otherwise n.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "drivers/sdmmc.h"
#include "usb/mass_storage.h"
#include "util.h"
#include "shared/access.h"

/* Same functions, declared the way the stock code calls them (see the definitions for the
 * full interface): the C must pass exactly these registers. */
int mmc_write_blocks_raw(int, u32, u32, const void *) __asm__("mmc_write_blocks");
u32 usb_mass_send_csw_raw(u32 a, u32 b) __asm__("usb_mass_send_csw");

#define ST 0x42E68D1Bu               /* data stage state: 0 idle, 1 busy */
#define ERR 0x42E68D1Cu              /* error bits: 8 read, 4 write */
#define CSWST 0x42E68D1Du            /* command state: 3 send status, 4 host ended the data */
#define USB 0x42E68C10u              /* +8: data residue */
#define BUF 0x42E08074u              /* sector bounce buffer */

#define LBA 0x42E68C88u
#define TOTAL 0x42E68C8Cu
#define N 0x42E68C90u

u32 usb_disk_sd_write(u32 a, u32 lba, u32 count, u32 d)
{
    u32 s = B8(ST), r;

    if (s == 0) {
        W32(LBA) = lba;
        W32(TOTAL) = count;
        W32(N) = 0;
        B8(ST) = 1;
        s = 1;
    }
    if (s == 1) {
        memcpy((void *)(BUF + (W32(N) << 9)), (const void *)W32(0x42E68CE0u), 0x200);
        W32(N) = W32(N) + 1;
        W32(USB + 8) = W32(USB + 8) - H16(0x42E68D16u);
    }
    if (W32(N) < W32(TOTAL) && B8(CSWST) != 4)
        return W32(N);
    if ((unsigned char)mmc_write_blocks_raw(0, W32(LBA), W32(TOTAL), (const void *)BUF))
        B8(ERR) |= 4;
    W32(N) = 0;
    r = usb_mass_send_csw_raw(0, 1);
    B8(ST) = 0;
    usb_disk_hook();
    return r;
}
