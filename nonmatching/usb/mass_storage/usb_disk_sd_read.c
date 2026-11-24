/*
 * usb_disk_sd_read -- stock 0x4006A738 (304 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * READ(10) data stage from the SD card, one 512-byte packet per call. The first call (state byte 0x42E68D1B = 0) stores the block count in 0x42E68C84 and starts; the first packet of a transfer reads all count blocks from sector lba with mmc_read_blocks(0, lba, count, 0x42E08074) (a failure sets bit 3 of 0x42E68D1C) and sends the start of the buffer, later ones send the next 512 bytes (offset word 0x42E68C80) with usb_mass_write(1, ..., 512). Each packet takes 512 from the residue word 0x42E68C10 + 8 and one from the remaining count. When none remain the state resets and 0x42E68D1D = 3 (send the status). Returns the remaining count.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "drivers/sdmmc.h"
#include "usb/mass_storage.h"
#include "shared/access.h"

/* Same functions, declared the way the stock code calls them (see the definitions for the
 * full interface): the C must pass exactly these registers. */
void usb_mass_write_raw(u32 ep, u32 buf, u32 n) __asm__("usb_mass_write");

#define ST 0x42E68D1Bu               /* data stage state: 0 idle, 1 busy */
#define ERR 0x42E68D1Cu              /* error bits: 8 read, 4 write */
#define CSWST 0x42E68D1Du            /* command state: 3 send status, 4 host ended the data */
#define USB 0x42E68C10u              /* +8: data residue */
#define BUF 0x42E08074u              /* sector bounce buffer */

#define DONE 0x42E68C7Cu             /* packets sent */
#define LEFT 0x42E68C84u             /* blocks left */
#define OFF 0x42E68C80u              /* buffer offset of the next packet */

u32 usb_disk_sd_read(u32 a, u32 lba, u32 count, u32 d)
{
    u32 s = B8(ST), left;

    if (s == 0) {
        W32(LEFT) = count;
        W32(DONE) = 0;
        B8(ST) = 1;
        s = 1;
    }
    if (s == 1) {
        if (W32(DONE) == 0) {
            if ((unsigned char)mmc_read_blocks(0, lba, count, (void *)BUF))
                B8(ERR) |= 8;
            usb_mass_write_raw(1, BUF, 0x200);
            left = W32(LEFT) - 1;
            W32(DONE) = W32(DONE) + 1;
            W32(OFF) = 0x200;
        } else {
            usb_mass_write_raw(1, BUF + W32(OFF), 0x200);
            left = W32(LEFT) - 1;
            W32(DONE) = W32(DONE) + 1;
            W32(OFF) = W32(OFF) + 0x200;
        }
        W32(LEFT) = left;
        W32(USB + 8) = W32(USB + 8) - 0x200;
    }
    left = W32(LEFT);
    if (left)
        return left;
    W32(DONE) = 0;
    W32(OFF) = 0;
    B8(CSWST) = 3;
    B8(ST) = 0;
    usb_disk_hook();
    return 0;
}
