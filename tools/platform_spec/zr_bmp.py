# Writing a screenshot .bmp (with the preview stored in the gap after the header), the linear
# resampling of a trace (zoom), the preview of a list C record and a plain number-to-text helper.
# exec'd by gen_platform.py (do not define HDR, SPEC, OUT or f here)


def wr(addr, size, name, desc, body, src="storage"):
    f(addr, size, name, src, desc, '#include "hw/fpga.h"' + "\n\n" + body.strip("\n"))


wr(0x4005C3F0, 508, "bmp_save",
   "Writes the screenshot file `path`: a 1,300,000-byte (0x13D620) block at 0x41D3E3A8 holding a BMP header ('BM', pixel data at 0xDEA8, 1022 x 600, 16 bits, BI_BITFIELDS with the RGB565 masks 0xF800 / 0x07E0 / 0x001F, image size field 0xBB800), "
   "then at +0x46 the preview from SYS+0x109684 (u16 width and height, little endian, and the pixels: 2 * w * h + 10 bytes copied from SYS+0x109688), then from +0xDEA8 the 600 rows of the framebuffer copy at 0x42210D20, bottom row first (rows of 0x7FC bytes). "
   "draw_target_default is called before the copy and fb_target_back at the end. The file is opened with mode 0xB (create always, read/write), written in one f_write with a null byte-count pointer, and closed. Returns 1 when f_write succeeds, 0 otherwise.", """
void *memcpy(void *d, const void *s, u32 n);
void draw_target_default(void);
void fb_target_back(void);
int f_open(void *fp, const char *path, unsigned char mode);
int f_lseek(void *fp, unsigned long long ofs);
int f_write(void *fp, const void *buf, u32 btw, u32 *bw);
int f_close(void *fp);
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
""")

wr(0x4005C678, 324, "simple_integer_index_mapping_nearest_neighbour",
   "Stretches a trace by linear interpolation (the name comes from an earlier project; the code interpolates). k = W32(SYS+0x554), m = the target scale; nothing happens when k >= m. "
   "n = (int)(w * 1.2 * k / m) source samples centred on src[center] are stretched to L = m * n / k samples in the buffer 0x416431E8 (8-bit fixed-point steps), then copy_u16 copies w of them, centred and moved back by `shift`, to src[center - w / 2]. "
   "The stock code divides by k with udiv, so k = 0 gives 0 there; that case is not tested.", """
void copy_u16(unsigned short *d, const unsigned short *s, u32 n);
#define SYS 0x40544E68u
#define W32(a) (*(volatile u32 *)(a))

void simple_integer_index_mapping_nearest_neighbour(u32 src_a, u32 center_a, u32 w, u32 m, u32 shift_a, u32 unused)
{
    volatile unsigned short *src = (volatile unsigned short *)src_a;
    int center = (int)center_a, shift = (int)shift_a;
    u32 k = W32(SYS + 0x554), t, acc, sl;
    volatile unsigned short *out = (volatile unsigned short *)0x416431E8u, *p, *o;
    unsigned short *q;
    int n, L, i, pos, prev, last, cur, diff;
    if (k >= m)
        return;
    n = (int)((double)w * 1.2 * (double)k / (double)m);
    p = src + (center - n / 2);
    q = (unsigned short *)(src + (center - ((int)w >> 1)));
    L = (int)((m * (u32)n) / k);
    last = *p++;
    out[0] = (unsigned short)last;
    i = 1;
    prev = 0;
    pos = (int)m;
    if (L >= 2) {
        do {
            t = k * (u32)i;
            cur = *p;
            acc = (t - (u32)prev) << 8;
            diff = cur - last;
            o = out + i;
            while (pos >= (int)t) {
                sl = acc / m;
                t += k;
                i++;
                *o++ = (unsigned short)(last + ((int)(sl * (u32)diff) >> 8));
                acc += k << 8;
            }
            last = cur;
            prev = pos;
            p++;
            pos += (int)m;
        } while (i < L);
    }
    copy_u16(q, (const unsigned short *)((u32)(out + (L - (int)(unsigned short)w) / 2) - 2u * (u32)shift),
             (unsigned short)w);
}
""", src="trace")

wr(0x4003754C, 460, "list_c_record_preview",
   "Preview of the selected list C record: list_c_record_select(a), then with len = W32(SYS+0x3DA53C) cells at SYS+0x3DA548 and f = len / 193 + 1: m = W32(SYS+0x554) * f * 193 / len, array_3da548_decimate(f), len /= f; "
   "the cells are copied to the middle of the buffer 0x416431E8 (around element 10000), stretched there to 193 samples (simple_integer_index_mapping_nearest_neighbour), and len = len * m / W32(SYS+0x554) cells are copied back. "
   "The minimum (over cells 1 .. len-2) is subtracted from every cell, and the cells are scaled to 8 + 122 * v / (max - min) into the preview block (u16 count at SYS+0x109684, values from SYS+0x109688). A length of 0 is not tested (the stock code divides by it and then loops 2^32 - 2 times).", """
void array_3da548_decimate(u32 f);
void copy_u16(unsigned short *d, const unsigned short *s, u32 n);
unsigned long long __aeabi_uldivmod(unsigned long long a, unsigned long long b);
#define SYS 0x40544E68u
#define H16(a) (*(volatile unsigned short *)(a))
#define W32(a) (*(volatile u32 *)(a))
#define LEN W32(SYS + 0x3DA53Cu)
#define CELLS (SYS + 0x3DA548u)
#define ZBUF 0x416431E8u
#define ZMID (ZBUF + 0x4E20u)

void list_c_record_preview(u32 a)
{
    u32 len, f, m, n, i, mx, mn;
    double sc;
    list_c_record_select(a);
    len = LEN;
    f = (u32)(((unsigned long long)len * 0xA9C84A48u) >> 32 >> 7) + 1;
    m = (u32)__aeabi_uldivmod((unsigned long long)W32(SYS + 0x554) * f * 193u,
                              (unsigned long long)(long long)(int)len);
    array_3da548_decimate((unsigned char)f);
    n = LEN / f;
    LEN = n;
    copy_u16((unsigned short *)(ZMID - 2 * ((int)n / 2)), (const unsigned short *)CELLS, n);
    simple_integer_index_mapping_nearest_neighbour(ZBUF, 10000, 193, m, 0, 0);
    n = (u32)__aeabi_uldivmod((unsigned long long)(long long)(int)LEN * m, (unsigned long long)W32(SYS + 0x554));
    LEN = n;
    copy_u16((unsigned short *)CELLS, (const unsigned short *)(ZMID - 2 * ((int)n / 2)), n);
    n = LEN;
    mx = 0;
    mn = 0xFFFFFFFFu;
    if (n - 1 >= 2)
        for (i = 0; i < n - 2; i++) {
            u32 v = H16(CELLS + 2 + 2 * i);
            if (v >= mx)
                mx = v;
            if (v <= mn)
                mn = v;
        }
    if (n != 0) {
        i = 0;
        do {
            H16(CELLS + 2 * i) = (unsigned short)(H16(CELLS + 2 * i) - mn);
            i++;
        } while (i < LEN);
    }
    sc = 122.0 / (double)(mx - mn);
    for (i = 0; i < LEN; i++)
        H16(SYS + 0x109688 + 2 * i) = (unsigned short)(int)(8.0 + (double)H16(CELLS + 2 * i) * sc);
    H16(SYS + 0x109684) = (unsigned short)LEN;
}
""", src="ui")

wr(0x40037160, 640, "number_text_88c0",
   "Writes n in decimal (no leading zeros) and a terminating 0 to SYS+0x88C0. n < 10 is one character even when negative (the stock code adds '0'); from 65536 on nothing is written.", """
#define SYS 0x40544E68u
#define B8(a) (*(volatile unsigned char *)(a))

void number_text_88c0(int n)
{
    int nd, i;
    if (n < 10) {
        B8(SYS + 0x88C0) = (unsigned char)(n + '0');
        B8(SYS + 0x88C1) = 0;
        return;
    }
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
    B8(SYS + 0x88C0 + nd) = 0;
    for (i = nd - 1; i >= 0; i--) {
        B8(SYS + 0x88C0 + i) = (unsigned char)('0' + n % 10);
        n /= 10;
    }
}
""", src="string")
