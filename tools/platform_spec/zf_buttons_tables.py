# Output and window buttons, the third level statistics, the second generator self-test, numpad keys,
# the 64-bit window recalculation and a 64-byte table at SYS+0x684.
# exec'd by gen_platform.py (do not define HDR, SPEC, OUT or f here)


def wf(addr, size, name, desc, body, src="ui"):
    f(addr, size, name, src, desc, '#include "hw/fpga.h"' + "\n\n" + body.strip("\n"))


TBF = '''#define TB(o) (*(volatile unsigned char *)(0x42E61FF8u + (o)))
void ui_window_open(u32 id);
'''

wf(0x400537C4, 380, "output_select_buttons",
   "Buttons 0x55C + 10i (i = 0..8): the first one released sets byte SYS+0x660 = i, applies the outputs (outputs_apply) and invalidates from window 3 (r1 is what outputs_apply left).", TBF + """
void touch_group_055C(void);
void outputs_apply(void);
void ui_window_invalidate_from(u32 id);

void output_select_buttons(void)
{
    u32 i;
    touch_group_055C();
    for (i = 0; i < 9; i++) {
        if (TB(0x55C + 10 * i) == 2) {
            TB(0x55C + 10 * i) = 0;
            TB(0) = 0;
            SYS8(0x660) = i;
            outputs_apply();
            ui_window_invalidate_from(3);
            return;
        }
    }
}
""")

wf(0x40052100, 384, "main_menu_buttons",
   "Buttons 0x2, 0xC, 0x16, 0x20 open windows 6, 0xA, 0xB, 0xC; 0x2A / 0x34 toggle SYS+0x802 / +0x803; 0x48 is only cleared; 0x3E saves a waveform (list_b_save_new); 0x52 resets the zoom and toggles SYS+0x2AA when the zoom count SYS+0x588 is above 12.", TBF + """
void touch_group_0002(void);
void list_b_save_new(void);
void zoom_reset(void);

void main_menu_buttons(void)
{
    u32 i;
    touch_group_0002();
    for (i = 0; i < 4; i++) {
        if (TB(2 + 10 * i) == 2) {
            TB(2 + 10 * i) = 0;
            TB(0) = 0;
            ui_window_open(i == 0 ? 6 : 9 + i);
            return;
        }
    }
    if (TB(0x2A) == 2) {
        TB(0x2A) = 0;
        TB(0) = 0;
        SYS8(0x802) = SYS8(0x802) == 0;
    } else if (TB(0x34) == 2) {
        TB(0x34) = 0;
        TB(0) = 0;
        SYS8(0x803) = SYS8(0x803) == 0;
    } else if (TB(0x48) == 2) {
        TB(0x48) = 0;
        TB(0) = 0;
    } else if (TB(0x3E) == 2) {
        TB(0x3E) = 0;
        TB(0) = 0;
        list_b_save_new();
    } else if (TB(0x52) == 2) {
        TB(0x52) = 0;
        TB(0) = 0;
        if (SYS8(0x588) > 12) {
            zoom_reset();
            SYS8(0x2AA) = SYS8(0x2AA) == 0;
        }
    }
}
""")

wf(0x4006233C, 24, "level_stats_61000",
   "When byte +2 of block 0x42E68668 is set (stock: falls into 0x40062354 with r3 = the block, ip = word +0xF0): for n = word +0xEC samples, v = sample * scale (+0xF0) / word +0xC0 / word +0x188 (sdiv), clamped to 0 .. scale, stores scale - v as u16 at 0x42E61000 and keeps the mean (+0x16C), max (+0x170) and min (+0x174).", """
#define MB8(o) (*(volatile unsigned char *)(0x42E68668u + (o)))
#define MB32(o) (*(volatile int *)(0x42E68668u + (o)))
#define SRC (*(volatile short *volatile *)0x42E68CB0u)

static int sdv(int a, int b)
{
    if (b == 0)
        return 0;
    if (b == -1)
        return (int)(0u - (u32)a);
    return a / b;
}

void level_stats_61000(void)
{
    int n, sc, d1, d2, mx = -32767, mn = 32767, i, v;
    u32 sum = 0;
    volatile short *s;
    volatile unsigned short *d = (volatile unsigned short *)0x42E61000u;
    if (MB8(2) == 0)
        return;
    sc = MB32(0xF0);
    n = MB32(0xEC);
    s = SRC;
    d2 = MB32(0x188);
    d1 = MB32(0xC0);
    for (i = 0; i < n; i++) {
        v = sdv(sdv((int)((u32)sc * (u32)(int)*s++), d1), d2);
        if (sc < v)
            v = sc;
        else if (v < 0)
            v = 0;
        v = sc - v;
        *d++ = v;
        if (v >= mx)
            mx = v;
        if (v <= mn)
            mn = v;
        sum += v;
    }
    MB32(0x16C) = sdv((int)sum, n);
    MB32(0x170) = mx;
    MB32(0x174) = mn;
}
""", src="trace")

wf(0x40064CF0, 200, "gen_selftest_64cf0",
   "Second generator self-test: settings 0x1797D51 / 1300 (with byte +0x2EFA = 0 and word 0x42E42888 = 0 first), 0x1279611 / 1500, 0xBC84B1 / 1200, 0x5DA66D / 1200, each applied with gen_update and checked by 0x400647F4 (0x15, 4, 1), (0x12, 5, 1), (0x16, 6, 2), (0x17, 7, 3); returns bit 0 of the AND of the four results.", """
void gen_update(void);
int func_400647f4(u32 a, u32 b, u32 c);
#define GB8(o) (*(volatile unsigned char *)(0x42E3F984u + (o)))
#define GB32(o) (*(volatile u32 *)(0x42E3F984u + (o)))

int gen_selftest_64cf0(void)
{
    u32 a, b, c;
    GB8(0x2EFA) = 0;
    GB32(0x2F04) = 0;
    GB32(0x2EFC) = 0x1797D51;
    GB32(0x2F00) = 1300;
    gen_update();
    a = func_400647f4(0x15, 4, 1);
    GB32(0x2EFC) = 0x1279611;
    GB32(0x2F00) = 1500;
    gen_update();
    a &= func_400647f4(0x12, 5, 1);
    GB32(0x2EFC) = 0xBC84B1;
    GB32(0x2F00) = 1200;
    gen_update();
    b = func_400647f4(0x16, 6, 2);
    GB32(0x2EFC) = 0x5DA66D;
    GB32(0x2F00) = 1200;
    gen_update();
    c = func_400647f4(0x17, 7, 3);
    return c & b & a & 1;
}
""", src="init")

wf(0x4005FE94, 228, "numpad_key",
   "Numpad key k: 1 toggles the sign byte 0x42E68AA1 (modes 2..4 of byte 0x42E68A89 only) and applies it; 2 appends '.' once (byte 0x42E68AA0); 3..12 append the digits '0'..'9' while the length byte 0x42E68A9F is below 10; 13 is backspace; 14..16 run 0x4005FC84 and close the numpad (numpad_close gets the registers 0x4005FC84 left).", """
void numpad_sign_apply(void);
void numpad_backspace(void);
void numpad_append_char(u32 c);
void func_4005fc84(void);
void numpad_close(void);
#define NP8(o) (*(volatile unsigned char *)(0x42E68A88u + (o)))

void numpad_key(u32 k)
{
    if (k == 1) {
        u32 m = NP8(1);
        if (m == 2 || m == 3 || m == 4) {
            NP8(0x19) = NP8(0x19) == 0;
            numpad_sign_apply();
        }
    } else if (k == 2) {
        if (NP8(0x18) == 0) {
            NP8(0x18) = 1;
            numpad_append_char(0x2E);
        }
    } else if (k == 0xD) {
        numpad_backspace();
    } else if (k - 3 < 10) {
        if (NP8(0x17) < 10)
            numpad_append_char((u32)(int)(signed char)(k + 0x2D));
    } else if (k == 0xE || k == 0xF || k == 0x10) {
        func_4005fc84();
        numpad_close();
    }
}
""")

wf(0x40061680, 292, "window_64_recalc",
   "Block 0x42E68668, by mode word +0x18C: 1 / 2 take the 64-bit limits a = +0x128, b = +0x120 and set the centre +0x130 = (a + b) / 2 and the span +0x138 = a - b; 3 limits the span to twice the centre, 4 moves the centre to span / 2 when twice the centre is below the span; both then set +0x120 / +0x128 = centre -/+ span / 2. Centre and span are always rewritten.", """
#define M64(o) (*(volatile long long *)(0x42E68668u + (o)))

void window_64_recalc(void)
{
    long long span = M64(0x138), c = M64(0x130), t, h;
    int mode = *(volatile int *)(0x42E68668u + 0x18C);
    if (mode == 1 || mode == 2) {
        long long a = M64(0x128), b = M64(0x120);
        c = (long long)((unsigned long long)b + (unsigned long long)a) / 2;
        span = (long long)((unsigned long long)a - (unsigned long long)b);
    } else if (mode == 3 || mode == 4) {
        t = (long long)((unsigned long long)c * 2u);
        if (t < span) {
            if (mode == 3)
                span = t;
            else
                c = span / 2;
        }
        h = span / 2;
        M64(0x120) = (long long)((unsigned long long)c - (unsigned long long)h);
        M64(0x128) = (long long)((unsigned long long)c + (unsigned long long)h);
    }
    M64(0x130) = c;
    M64(0x138) = span;
}
""", src="trace")

wf(0x40063A50, 368, "table_684_init",
   "Fills the 64 bytes SYS+0x684..0x6C3 with fixed values (0x40..0x55; stored byte by byte by the stock, here as 16 words). Their meaning is OPEN.", """
void table_684_init(void)
{
    volatile u32 *p = (volatile u32 *)(0x40544E68u + 0x684);
    p[0] = 0x5252534Du;
    p[1] = 0x4045484Cu;
    p[2] = 0x53545550u;
    p[3] = 0x42484A51u;
    p[4] = 0x52515350u;
    p[5] = 0x4045494Du;
    p[6] = 0x40444846u;
    p[7] = 0x343A3C40u;
    p[8] = 0x40414039u;
    p[9] = 0x4046464Bu;
    p[10] = 0x403A3B43u;
    p[11] = 0x40434442u;
    p[12] = 0x403A463Au;
    p[13] = 0x404C4C4Eu;
    p[14] = 0x40434649u;
    p[15] = 0x40464652u;
}
""", src="acquisition")
