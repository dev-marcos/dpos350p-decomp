# Saved files: the names /Screenshots/<n>.bmp and /Waveform/<n>.wav, the small RGB565 previews
# (thumbnails) of the screen and of the trace area, the trace rebuilt from a saved waveform
# buffer, and the record list at SYS+0xD744 (append a record, clear a record by its id).
# exec'd by gen_platform.py (do not define HDR, SPEC, OUT or f here)


def wq(addr, size, name, desc, body, src="storage"):
    f(addr, size, name, src, desc, '#include "hw/fpga.h"' + "\n\n" + body.strip("\n"))


PATH = """void __aeabi_memcpy4(void *d, const void *s, unsigned n);
#define SYS 0x40544E68u
#define B8(a) (*(volatile unsigned char *)(a))

/* p points at the '/' before the number; n < 10 is written as one character, also when it is
   negative; 10..65535 as 2..5 decimal digits; from 65536 on only the '/' is there */
static void path_number(u32 p, int n, const char *ext)
{
    int nd, i;
    if (n < 10) {
        B8(p + 1) = (unsigned char)(n + '0');
        nd = 1;
    } else {
        if (n < 100)
            nd = 2;
        else if (n < 1000)
            nd = 3;
        else if (n < 10000)
            nd = 4;
        else if (n < 65536)
            nd = 5;
        else
            return;
        for (i = nd; i >= 1; i--) {
            B8(p + i) = (unsigned char)('0' + n % 10);
            n /= 10;
        }
    }
    for (i = 0; i < 4; i++)
        B8(p + 1 + nd + i) = (unsigned char)ext[i];
    B8(p + 5 + nd) = 0;
}
"""

wq(0x40034B78, 796, "screenshot_path_make",
   "Writes the file name '/Screenshots/<n>.bmp' to SYS+0x88C0: the 13 bytes at 0x4008F82C ('/Screenshots' and its 0), the 0 replaced by '/', then n in decimal (no leading zeros) and '.bmp'. "
   "n < 10 is one character even when negative (the stock code adds '0'); for n >= 65536 only the '/' is written and the name is not terminated.", PATH + """
void screenshot_path_make(int n)
{
    __aeabi_memcpy4((void *)(SYS + 0x88C0), (const void *)0x4008F82Cu, 13);
    B8(SYS + 0x88CC) = '/';
    path_number(SYS + 0x88CC, n, ".bmp");
}
""")

wq(0x40035BC4, 796, "waveform_path_make",
   "Same as screenshot_path_make for '/Waveform/<n>.wav': the 10 bytes at 0x4008FFB4 ('/Waveform' and its 0) to SYS+0x88C0, '/' at SYS+0x88C9, then the number and '.wav'.", PATH + """
void waveform_path_make(int n)
{
    __aeabi_memcpy4((void *)(SYS + 0x88C0), (const void *)0x4008FFB4u, 10);
    B8(SYS + 0x88C9) = '/';
    path_number(SYS + 0x88C9, n, ".wav");
}
""")

THUMB = """void copy_u16(unsigned short *d, const unsigned short *s, u32 n);
#define SYS 0x40544E68u
#define H16(a) (*(volatile unsigned short *)(a))
#define W32(a) (*(volatile u32 *)(a))
#define TW (SYS + 0x109684u)     /* u16 width, u16 height, then the RGB565 pixels */

/* (1e6 * sum) / a / b with the stock reciprocal multiplications */
static u32 scale(u32 sum, u32 m1, u32 m2, int s2)
{
    u32 t = (u32)(((unsigned long long)(sum * 1000000u) * m1) >> 32) >> 12;
    return (u32)(((unsigned long long)t * m2) >> 32) >> s2;
}

static unsigned short pack(u32 r, u32 g, u32 b)
{
    if (r > 0x1F)
        r = 0x1F;
    if (g > 0x3F)
        g = 0x3F;
    if (b > 0x1F)
        b = 0x1F;
    return (unsigned short)(r << 11 | g << 5 | b);
}
"""

wq(0x40034E94, 472, "screenshot_thumb_make",
   "Builds the preview of a screenshot: copies the scanned-out framebuffer 0x41FB8FE8 (1022 x 600 RGB565) to 0x42210D20, then box-filters it in steps of 5.035 x 4.348 pixels (6 x 5 samples per output pixel, coordinates in 1/1000 pixel) into a 203 x 138 picture. "
   "Header at SYS+0x109684: u16 width (counted per row) and u16 height; the pixels follow at SYS+0x109688. Each channel is the sum of its samples x 1e6 / 5035 / 4348, clamped to 31 / 63 / 31.", THUMB + """
void screenshot_thumb_make(void)
{
    volatile unsigned short *fb = (volatile unsigned short *)0x42210D20u;
    volatile unsigned short *dst = (volatile unsigned short *)(SYS + 0x109688u);
    int x, y = 0, i, j, k = 0;
    copy_u16((unsigned short *)0x42210D20u, (const unsigned short *)0x41FB8FE8u, 0x95B50);
    W32(TW) = 0;
    do {
        H16(TW) = 0;
        H16(TW + 2) = (unsigned short)(H16(TW + 2) + 1);
        x = 0;
        do {
            u32 r = 0, g = 0, b = 0;
            for (i = 0; i < 5; i++) {
                u32 row = 1022u * (u32)((y + 1000 * i) / 1000);
                for (j = 0; j < 6; j++) {
                    u32 p = fb[(u32)((x + 1000 * j) / 1000) + row];
                    r += p >> 11 & 0x1F;
                    g += p >> 5 & 0x3F;
                    b += p & 0x1F;
                }
            }
            dst[k++] = pack(scale(r, 0xD041E4DAu, 0xF129AF75u, 12), scale(g, 0xD041E4DAu, 0xF129AF75u, 12),
                            scale(b, 0xD041E4DAu, 0xF129AF75u, 12));
            H16(TW) = (unsigned short)(H16(TW) + 1);
            x += 5035;
        } while (x < 1022001);
        y += 4348;
    } while (y < 600001);
}
""")

wq(0x40035EE0, 528, "waveform_thumb_make",
   "Builds the preview of a saved waveform from the trace area (x u16 SYS+4, y u16 SYS+6, width u16 SYS+8, height u16 SYS+0xA) of the scanned-out framebuffer 0x41FB8FE8, in steps of 4.685 x 3.645 pixels (5 x 4 samples per output pixel), both ends included. "
   "Same layout as screenshot_thumb_make (u16 width and height at SYS+0x109684, pixels from SYS+0x109688); each channel is the sum x 1e6 / 4685 / 3645, clamped. Nothing but the header word is written when the area has a negative height.", THUMB + """
void waveform_thumb_make(void)
{
    volatile unsigned short *fb = (volatile unsigned short *)0x41FB8FE8u;
    volatile unsigned short *dst = (volatile unsigned short *)(SYS + 0x109688u);
    int x0 = (int)(1000u * H16(SYS + 4)), x1 = x0 + (int)(1000u * H16(SYS + 8));
    int y0 = (int)(1000u * H16(SYS + 6)), y1 = y0 + (int)(1000u * H16(SYS + 0xA));
    int x, y, i, j, k = 0;
    W32(TW) = 0;
    if (y1 < y0)
        return;
    y = y0;
    do {
        H16(TW) = 0;
        H16(TW + 2) = (unsigned short)(H16(TW + 2) + 1);
        if (x1 >= x0) {
            x = x0;
            do {
                u32 r = 0, g = 0, b = 0;
                for (i = 0; i < 4; i++) {
                    u32 row = 1022u * (u32)((y + 1000 * i) / 1000);
                    for (j = 0; j < 5; j++) {
                        u32 p = fb[(u32)((x + 1000 * j) / 1000) + row];
                        r += p >> 11 & 0x1F;
                        g += p >> 5 & 0x3F;
                        b += p & 0x1F;
                    }
                }
                dst[k++] = pack(scale(r, 0xDFD0C9F6u, 0x8FD66C05u, 11), scale(g, 0xDFD0C9F6u, 0x8FD66C05u, 11),
                                scale(b, 0xDFD0C9F6u, 0x8FD66C05u, 11));
                H16(TW) = (unsigned short)(H16(TW) + 1);
                x += 4685;
            } while (x1 >= x);
        }
        y += 3645;
    } while (y1 >= y);
}
""")

wq(0x40036864, 244, "saved_wave_trace_build",
   "Rebuilds the trace of a saved waveform. State at SYS+0x3DA530: byte +0 buffer index (buffers of 0x927C0 bytes from 0x411AF3E8), word +4 first column, word +8 end column, word +0xC set to end - first. "
   "For each column x in [first, end): the mean of n = W32(SYS+0x5F8) u16 samples starting at W32(SYS+0x38C) + n * (x - u16 SYS+8 / 2), divided by W32(SYS+0x1C0); u16 SYS+0xA minus that goes to u16 SYS+0x3DA548 + 2 * (x - first).", """
long long __aeabi_ldivmod(long long a, long long b);
#define SYS 0x40544E68u
#define H16(a) (*(volatile unsigned short *)(a))
#define W32(a) (*(volatile u32 *)(a))
#define B8(a) (*(volatile unsigned char *)(a))
#define ST (SYS + 0x3DA530u)

void saved_wave_trace_build(void)
{
    u32 base = 0x411AF3E8u + 0x927C0u * B8(ST);
    int x = (int)W32(ST + 4);
    W32(ST + 0xC) = W32(ST + 8) - W32(ST + 4);
    for (; x < (int)W32(ST + 8); x++) {
        u32 n = W32(SYS + 0x5F8), i;
        volatile unsigned short *p = (volatile unsigned short *)(base + 2 * (W32(SYS + 0x38C) + n * ((u32)x - (H16(SYS + 8) >> 1))));
        unsigned long long sum = 0;
        long long v;
        for (i = 0; i < n; i++)
            sum += p[i];
        v = __aeabi_ldivmod((long long)sum, (long long)n);
        v = __aeabi_ldivmod(v, (long long)W32(SYS + 0x1C0));
        H16(SYS + 0x3DA548u + 2 * ((u32)x - W32(ST + 4))) = (unsigned short)(H16(SYS + 0xA) - (u32)v);
    }
}
""")

REC = """void copy_u16(unsigned short *d, const unsigned short *s, u32 n);
#define SYS 0x40544E68u
#define H16(a) (*(volatile unsigned short *)(a))
#define W32(a) (*(volatile u32 *)(a))
#define B8(a) (*(volatile unsigned char *)(a))
#define RECS (SYS + 0xD744u)                /* u16 cells, one byte per cell */
#define RSIZE W32(SYS + 0x3DA544u)          /* record size in cells */
#define RCOUNT H16(SYS + 0x758)             /* number of records */

/* record id: low bytes of cells 0 and 1 */
static u32 rec_id(u32 p)
{
    return (H16(p) << 8 & 0xFF00) | B8(p + 2);
}
"""

wq(0x40036DE4, 192, "record_append",
   "Stores the record being built into the first free slot of the record list at SYS+0xD744 (u16 SYS+0x758 slots of W32(SYS+0x3DA544) cells; a slot is free when its id, the low bytes of cells 0 and 1, is 0). "
   "Cells 0..1 get the id u16 SYS+0xF9C (high byte, low byte), cells 2..3 the length word SYS+0x3DA53C (bits 8..15, bits 0..7), then copy_u16 copies that many cells from SYS+0x3DA548. "
   "With no free slot the stock code uses whatever the caller left in r2 as the cell index; here that is the third argument.", REC + """
void record_append(u32 a0, u32 a1, u32 r2_in)
{
    int i = 0, lim = (int)(RSIZE * RCOUNT);
    u32 p = RECS, idx = r2_in, d;
    (void)a0;
    (void)a1;
    for (; i < lim; i += (int)RSIZE, p += 2 * RSIZE)
        if (rec_id(p) == 0) {
            idx = (u32)i;
            break;
        }
    d = RECS + 2 * idx;
    H16(d) = (unsigned short)(H16(SYS + 0xF9C) >> 8 & 0xFF);
    H16(d + 2) = B8(SYS + 0xF9C);
    H16(d + 4) = (unsigned short)(W32(SYS + 0x3DA53Cu) >> 8 & 0xFF);
    H16(d + 6) = B8(SYS + 0x3DA53Cu);
    copy_u16((unsigned short *)(d + 8), (const unsigned short *)(SYS + 0x3DA548u), W32(SYS + 0x3DA53Cu));
}
""")

wq(0x40037020, 116, "record_clear",
   "Finds the record with id `id` in the record list at SYS+0xD744 and calls copy_u16(slot, 0, record size), which copies the record size in cells from address 0 over it (the stock code passes 0 as the source, probably meant as a fill). "
   "With no such record the stock code uses whatever the caller left in r3 as the cell index; here that is the fourth argument.", REC + """
void record_clear(u32 id, u32 a1, u32 a2, u32 r3_in)
{
    int i = 0, lim = (int)(RSIZE * RCOUNT);
    u32 p = RECS, idx = r3_in, n = RSIZE;
    (void)a1;
    (void)a2;
    for (; i < lim; i += (int)n, p += 2 * n)
        if (rec_id(p) == id) {
            idx = (u32)i;
            break;
        }
    copy_u16((unsigned short *)(RECS + 2 * idx), (const unsigned short *)0, n);
}
""")
