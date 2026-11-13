# Display: TCON0 (LCD timing controller) and DE2 (display engine 2.0) bring-up.  exec'd by gen_platform.py
DH = """
typedef struct { u32 de_base, tcon_base, width, height, bpp, fb; } disp_t;   /* 0x42E68B68 */
#define g_disp_ptr  (*(disp_t **)0x42E68CB4u)
#define g_disp      ((disp_t *)0x42E68B68u)
#define lcd_timing  ((const u32 *)0x400952B0u)    /* panel timing table (.data), indexed in words */
u32 lcd_readl(u32 addr);
void lcd_writel(u32 addr, u32 v);
u32 de_readl(u32 addr);
void de_writel(u32 addr, u32 v);
void ccu_gate_on(s32 code);
void ccu_gate_off(s32 code);
void delay_ms(u32 ms);
void *memset_w(void *d, int v, unsigned int n);
unsigned long long __aeabi_llsl(unsigned long long v, int sh);
"""


def d(addr, size, name, desc, body):
    f(addr, size, name, "display", desc, DH.strip("\n") + "\n\n" + body.strip("\n"))


d(0x4003DFFC, 8, "lcd_readl", "Register read (copy in the LCD source file).", "u32 lcd_readl(u32 addr) { return REG32(addr); }")
d(0x4003E004, 8, "lcd_writel", "Register write.", "void lcd_writel(u32 addr, u32 v) { REG32(addr) = v; }")
d(0x4003E00C, 80, "tcon_clock_off", "TCON bus gate (0x01C20064 bit 4), TCON clock (0x01C20118 bit 31) and TCON reset (0x01C202C4 bit 4) off.", """
void tcon_clock_off(void)
{
    lcd_writel(0x01C20064u, lcd_readl(0x01C20064u) & ~0x10u);
    lcd_writel(0x01C20118u, lcd_readl(0x01C20118u) & ~0x80000000u);
    lcd_writel(0x01C202C4u, lcd_readl(0x01C202C4u) & ~0x10u);
}
""")
d(0x4003E05C, 8, "de_clock_off", "DE bus gate and reset off.", "void de_clock_off(void) { ccu_gate_off(0x0C8C588C); }")
d(0x4003E064, 636, "tcon_init",
  "PLL_VIDEO from the panel table (N, M), TCON clocks on, TCON0 control/clock divider, BASIC0..3 timing (active size, horizontal/vertical total and back porch, sync widths), FRM dither tables, FRM mode (18 bpp: off), IO polarity from the table flags.", """
void tcon_init(disp_t *dp)
{
    const u32 *t = lcd_timing;
    u32 tc = dp->tcon_base, v;
    lcd_writel(0x01C20010u, lcd_readl(0x01C20010u) & ~0x80000000u);
    lcd_writel(0x01C20010u, ((t[0x40 / 4] - 1) | ((t[0x3C / 4] - 1) << 8)) | 0x3000000);
    lcd_writel(0x01C20010u, lcd_readl(0x01C20010u) | 0x80000000u);
    ccu_gate_on(0x0C845884);
    lcd_writel(0x01C20118u, 0x80000000u);
    delay_ms(1);
    lcd_writel(0x01C0C040u, lcd_readl(0x01C0C040u) & ~1u);
    lcd_writel(0x01C0C040u, (((t[0x18 / 4] + t[0x1C / 4] + t[0x20 / 4]) << 4) & 0x1F0) | 0x80000000u);
    lcd_writel(0x01C0C044u, t[0x44 / 4] | 0xF0000000u);
    lcd_writel(tc + 0x48, (t[0x14 / 4] - 1) | ((t[0x04 / 4] - 1) << 16));
    v = t[0x0C / 4] + t[0x10 / 4];
    lcd_writel(tc + 0x4C, (v - 1) | ((v + t[0x34 / 4] * t[0x04 / 4] + t[0x08 / 4] - 1) << 16));
    v = t[0x1C / 4] + t[0x20 / 4];
    lcd_writel(tc + 0x50, (v - 1) | ((v + t[0x18 / 4] + t[0x14 / 4]) << 17));
    lcd_writel(tc + 0x54, (t[0x20 / 4] - 1) | ((t[0x10 / 4] - 1) << 16));
    lcd_writel(0x01C0C040u, lcd_readl(0x01C0C040u) | 0x800000);
    lcd_writel(tc + 0x58, 0);
    lcd_writel(tc + 0x60, 0);
    lcd_writel(tc + 0x14, 0x11111111u);
    lcd_writel(tc + 0x18, 0x11111111u);
    lcd_writel(tc + 0x1C, 0x11111111u);
    lcd_writel(tc + 0x20, 0x11111111u);
    lcd_writel(tc + 0x24, 0x11111111u);
    lcd_writel(tc + 0x28, 0x11111111u);
    lcd_writel(tc + 0x2C, 0x01010000u);
    lcd_writel(tc + 0x30, 0x15151111u);
    lcd_writel(tc + 0x34, 0x57575555u);
    lcd_writel(tc + 0x38, 0x7F7F7777u);
    lcd_writel(tc + 0x10, dp->bpp == 0x12 ? 0x80000000u : 0x80000050u);
    v = t[0x24 / 4] == 0 ? 0x12000000u : 0x10000000u;
    if (t[0x28 / 4] == 0) v |= 0x1000000;
    if (t[0x2C / 4] == 0) v |= 0x08000000;
    if (t[0x30 / 4] == 0) v |= 0x04000000;
    lcd_writel(tc + 0x88, v);
    lcd_writel(tc + 0x8C, 0);
}
""")
d(0x4003E2E0, 32, "tcon_enable", "TCON global enable (bit 31).", """
void tcon_enable(disp_t *dp)
{
    u32 r = dp->tcon_base;
    lcd_writel(r, lcd_readl(r) | 0x80000000u);
}
""")
d(0x4003E300, 4, "de_commit", "Loads de_base and falls into de_commit_base.", """
void de_commit_base(u32 de);

void de_commit(disp_t *dp) { de_commit_base(dp->de_base); }
""")
d(0x4003E304, 16, "de_commit_base", "Mixer 0 GLB_DBUFFER (+0x100008) = 1: apply the shadow registers.",
  "void de_commit_base(u32 de) { lcd_writel(de + 0x100008, 1); }")
d(0x4003E314, 708, "de_init",
  "DE clocks, mixer 0 on with the panel size, channel/blender register blocks cleared, blender routing 0x03010301, four pipes with opaque black fill, post-processing blocks (CSC, peaking, ...) off, then the UI layer setup.", """
void de_setup_ui(void);

void de_init(disp_t *dp)
{
    u32 de = dp->de_base, bld = de + 0x101000, glb = de + 0x100000;
    u32 size = (dp->width - 1) | ((dp->height - 1) << 16);
    int i;
    ccu_gate_on(0x0C8C588C);
    lcd_writel(0x01C20104u, lcd_readl(0x01C20104u) | 0x80000000u);
    lcd_writel(de + 8, lcd_readl(de + 8) | 1);
    lcd_writel(de, lcd_readl(de) | 1);
    lcd_writel(de + 4, lcd_readl(de + 4) | 1);
    lcd_writel(de + 0x10, lcd_readl(de + 0x10) & ~1u);
    lcd_writel(glb, 1);
    lcd_writel(glb + 4, 0);
    lcd_writel(glb + 8, 0);
    lcd_writel(glb + 0xC, size);
    memset_w((void *)(dp->de_base + 0x102000), 0, 0x100);
    for (i = 1; i <= 3; i++)
        memset_w((void *)(dp->de_base + 0x1000 * i + 0x102000), 0, 0x8C);
    memset_w((void *)bld, 0, 0x100);
    lcd_writel(bld, 0);
    lcd_writel(bld + 0x80, 0);
    lcd_writel(bld + 0x84, 0);
    lcd_writel(bld + 0x88, 0);
    lcd_writel(bld + 0x90, 0x03010301u);
    lcd_writel(bld + 0x94, 0x03010301u);
    lcd_writel(bld + 0x8C, size);
    lcd_writel(bld + 0xFC, 0);
    lcd_writel(bld + 0xB0, 0);
    for (i = 0; i < 4; i++) {
        lcd_writel(bld + 4 + 0x10 * i, 0xFF000000u);
        lcd_writel(bld + 8 + 0x10 * i, size);
    }
    lcd_writel(dp->de_base + 0x120000, 0);
    lcd_writel(dp->de_base + 0x130000, 0);
    lcd_writel(dp->de_base + 0x140000, 0);
    lcd_writel(dp->de_base + 0x150000, 0);
    lcd_writel(dp->de_base + 0x1A0000, 0);
    lcd_writel(dp->de_base + 0x1A2000, 0);
    lcd_writel(dp->de_base + 0x1A4000, 0);
    lcd_writel(dp->de_base + 0x1A6000, 0);
    lcd_writel(dp->de_base + 0x1A8000, 0);
    lcd_writel(dp->de_base + 0x1AA000, 0);
    lcd_writel(dp->de_base + 0x1B0000, 0);
    de_setup_ui();
}
""")
d(0x4003E5D8, 100, "display_init", "Fill the display descriptor (DE2 0x01000000, TCON 0x01C0C000, width, height, 16 bpp, framebuffer), TCON init, DE init, TCON enable, 1 ms, commit.", """
void tcon_init(disp_t *dp);
void de_init(disp_t *dp);
void tcon_enable(disp_t *dp);
void de_commit(disp_t *dp);

void display_init(u32 width, u32 height, u32 fb)
{
    disp_t *dp = g_disp;
    g_disp_ptr = dp;
    dp->width = width;
    dp->bpp = 0x10;
    dp->de_base = 0x01000000u;
    dp->tcon_base = 0x01C0C000u;
    dp->height = height;
    dp->fb = fb;
    tcon_init(dp);
    de_init(g_disp_ptr);
    tcon_enable(g_disp_ptr);
    delay_ms(1);
    de_commit(g_disp_ptr);
}
""")
d(0x40062F9C, 8, "de_readl", "Register read (copy in the DE source file).", "u32 de_readl(u32 addr) { return REG32(addr); }")
d(0x40062FA4, 8, "de_writel", "Register write.", "void de_writel(u32 addr, u32 v) { REG32(addr) = v; }")
d(0x40062FAC, 112, "de_pipe_enable", "Blender BLD_FILL_COLOR_CTL (+0x101000): pipe bit (8 + pipe) on or off; returns 0.", """
int de_pipe_enable(int pipe, int on)
{
    disp_t *dp = g_disp_ptr;
    u32 bit = (u32)__aeabi_llsl(1, pipe + 8), r = dp->de_base + 0x101000;
    if (on) de_writel(r, bit | de_readl(r));
    else de_writel(r, de_readl(r) & ~bit);
    return 0;
}
""")
d(0x4006301C, 84, "de_route", "Blender BLD_CH_RTCTL (+0x101080): channel of a pipe (4 bits per pipe); returns 0.", """
int de_route(int pipe, u32 ch)
{
    u32 r = g_disp_ptr->de_base + 0x101000 + 0x80, sh = (u32)pipe << 2;
    de_writel(r, (de_readl(r) & ~(0xFu << sh)) | (ch << sh));
    return 0;
}
""")
d(0x40063070, 152, "de_ui_set_layer", "UI channel ch (+0x102000 + ch * 0x1000), layer l (32 bytes each): attribute = enable | format << 8, size = (h-1)<<16 | (w-1), coordinate, pitch, top address; returns 0.", """
int de_ui_set_layer(int ch, int layer, u32 x, u32 y, u32 w, u32 h, u32 addr, u32 pitch, signed char fmt)
{
    u32 r = g_disp_ptr->de_base + ((u32)ch << 12) + 0x102000 + ((u32)layer << 5);
    de_writel(r, 1 | ((u32)(int)fmt << 8));
    de_writel(r + 4, 0xFFFEFFFFu + (w + (h << 16)));
    de_writel(r + 8, x + (y << 16));
    de_writel(r + 0xC, pitch);
    de_writel(r + 0x10, addr);
    return 0;
}
""")
d(0x40063108, 68, "de_ui_set_size", "UI channel overlay size (+0x88) = (h-1)<<16 | (w-1); returns 0.", """
int de_ui_set_size(int ch, u32 w, u32 h)
{
    u32 r = g_disp_ptr->de_base + ((u32)ch << 12) + 0x102000 + 0x88;
    de_writel(r, 0xFFFEFFFFu + (w + (h << 16)));
    return 0;
}
""")
d(0x4006314C, 132, "de_setup_ui", "Route pipe 0 to UI channel 2, enable pipe 0, layer 0 of channel 2 = the framebuffer (RGB565, format 0x0A, pitch 2 * width), overlay size; returns 0.", """
int de_route(int pipe, u32 ch);
int de_pipe_enable(int pipe, int on);
int de_ui_set_layer(int ch, int layer, u32 x, u32 y, u32 w, u32 h, u32 addr, u32 pitch, signed char fmt);
int de_ui_set_size(int ch, u32 w, u32 h);

int de_setup_ui(void)
{
    disp_t *dp;
    de_route(0, 2);
    de_pipe_enable(0, 1);
    dp = g_disp_ptr;
    de_ui_set_layer(2, 0, 0, 0, dp->width, dp->height, dp->fb, dp->width << 1, 0x0A);
    dp = g_disp_ptr;
    de_ui_set_size(2, dp->width, dp->height);
    return 0;
}
""")

# ---------------------------------------------------------------- framebuffer helpers
FBH = """
#define FB_PIXELS 0x95B50u                     /* 1022 * 600 */
#define fb_front  ((unsigned short *)0x41FB8FE8u)
#define fb_back   ((unsigned short *)0x41BFBF68u)
void copy_u16(unsigned short *d, const unsigned short *s, u32 n);
void fill_u16(unsigned short *d, unsigned short v, u32 n);
void fill_u32(u32 *d, u32 v, u32 n);
void dcache_clean_inv_all(void);
void dcache_clean_inv_all_t(void);
"""


def fb(addr, size, name, desc, body):
    f(addr, size, name, "display", desc, FBH.strip("\n") + "\n\n" + body.strip("\n"))


fb(0x4003442C, 40, "fb_present", "Copy the back buffer (0x41BFBF68) to the scanned-out framebuffer (0x41FB8FE8), then flush the D-cache.", """
void fb_present(void)
{
    copy_u16(fb_front, fb_back, FB_PIXELS);
    dcache_clean_inv_all_t();
}
""")
fb(0x40034454, 24, "fb_clear_current", "Clear the buffer whose address is stored at 0x40095498.",
   "void fb_clear_current(void) { fill_u16(*(unsigned short **)0x40095498u, 0, FB_PIXELS); }")
fb(0x4003446C, 116, "fb_clear_all", "Clear the four 16-bit screen buffers and the two 32-bit buffers (0x40CFF978, 0x40F576B0), then present.", """
void fb_present(void);

void fb_clear_all(void)
{
    fill_u16(fb_front, 0, FB_PIXELS);
    fill_u16(fb_back, 0, FB_PIXELS);
    fill_u16((unsigned short *)0x420E4E84u, 0, FB_PIXELS);
    fill_u16((unsigned short *)0x4233CBBCu, 0, FB_PIXELS);
    fill_u32((u32 *)0x40CFF978u, 0, FB_PIXELS);
    fill_u32((u32 *)0x40F576B0u, 0, FB_PIXELS);
    fb_present();
}
""")
fb(0x400356E4, 16, "memset_w", "memset(dst, value, n): b __aeabi_memset (0x4005C604) with the length and value registers swapped.", """
void *__aeabi_memset_entry(void *d, unsigned int n, int v);

void *memset_w(void *d, int v, unsigned int n) { return __aeabi_memset_entry(d, n, v); }
""")
fb(0x400356F4, 24, "fill_u16", "Store a halfword n times.", """
void fill_u16(unsigned short *d, unsigned short v, u32 n)
{
    while (n) { *d++ = v; n--; }
}
""")
fb(0x4003570C, 24, "fill_u32", "Store a word n times.", """
void fill_u32(u32 *d, u32 v, u32 n)
{
    while (n) { *d++ = v; n--; }
}
""")
fb(0x40035724, 4, "memcpy", "b __aeabi_memcpy.", """
void *__aeabi_memcpy(void *d, const void *s, unsigned int n);

void *memcpy(void *d, const void *s, unsigned int n) { return __aeabi_memcpy(d, s, n); }
""")
fb(0x40035728, 28, "copy_u16", "Copy n halfwords.", """
void copy_u16(unsigned short *d, const unsigned short *s, u32 n)
{
    while (n) { *d++ = *s++; n--; }
}
""")
fb(0x4003E6F4, 4, "dcache_clean_inv_all_t", "b dcache_clean_inv_all.", "void dcache_clean_inv_all_t(void) { dcache_clean_inv_all(); }")
