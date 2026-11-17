# The sectioned settings image (header H at offset 0, block 0x400448B4 at 0x32, then records I
# and J), the 141-entry UI text table at 0x42C2FE74 (four language columns), str_copy, the
# numeric entry pad (state 0x42E68A88, window 0x21) and four veneers into 0x4005F778.
# exec'd by gen_platform.py


def v(addr, size, name, desc, body, src="settings"):
    f(addr, size, name, src, desc, '#include "hw/fpga.h"' + "\n\n" + body.strip("\n"))


SER = """void serializer_set_cursor(u32 c);
void serializer_set_encode_mode(void);
void serializer_set_decode_mode(void);
u32 serializer_get_cursor(void);
void serializer_process_field(volatile void *p, u32 size);
"""

# ---------------------------------------------------------------- sectioned settings image
v(0x40045580, 100, "settings_i_fields",
  "Field list of record I: the halfwords SYS+0x109684 (w) and SYS+0x109686 (h), then w x h halfwords from SYS+0x109688 (count read after the two fields, signed; nothing when < 1).", SER + """
void settings_i_fields(void)
{
    int n, i;
    serializer_process_field(&SYS16(0x109684), 2);
    serializer_process_field(&SYS16(0x109686), 2);
    n = (int)((u32)SYS16(0x109684) * (u32)SYS16(0x109686));
    for (i = 0; i < n; i++)
        serializer_process_field(&SYS16(0x109688 + 2 * i), 2);
}
""")
v(0x400455E4, 140, "settings_j_fields",
  "Field list of record J: when SYS+0x3E is set, SYS+0x2B0 halfwords from 0x42AC7CF4; when SYS+0x102 is set, SYS+0x2B0 halfwords from 0x42B29F44 (the count is re-read every step).", SER + """
void settings_j_fields(void)
{
    u32 i;
    if (SYS8(0x3E))
        for (i = 0; i < SYS32(0x2B0); i++)
            serializer_process_field((volatile void *)(0x42AC7CF4u + 2 * i), 2);
    if (SYS8(0x102))
        for (i = 0; i < SYS32(0x2B0); i++)
            serializer_process_field((volatile void *)(0x42B29F44u + 2 * i), 2);
}
""")
v(0x40045670, 24, "settings_h_decode", "Decode mode, then settings_h_fields (the header at cursor 0).", SER + """
void settings_h_fields(void);

void settings_h_decode(void)
{
    serializer_set_decode_mode();
    settings_h_fields();
}
""")
v(0x40045688, 36, "settings_block_448b4_decode", "Decode mode, cursor = SYS+0x75C, then the field list 0x400448B4.", SER + """
void func_400448b4(void);

void settings_block_448b4_decode(void)
{
    serializer_set_decode_mode();
    serializer_set_cursor(SYS32(0x75C));
    func_400448b4();
}
""")
v(0x400456AC, 36, "settings_i_decode", "Decode mode, cursor = SYS+0x760, then settings_i_fields.", SER + """
void settings_i_fields(void);

void settings_i_decode(void)
{
    serializer_set_decode_mode();
    serializer_set_cursor(SYS32(0x760));
    settings_i_fields();
}
""")
v(0x400456D0, 36, "settings_j_decode", "Decode mode, cursor = SYS+0x764, then settings_j_fields.", SER + """
void settings_j_fields(void);

void settings_j_decode(void)
{
    serializer_set_decode_mode();
    serializer_set_cursor(SYS32(0x764));
    settings_j_fields();
}
""")
v(0x400456F4, 32, "settings_image_decode", "settings_h_decode, settings_block_448b4_decode, settings_i_decode, settings_j_decode.", """
void settings_h_decode(void);
void settings_block_448b4_decode(void);
void settings_i_decode(void);
void settings_j_decode(void);

void settings_image_decode(void)
{
    settings_h_decode();
    settings_block_448b4_decode();
    settings_i_decode();
    settings_j_decode();
}
""")
v(0x40045714, 116, "settings_image_encode",
  "Encode mode; block 0x400448B4 from cursor 0x32 (SYS+0x75C = 0x32), then records I and J, recording each section start (SYS+0x75C / 0x760 / 0x764), size (SYS+0x768 / 0x76C / 0x770) and the end (SYS+0x774); finally the header H at cursor 0.", SER + """
void func_400448b4(void);
void settings_i_fields(void);
void settings_j_fields(void);
void settings_h_fields(void);

void settings_image_encode(void)
{
    serializer_set_encode_mode();
    SYS32(0x75C) = 0x32;
    serializer_set_cursor(0x32);
    func_400448b4();
    SYS32(0x768) = serializer_get_cursor() - SYS32(0x75C);
    SYS32(0x760) = serializer_get_cursor();
    settings_i_fields();
    SYS32(0x76C) = serializer_get_cursor() - SYS32(0x760);
    SYS32(0x764) = serializer_get_cursor();
    settings_j_fields();
    SYS32(0x770) = serializer_get_cursor() - SYS32(0x764);
    SYS32(0x774) = serializer_get_cursor();
    settings_h_fields();
}
""")

# ---------------------------------------------------------------- UI text table
TXT = """#define TXT(k) (0x42C2FE74u + (k) * 0x1A0u)   /* 141 entries of 0x1A0 bytes */
#define TB(a)   (*(volatile unsigned char *)(a))
"""
v(0x400342B4, 24, "str_copy", "Copies the NUL-terminated string s to d, terminator included; returns d advanced past the terminator.", """
unsigned char *str_copy(unsigned char *d, const unsigned char *s)
{
    do
        *d++ = *s;
    while (*s++);
    return d;
}
""", src="string")
v(0x400457B0, 236, "ui_text_language_apply",
  "For the language column L = SYS+0x7AC (1..4; anything else does nothing), every one of the 141 text entries gets +0 = pointer to its column L string (+4 + 0x64 x (L - 1)) and the two bytes +0x194/+0x195 = that column's pair at +0x196 + 2 x (L - 1). The language reading of SYS+0x7AC is DERIVED.", TXT + """
void ui_text_language_apply(void)
{
    u32 L = SYS8(0x7AC), k, e;
    if (L < 1 || L > 4)
        return;
    L -= 1;
    for (k = 0; k < 141; k++) {
        e = TXT(k);
        *(volatile u32 *)e = e + 4 + 0x64 * L;
        TB(e + 0x194) = TB(e + 0x196 + 2 * L);
        TB(e + 0x195) = TB(e + 0x197 + 2 * L);
    }
}
""", src="ui")
v(0x4004579C, 20, "ui_text_init", "Fills the text table (0x40045974, 13.5 KB of str_copy calls), then falls into ui_text_language_apply.", """
void func_40045974(void);
void ui_text_language_apply(void);

void ui_text_init(void)
{
    func_40045974();
    ui_text_language_apply();
}
""", src="ui")
for _a, _col in ((0x4004589C, 0), (0x400458CC, 1), (0x400458FC, 2), (0x4004592C, 3)):
    v(_a, 48, "ui_text_set_lang%d" % (_col + 1),
      "Text entry k, language column %d: str_copy(entry + 0x%X, s), then the bytes +0x%X / +0x%X = a / b." % (_col + 1, 4 + 0x64 * _col, 0x196 + 2 * _col, 0x197 + 2 * _col),
      TXT + """unsigned char *str_copy(unsigned char *d, const unsigned char *s);

void ui_text_set_lang%d(u32 k, const unsigned char *s, u32 a, u32 b)
{
    u32 e = TXT(k);
    str_copy((unsigned char *)(e + 0x%X), s);
    TB(e + 0x%X) = a;
    TB(e + 0x%X) = b;
}
""" % (_col + 1, 4 + 0x64 * _col, 0x196 + 2 * _col, 0x197 + 2 * _col), src="ui")

# ---------------------------------------------------------------- numeric entry pad
PAD = """#define PAD(o)  (*(volatile unsigned char *)(0x42E68A88u + (o)))
#define WIN21   ((volatile unsigned short *)(0x42E64520u + 0x21u * 14u))
"""
v(0x4005F914, 8, "mem_clear", "__aeabi_memset_entry(d, n, 0) (enters the stock memset after its fill-byte spreading).", """
void *__aeabi_memset_entry(void *d, unsigned int n, int v);

void mem_clear(void *d, unsigned int n)
{
    __aeabi_memset_entry(d, n, 0);
}
""", src="string")
v(0x4005FB4C, 72, "numpad_reset",
  "Entry pad state 0x42E68A88: the 20-byte text +3 cleared, +0 (16 bit) = 0, length +0x17 = 0, the '.'/'-' flags +0x18 (16 bit) = 0, value +0x20 (double) = 0.0, and the pointers +0x28 -> +0x38, +0x2C -> +0x40, +0x30 -> +0x48.", PAD + """
void mem_clear(void *d, unsigned int n);

void numpad_reset(void)
{
    mem_clear((void *)0x42E68A8Bu, 0x14);
    *(volatile unsigned short *)0x42E68A88u = 0;
    PAD(0x17) = 0;
    *(volatile unsigned short *)0x42E68AA0u = 0;
    *(volatile double *)0x42E68AA8u = 0.0;
    *(volatile u32 *)0x42E68AB4u = 0x42E68AC8u;
    *(volatile u32 *)0x42E68AB8u = 0x42E68AD0u;
    *(volatile u32 *)0x42E68AB0u = 0x42E68AC0u;
}
""", src="ui")
v(0x4005FB94, 32, "numpad_append_char", "Text[+0x17] = c, then the length +0x17 += 1 (no bound check).", PAD + """
void numpad_append_char(u32 c)
{
    PAD(3 + PAD(0x17)) = c;
    PAD(0x17) = PAD(0x17) + 1;
}
""", src="ui")
v(0x4005FBB4, 80, "numpad_backspace",
  "Length -1 when not 0; the character now at the end clears the flag +0x19 when it is '-' or +0x18 when it is '.', and is replaced by NUL.", PAD + """
void numpad_backspace(void)
{
    int c;
    if (PAD(0x17))
        PAD(0x17) = PAD(0x17) - 1;
    c = (signed char)PAD(3 + PAD(0x17));
    if (c == '-')
        PAD(0x19) = 0;
    else if (c == '.')
        PAD(0x18) = 0;
    PAD(3 + PAD(0x17)) = 0;
}
""", src="ui")
for _a, _m in ((0x4005F9F0, 1), (0x4005FA34, 3)):
    v(_a, 68, "numpad_open_mode%d" % _m,
      "numpad_reset, mode byte +1 = %d, window 0x21 position = (x, y), then ui_window_open(0x21)." % _m, PAD + """void numpad_reset(void);
void ui_window_open(u32 id);

void numpad_open_mode%d(u32 x, u32 y)
{
    numpad_reset();
    PAD(1) = %d;
    WIN21[4] = y;
    WIN21[3] = x;
    ui_window_open(0x21);
}
""" % (_m, _m), src="ui")
v(0x4005FB44, 8, "numpad_close", "ui_window_close(0x21); the caller's r1/r2 pass through to its hidden slot argument.", """
void ui_window_close(u32 id, u32 r1, u32 r2);

void numpad_close(u32 r0, u32 r1, u32 r2)
{
    (void)r0;
    ui_window_close(0x21, r1, r2);
}
""", src="ui")

# ---------------------------------------------------------------- veneers into 0x4005F778
v(0x4005F8D8, 16, "trace_span_full", "0x4005F778(tr, 0, SYS+0x2C0).", """
void trace_scale_segment(u32 tr, u32 start, u32 len);

void trace_span_full(u32 tr)
{
    trace_scale_segment(tr, 0, SYS32(0x2C0));
}
""", src="trace")
v(0x4005F8E8, 20, "trace_span_window", "0x4005F778(tr, SYS+0x6FC, SYS+0x6F6 (16 bit)).", """
void trace_scale_segment(u32 tr, u32 start, u32 len);

void trace_span_window(u32 tr)
{
    trace_scale_segment(tr, SYS32(0x6FC), SYS16(0x6F6));
}
""", src="trace")
v(0x4005F90C, 4, "trace_span_full_veneer", "Branch to trace_span_full.", """
void trace_span_full(u32 tr);

void trace_span_full_veneer(u32 tr)
{
    trace_span_full(tr);
}
""", src="trace")
v(0x4005F910, 4, "trace_span_window_veneer", "Branch to trace_span_window.", """
void trace_span_window(u32 tr);

void trace_span_window_veneer(u32 tr)
{
    trace_span_window(tr);
}
""", src="trace")
