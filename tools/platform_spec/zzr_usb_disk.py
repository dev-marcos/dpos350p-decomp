# USB mass-storage data stage on the SD card: the READ(10) source and the WRITE(10) sink behind
# usb_mass_disk_read / usb_mass_disk_write, and the empty hook they end with.
# exec'd by gen_platform.py (do not define HDR, SPEC, OUT or f here)


def wr(addr, size, name, desc, body, src="storage"):
    f(addr, size, name, src, desc, '#include "hw/fpga.h"' + "\n\n" + body.strip("\n"))


COM = """
#define B8(a) (*(volatile unsigned char *)(a))
#define H16(a) (*(volatile unsigned short *)(a))
#define W32(a) (*(volatile u32 *)(a))
#define ST 0x42E68D1Bu               /* data stage state: 0 idle, 1 busy */
#define ERR 0x42E68D1Cu              /* error bits: 8 read, 4 write */
#define CSWST 0x42E68D1Du            /* command state: 3 send status, 4 host ended the data */
#define USB 0x42E68C10u              /* +8: data residue */
#define BUF 0x42E08074u              /* sector bounce buffer */

void usb_disk_hook(void);
"""

wr(0x4006A734, 4, "usb_disk_hook",
   "Empty hook (bx lr), tail-called by usb_disk_sd_read / usb_disk_sd_write when a transfer ends.",
   COM + """
void usb_disk_hook(void)
{
}
""")

wr(0x4006A738, 304, "usb_disk_sd_read",
   "READ(10) data stage from the SD card, one 512-byte packet per call. The first call (state byte 0x42E68D1B = 0) stores the block count in 0x42E68C84 and "
   "starts; the first packet of a transfer reads all count blocks from sector lba with mmc_read_blocks(0, lba, count, 0x42E08074) (a failure sets bit 3 of "
   "0x42E68D1C) and sends the start of the buffer, later ones send the next 512 bytes (offset word 0x42E68C80) with usb_mass_write(1, ..., 512). Each "
   "packet takes 512 from the residue word 0x42E68C10 + 8 and one from the remaining count. When none remain the state resets and 0x42E68D1D = 3 (send "
   "the status). Returns the remaining count.",
   COM + """
#define DONE 0x42E68C7Cu             /* packets sent */
#define LEFT 0x42E68C84u             /* blocks left */
#define OFF 0x42E68C80u              /* buffer offset of the next packet */

void usb_mass_write(u32 ep, u32 buf, u32 n);

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
            usb_mass_write(1, BUF, 0x200);
            left = W32(LEFT) - 1;
            W32(DONE) = W32(DONE) + 1;
            W32(OFF) = 0x200;
        } else {
            usb_mass_write(1, BUF + W32(OFF), 0x200);
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
""")

wr(0x4006A868, 252, "usb_disk_sd_write",
   "WRITE(10) data stage to the SD card, one 512-byte packet per call. The first call (state byte 0x42E68D1B = 0) stores lba (0x42E68C88) and the "
   "block count (0x42E68C8C). Each call copies the received packet (pointer word 0x42E68CE0) to 0x42E08074 + 512 x n (n = word 0x42E68C90, then n + 1) "
   "and takes half 0x42E68D16 from the residue word 0x42E68C10 + 8. Once all blocks are in, or when 0x42E68D1D is 4, mmc_write_blocks(0, lba, count, "
   "0x42E08074) writes them (a failure sets bit 2 of 0x42E68D1C), n = 0, usb_mass_send_csw(0, 1) and the state resets; the result is then the one of "
   "usb_mass_send_csw, otherwise n.",
   COM + """
#define LBA 0x42E68C88u
#define TOTAL 0x42E68C8Cu
#define N 0x42E68C90u

u32 usb_mass_send_csw(u32 a, u32 b);

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
    if ((unsigned char)mmc_write_blocks(0, W32(LBA), W32(TOTAL), (const void *)BUF))
        B8(ERR) |= 4;
    W32(N) = 0;
    r = usb_mass_send_csw(0, 1);
    B8(ST) = 0;
    usb_disk_hook();
    return r;
}
""")
