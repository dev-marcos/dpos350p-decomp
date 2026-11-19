# Field lists of three settings records (the serializer of batch 15 runs one call per field, in
# encode or decode mode): the user slot, the main settings image, and the calibration table.
# The call sequences were taken from stock runs (tools/gen_call_list.py) and folded into loops.
# exec'd by gen_platform.py (do not define HDR, SPEC, OUT or f here)


def wr(addr, size, name, desc, body, src="storage"):
    f(addr, size, name, src, desc, '#include "hw/fpga.h"' + "\n\n" + body.strip("\n"))


COM = """
#define SYS 0x40544E68u
#define BLK 0x42E68668u    /* level (spectrum) view block */
#define BODE 0x42E180C8u   /* frequency-response view block */
#define B8(a) (*(volatile unsigned char *)(a))
#define H16(a) (*(volatile unsigned short *)(a))

void serializer_set_cursor(u32 v);
void serializer_process_field(volatile void *p, u32 size);

#define FLD(p, s) serializer_process_field((volatile void *)(p), (s))
"""

wr(0x40043F54, 1032, "slot_fields",
   "The field list of a user slot (settings_slot_save / _load): cursor 0, then 121 fields: the acquisition and display state of SYS, the generator words 0x42E3F984+0x2EFA.., "
   "the level-view settings (BLK +1..+3, +0x58.., +0xF4, the 8 s64 at +0x120, +0x188 / +0x18C), the Bode sweep settings (BODE +0x44 / +0x46 / +0x4C.. / +0x60) and 0x42E22D28 .. +8.",
   COM + """
void slot_fields(void)
{
    int j;
    serializer_set_cursor(0);
    FLD(SYS + 0x664, 1);
    FLD(SYS + 0x671, 1);
    FLD(SYS + 0x588, 1);
    FLD(SYS + 0x598, 1);
    FLD(SYS + 0x2AA, 1);
    FLD(SYS + 0x570, 8);
    FLD(SYS + 0x578, 8);
    for (j = 0; j < 5; j++) {
        FLD(SYS + 0x3E + 0x1 * j, 1);
    }
    FLD(SYS + 0x60, 4);
    FLD(SYS + 0x46, 1);
    for (j = 0; j < 5; j++) {
        FLD(SYS + 0x102 + 0x1 * j, 1);
    }
    FLD(SYS + 0x120, 4);
    FLD(SYS + 0x10A, 1);
    FLD(SYS + 0x27B, 1);
    FLD(SYS + 0x27C, 1);
    FLD(SYS + 0x27E, 2);
    FLD(SYS + 0x284, 1);
    for (j = 0; j < 3; j++) {
        FLD(SYS + 0x654 - 0x1 * j, 1);
    }
    FLD(SYS + 0x660, 1);
    FLD(SYS + 0x655, 1);
    FLD(SYS + 0x802, 1);
    FLD(SYS + 0x803, 1);
    for (j = 0; j < 4; j++) {
        FLD(SYS + 0x808 + 0x4 * j, 4);
    }
    FLD(SYS + 0x804, 1);
    for (j = 0; j < 50; j++) {
        FLD(SYS + 0x880 + 0x2 * j, 2);
    }
    FLD(0x42E3F984u + 0x2EFA, 1);
    for (j = 0; j < 4; j++) {
        FLD(0x42E3F984u + 0x2EFC + 0x4 * j, 4);
    }
    for (j = 0; j < 3; j++) {
        FLD(BLK + 0x1 + 0x1 * j, 1);
    }
    for (j = 0; j < 3; j++) {
        FLD(BLK + 0x58 + 0x4 * j, 4);
    }
    FLD(BLK + 0xF4, 4);
    for (j = 0; j < 8; j++) {
        FLD(BLK + 0x120 + 0x8 * j, 8);
    }
    FLD(BLK + 0x188, 4);
    FLD(BLK + 0x18C, 4);
    FLD(BODE + 0x44, 1);
    FLD(BODE + 0x46, 1);
    for (j = 0; j < 5; j++) {
        FLD(BODE + 0x4C + 0x4 * j, 4);
    }
    FLD(BODE + 0x60, 8);
    FLD(0x42E22D28u + 0x0, 1);
    FLD(0x42E22D28u + 0x4, 4);
    FLD(0x42E22D28u + 0x8, 4);
}
""")

wr(0x400448B4, 3276, "settings_image_fields",
   "The field list of the main settings image (settings_image_encode starts it at cursor 0x32): the fields of SYS from +4 up, then u16 SYS+0x5D0 triples of words "
   "+0x2C4 / +0x38C / +0x454, the block from +0x800, and B8 SYS+0x8E4 u16 values from +0x880. Both counts are re-read on every pass, so a decode that loads them takes effect at once.",
   COM + """
void settings_image_fields(void)
{
    int j;
    for (j = 0; j < 5; j++) {
        FLD(SYS + 0x4 + 0x2 * j, 2);
    }
    FLD(SYS + 0xC, 2);
    for (j = 0; j < 12; j++) {
        FLD(SYS + 0x10 + 0x2 * j, 2);
    }
    FLD(SYS + 0x28, 1);
    FLD(SYS + 0x2A, 1);
    FLD(SYS + 0x2C, 2);
    FLD(SYS + 0x30, 8);
    FLD(SYS + 0x39, 1);
    for (j = 0; j < 7; j++) {
        FLD(SYS + 0x3C + 0x1 * j, 1);
    }
    FLD(SYS + 0x44, 2);
    FLD(SYS + 0x46, 1);
    FLD(SYS + 0x50, 8);
    FLD(SYS + 0x58, 8);
    FLD(SYS + 0x60, 4);
    FLD(SYS + 0x64, 4);
    FLD(SYS + 0xD8, 8);
    for (j = 0; j < 3; j++) {
        FLD(SYS + 0xE0 + 0x4 * j, 4);
    }
    for (j = 0; j < 6; j++) {
        FLD(SYS + 0xEC + 0x2 * j, 2);
    }
    FLD(SYS + 0x100, 1);
    for (j = 0; j < 5; j++) {
        FLD(SYS + 0x102 + 0x1 * j, 1);
    }
    FLD(SYS + 0x108, 2);
    FLD(SYS + 0x10A, 1);
    FLD(SYS + 0x110, 8);
    FLD(SYS + 0x118, 8);
    FLD(SYS + 0x120, 4);
    FLD(SYS + 0x124, 4);
    FLD(SYS + 0x198, 8);
    for (j = 0; j < 3; j++) {
        FLD(SYS + 0x1A0 + 0x4 * j, 4);
    }
    for (j = 0; j < 3; j++) {
        FLD(SYS + 0x1B0 + 0x1 * j, 1);
    }
    for (j = 0; j < 3; j++) {
        FLD(SYS + 0x1B8 + 0x1 * j, 1);
    }
    FLD(SYS + 0x1BC, 4);
    FLD(SYS + 0x1C0, 4);
    for (j = 0; j < 4; j++) {
        FLD(SYS + 0x264 + 0x4 * j, 4);
    }
    FLD(SYS + 0x274, 1);
    FLD(SYS + 0x275, 1);
    FLD(SYS + 0x276, 2);
    for (j = 0; j < 5; j++) {
        FLD(SYS + 0x278 + 0x1 * j, 1);
    }
    for (j = 0; j < 3; j++) {
        FLD(SYS + 0x27E + 0x2 * j, 2);
    }
    for (j = 0; j < 3; j++) {
        FLD(SYS + 0x288 + 0x4 * j, 4);
    }
    FLD(SYS + 0x29C, 2);
    FLD(SYS + 0x2A0, 4);
    for (j = 0; j < 3; j++) {
        FLD(SYS + 0x2A4 + 0x1 * j, 1);
    }
    FLD(SYS + 0x2A9, 1);
    FLD(SYS + 0x2AA, 1);
    FLD(SYS + 0x2AC, 2);
    for (j = 0; j < 5; j++) {
        FLD(SYS + 0x2B0 + 0x4 * j, 4);
    }
    FLD(SYS + 0x520, 4);
    FLD(SYS + 0x524, 4);
    FLD(SYS + 0x530, 8);
    FLD(SYS + 0x538, 8);
    FLD(SYS + 0x528, 8);
    FLD(SYS + 0x540, 4);
    for (j = 0; j < 5; j++) {
        FLD(SYS + 0x544 + 0x2 * j, 2);
    }
    for (j = 0; j < 7; j++) {
        FLD(SYS + 0x554 + 0x4 * j, 4);
    }
    FLD(SYS + 0x570, 8);
    FLD(SYS + 0x578, 8);
    FLD(SYS + 0x581, 1);
    FLD(SYS + 0x588, 1);
    FLD(SYS + 0x590, 8);
    FLD(SYS + 0x598, 1);
    FLD(SYS + 0x5A0, 8);
    FLD(SYS + 0x550, 2);
    FLD(SYS + 0x54F, 1);
    FLD(SYS + 0x5D0, 2);
    FLD(SYS + 0x5A8, 8);
    FLD(SYS + 0x5B0, 1);
    FLD(SYS + 0x5B8, 8);
    FLD(SYS + 0x5C0, 1);
    FLD(SYS + 0x5C8, 8);
    FLD(SYS + 0x5D4, 4);
    FLD(SYS + 0x5D8, 4);
    FLD(SYS + 0x5E0, 8);
    FLD(SYS + 0x5E8, 4);
    FLD(SYS + 0x5EC, 4);
    FLD(SYS + 0x5F4, 2);
    for (j = 0; j < 12; j++) {
        FLD(SYS + 0x5F8 + 0x4 * j, 4);
    }
    FLD(SYS + 0x628, 1);
    FLD(SYS + 0x634, 4);
    FLD(SYS + 0x63C, 4);
    for (j = 0; j < 3; j++) {
        FLD(SYS + 0x640 + 0x2 * j, 2);
    }
    for (j = 0; j < 4; j++) {
        FLD(SYS + 0x652 + 0x1 * j, 1);
    }
    FLD(SYS + 0x660, 1);
    FLD(SYS + 0x284, 1);
    FLD(SYS + 0x664, 1);
    for (j = 0; j < 5; j++) {
        FLD(SYS + 0x666 + 0x1 * j, 1);
    }
    for (j = 0; j < 3; j++) {
        FLD(SYS + 0x671 + 0x1 * j, 1);
    }
    for (j = 0; j < 3; j++) {
        FLD(SYS + 0x710 + 0xC * j, 4);
        FLD(SYS + 0x714 + 0xC * j, 4);
    }
    FLD(SYS + 0x724, 4);
    FLD(SYS + 0x730, 4);
    for (j = 0; j < 6; j++) {
        FLD(SYS + 0x738 + 0x4 * j, 4);
    }
    for (j = 0; j < 5; j++) {
        FLD(SYS + 0x750 + 0x2 * j, 2);
    }
    for (j = 0; j < (int)H16(SYS + 0x5D0); j++) {   /* count re-read: decoding may have just set it */
        FLD(SYS + 0x2C4 + 4 * j, 4);
        FLD(SYS + 0x38C + 4 * j, 4);
        FLD(SYS + 0x454 + 4 * j, 4);
    }
    for (j = 0; j < 5; j++) {
        FLD(SYS + 0x800 + 0x1 * j, 1);
    }
    for (j = 0; j < 4; j++) {
        FLD(SYS + 0x808 + 0x4 * j, 4);
    }
    for (j = 0; j < 24; j++) {
        FLD(SYS + 0x9A8 + 0x8 * j, 8);
    }
    for (j = 0; j < (int)B8(SYS + 0x8E4); j++)
        FLD(SYS + 0x880 + 2 * j, 2);
}
""")

wr(0x40044450, 560, "calibration_process_serialized_fields",
   "The field list of the calibration record: cursor 0, then 50 rows of the 32 word columns of the table 0x42E53A0C (+0x190 + 0xC8 x column + 4 x row), then 20 rows of the four word "
   "columns +0x50 / +0xF0 / +0xA0 / +0x140. (name: Hack)",
   COM + """
void calibration_process_serialized_fields(void)
{
    int j;
    serializer_set_cursor(0);
    for (j = 0; j < 50; j++) {
        FLD(0x42E53A0Cu + 0x190 + 0x4 * j, 4);
        FLD(0x42E53A0Cu + 0x258 + 0x4 * j, 4);
        FLD(0x42E53A0Cu + 0x320 + 0x4 * j, 4);
        FLD(0x42E53A0Cu + 0x3E8 + 0x4 * j, 4);
        FLD(0x42E53A0Cu + 0x4B0 + 0x4 * j, 4);
        FLD(0x42E53A0Cu + 0x578 + 0x4 * j, 4);
        FLD(0x42E53A0Cu + 0x640 + 0x4 * j, 4);
        FLD(0x42E53A0Cu + 0x708 + 0x4 * j, 4);
        FLD(0x42E53A0Cu + 0x7D0 + 0x4 * j, 4);
        FLD(0x42E53A0Cu + 0x898 + 0x4 * j, 4);
        FLD(0x42E53A0Cu + 0x960 + 0x4 * j, 4);
        FLD(0x42E53A0Cu + 0xA28 + 0x4 * j, 4);
        FLD(0x42E53A0Cu + 0xAF0 + 0x4 * j, 4);
        FLD(0x42E53A0Cu + 0xBB8 + 0x4 * j, 4);
        FLD(0x42E53A0Cu + 0xC80 + 0x4 * j, 4);
        FLD(0x42E53A0Cu + 0xD48 + 0x4 * j, 4);
        FLD(0x42E53A0Cu + 0xE10 + 0x4 * j, 4);
        FLD(0x42E53A0Cu + 0xED8 + 0x4 * j, 4);
        FLD(0x42E53A0Cu + 0xFA0 + 0x4 * j, 4);
        FLD(0x42E53A0Cu + 0x1068 + 0x4 * j, 4);
        FLD(0x42E53A0Cu + 0x1130 + 0x4 * j, 4);
        FLD(0x42E53A0Cu + 0x11F8 + 0x4 * j, 4);
        FLD(0x42E53A0Cu + 0x12C0 + 0x4 * j, 4);
        FLD(0x42E53A0Cu + 0x1388 + 0x4 * j, 4);
        FLD(0x42E53A0Cu + 0x1450 + 0x4 * j, 4);
        FLD(0x42E53A0Cu + 0x1518 + 0x4 * j, 4);
        FLD(0x42E53A0Cu + 0x15E0 + 0x4 * j, 4);
        FLD(0x42E53A0Cu + 0x16A8 + 0x4 * j, 4);
        FLD(0x42E53A0Cu + 0x1770 + 0x4 * j, 4);
        FLD(0x42E53A0Cu + 0x1838 + 0x4 * j, 4);
        FLD(0x42E53A0Cu + 0x1900 + 0x4 * j, 4);
        FLD(0x42E53A0Cu + 0x19C8 + 0x4 * j, 4);
    }
    for (j = 0; j < 20; j++) {
        FLD(0x42E53A0Cu + 0x50 + 0x4 * j, 4);
        FLD(0x42E53A0Cu + 0xF0 + 0x4 * j, 4);
        FLD(0x42E53A0Cu + 0xA0 + 0x4 * j, 4);
        FLD(0x42E53A0Cu + 0x140 + 0x4 * j, 4);
    }
}
""")
