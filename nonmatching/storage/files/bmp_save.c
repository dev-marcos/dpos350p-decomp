/*
 * bmp_save -- stock 0x4005C3F0 (508 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Writes the screenshot file `path`: a 1,300,000-byte (0x13D620) block at 0x41D3E3A8 holding a BMP header ('BM', pixel data at 0xDEA8, 1022 x 600, 16 bits, BI_BITFIELDS with the RGB565 masks 0xF800 / 0x07E0 / 0x001F, image size field 0xBB800), then at +0x46 the preview from SYS+0x109684 (u16 width and height, little endian, and the pixels: 2 * w * h + 10 bytes copied from SYS+0x109688), then from +0xDEA8 the 600 rows of the framebuffer copy at 0x42210D20, bottom row first (rows of 0x7FC bytes). draw_target_default is called before the copy and fb_target_back at the end. The file is opened with mode 0xB (create always, read/write), written in one f_write with a null byte-count pointer, and closed. Returns 1 when f_write succeeds, 0 otherwise.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "fatfs_app.h"
#include "storage/files.h"
#include "ui/draw.h"
#include "util.h"

#define SYS 0x40544E68u
#define H16(a) (*(volatile unsigned short *)(a))
#define BMP ((volatile unsigned char *)0x41D3E3A8u)

static const unsigned char bmp_header[0x46] = {
    'B', 'M', 0x20, 0xD6, 0x13, 0, 0, 0, 0, 0, 0xA8, 0xDE, 0, 0,          /* file size, data offset */
    0x28, 0, 0, 0, 0xFE, 0x03, 0, 0, 0x58, 0x02, 0, 0, 1, 0, 0x10, 0,    /* 40, 1022, 600, 1, 16 */
    3, 0, 0, 0, 0, 0xB8, 0x0B, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,    /* 0x1E: BI_BITFIELDS, size */
    0, 0, 0, 0, 0, 0xF8, 0, 0, 0xE0, 0x07, 0, 0, 0x1F, 0, 0, 0, 0, 0, 0, 0,  /* 0x32: masks at 0x36 */
};

int bmp_save(const char *path)
{
    unsigned char fil[0x250];
    u32 i, w, h;
    int ok = 0;
    for (i = 0; i < sizeof bmp_header; i++)
        BMP[i] = bmp_header[i];
    w = H16(SYS + 0x109684);
    BMP[0x47] = (unsigned char)(w >> 8);
    BMP[0x46] = (unsigned char)w;
    h = H16(SYS + 0x109686);
    BMP[0x48] = (unsigned char)h;
    BMP[0x49] = (unsigned char)(h >> 8);
    memcpy((void *)(BMP + 0x4A), (const void *)(SYS + 0x109688), 2 * (h * w) + 10);
    draw_target_default();
    for (i = 0; i < 600; i++)
        memcpy((void *)(BMP + 0xDEA8 + 0x7FC * i), (const void *)(0x4233BBC4u - 0x7FC * i), 0x7FC);
    if (f_open(fil, path, 0xB) == 0) {
        f_lseek(fil, 0);
        if (f_write(fil, (const void *)BMP, 0x13D620, 0) == 0)
            ok = 1;
        f_close(fil);
    }
    fb_target_back();
    return ok;
}
