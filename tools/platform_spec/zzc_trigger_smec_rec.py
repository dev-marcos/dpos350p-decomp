# The automatic trigger level (fewest crossings among evenly spaced candidate levels), the sync of
# eight byte parameters to the FPGA, and the field list of settings record C (the FFT spur lists).
# exec'd by gen_platform.py (do not define HDR, SPEC, OUT or f here)


def wr(addr, size, name, desc, body, src="acquisition"):
    f(addr, size, name, src, desc, '#include "hw/fpga.h"' + "\n\n" + body.strip("\n"))


COM = """
#define SYS 0x40544E68u
#define B8(a) (*(volatile unsigned char *)(a))
#define H16(a) (*(volatile unsigned short *)(a))
#define W32(a) (*(volatile u32 *)(a))
"""

wr(0x4006382C, 524, "trigger_level_auto",
   "Picks the trigger level u16 SYS+0x27E: the L = u16 SYS+0x632 candidate levels min + i x range / (L + 1) (i = 1..L; min / range = words +4 / +8 of SYS+0xB28, or SYS+0xB44 with SYS+0x27C set) "
   "are scored by the number of crossings (rising or falling) of the samples (W32 SYS+0x7B4 / +0x7B8 + 0x14, interleaved by SYS+0x558, u16 SYS+0x7BC - 0x14 per stream, each >> SYS+0x1BC); "
   "it takes the level with the fewest, or the middle one when its count x 4 is below the fewest x 5.",
   COM + """
void fill_u32(u32 *d, u32 v, u32 n);

/* register shift as the core does it: the low byte of n, 32 and more give 0 */
static u32 shr(u32 v, u32 n)
{
    n &= 0xFF;
    return n >= 32 ? 0 : v >> n;
}

void trigger_level_auto(void)
{
    u32 cnt[10];
    u32 st, n, base, lvb, seg, k, L, i, sh, idx, mn, c;
    int prev, cur, lv;
    volatile unsigned short *p, *q;
    st = W32(SYS + 0x558);
    n = W32(SYS + 0x7BC) & 0xFFFF;
    if (B8(SYS + 0x27C)) {
        base = W32(SYS + 0x7B8);
        lvb = SYS + 0xB44;
    } else {
        base = W32(SYS + 0x7B4);
        lvb = SYS + 0xB28;
    }
    fill_u32(cnt, 0, 10);
    seg = (u32)(((int)n - 0x14) / (int)st) & 0xFFFF;
    for (k = 0; (int)k < (int)st; k++) {
        p = (volatile unsigned short *)(base + 0x14 + 2 * k);
        sh = W32(SYS + 0x1BC);
        prev = (int)shr(*p, sh);
        if ((int)seg <= 1)
            continue;
        q = p + st;
        for (c = seg - 1; c; c--) {
            cur = (int)shr(*q, sh);
            L = H16(SYS + 0x632);
            for (i = 0; (int)i < (int)L;) {
                i++;
                lv = (int)(i * W32(lvb + 8) / (L + 1) + W32(lvb + 4));
                if (prev < lv) {
                    if (cur >= lv)
                        cnt[i - 1]++;
                } else if (lv < prev && lv >= cur) {
                    cnt[i - 1]++;
                }
            }
            prev = cur;
            q += st;
        }
    }
    L = H16(SYS + 0x632);
    mn = 0xFFFFFFFFu;
    idx = 0;
    for (i = 0; (int)i < (int)L; i++) {
        if (cnt[i] < mn) {
            idx = i;
            mn = cnt[i];
        }
    }
    if ((unsigned long long)cnt[L >> 1] * 4 < (unsigned long long)mn * 5)
        idx = L >> 1;
    H16(SYS + 0x27E) = (unsigned short)((idx + 1) * W32(lvb + 8) / (L + 1) + W32(lvb + 4));
}
""")

wr(0x40063CD0, 564, "smec_byte_params_sync",
   "For each of the 8 bytes SYS+0x674..+0x67B that differs from its copy SYS+0xBB8+i: updates the copy, sends SMEC job 0x19+i (fpga_param_updater, size 1), commits 0xB9+i and writes the byte to FPGA "
   "selector 0xA6+i. Then writes 0 and 1 to selector 0x11.",
   COM + """
void fpga_param_updater(u32 idx, u32 size, u32 v32, u32 vsmall);
void smec_commit(u32 idx);
void fpga_selector_command_write(u32 sel);
void fpga_data_byte_write(u32 v);

void smec_byte_params_sync(void)
{
    u32 i, v;
    for (i = 0; i < 8; i++) {
        v = B8(SYS + 0x674 + i);
        if (B8(SYS + 0xBB8 + i) != v) {
            B8(SYS + 0xBB8 + i) = (unsigned char)v;
            fpga_param_updater(0x19 + i, 1, W32(SYS + 0xBB8 + i), v);   /* the word load is unaligned for odd i */
            smec_commit(0xB9 + i);
            fpga_selector_command_write(0xA6 + i);
            fpga_data_byte_write(B8(SYS + 0xBB8 + i));
        }
    }
    fpga_selector_command_write(0x11);
    fpga_data_byte_write(0);
    fpga_selector_command_write(0x11);
    fpga_data_byte_write(1);
}
""")

wr(0x40043B0C, 348, "record_c_fields",
   "The field list of settings record C, the FFT spur lists at 0x42E3C000: cursor 0, the u16 counts +6 / +4 / +2 / +0, the float means +0x14 / +0x10 / +0xC / +8, "
   "then 200 / 400 / 650 / 1200 pairs (float value, u16 bin) from +0x2340 / +0x1D00 / +0x12D8 / +0x18 and +0x37F4 / +0x34D4 / +0x2FC0 / +0x2660.",
   COM + """
#define SPUR 0x42E3C000u

void serializer_set_cursor(u32 v);
void serializer_process_field(volatile void *p, u32 size);

static void pairs(u32 vals, u32 idxs, u32 n)
{
    u32 i;
    for (i = 0; i < n; i++) {
        serializer_process_field((volatile void *)(SPUR + vals + 4 * i), 4);
        serializer_process_field((volatile void *)(SPUR + idxs + 2 * i), 2);
    }
}

void record_c_fields(void)
{
    serializer_set_cursor(0);
    serializer_process_field((volatile void *)(SPUR + 6), 2);
    serializer_process_field((volatile void *)(SPUR + 4), 2);
    serializer_process_field((volatile void *)(SPUR + 2), 2);
    serializer_process_field((volatile void *)SPUR, 2);
    serializer_process_field((volatile void *)(SPUR + 0x14), 4);
    serializer_process_field((volatile void *)(SPUR + 0x10), 4);
    serializer_process_field((volatile void *)(SPUR + 0xC), 4);
    serializer_process_field((volatile void *)(SPUR + 8), 4);
    pairs(0x2340, 0x37F4, 200);
    pairs(0x1D00, 0x34D4, 400);
    pairs(0x12D8, 0x2FC0, 650);
    pairs(0x18, 0x2660, 1200);
}
""", src="storage")
