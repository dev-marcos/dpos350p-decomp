# More settings records (the 5 user slots, calibration, records G and H, load-all) and the
# small helpers around the time-base / zoom index SYS_INFO+0x598. exec'd by gen_platform.py


def r(addr, size, name, desc, body, src="settings"):
    f(addr, size, name, src, desc, '#include "hw/fpga.h"' + "\n\n" + body.strip("\n"))


SER = """void serializer_set_cursor(u32 c);
void serializer_set_encode_mode(void);
void serializer_set_decode_mode(void);
void serializer_process_field(volatile void *p, u32 size);
void persistence_read(u32 dev, u32 addr, volatile void *buf, u32 len);
void persistence_write_read_modify_write(u32 dev, u32 addr, volatile void *buf, u32 len);
#define SER_IMAGE ((volatile void *)0x408A526Cu)   /* SYS_INFO+0x360404 */
"""
SLOT = """
static u32 slot_addr(void)
{
    u32 s = SYS8(0x7C0) - 1;
    return s <= 4 ? SYS32(0x7C8 + 4 * s) : 0;
}
"""

# ---------------------------------------------------------------- user slots
r(0x4004435C, 120, "settings_slot_save",
  "Saves the current user slot (SYS_INFO+0x7C0 = 1..5): encode the field list 0x40043F54, then write 0x1000 bytes of the image to the flash address SYS+0x7C8 + 4 x (slot - 1). Stock quirk: for a slot outside 1..5 the address is whatever the caller left in r4; the C uses 0 and the tests keep the slot in range.", SER + """void func_40043f54(void);
""" + SLOT + """
void settings_slot_save(void)
{
    serializer_set_encode_mode();
    func_40043f54();
    persistence_write_read_modify_write(0, slot_addr(), SER_IMAGE, 0x1000);
}
""")
r(0x400443D4, 124, "settings_slot_load",
  "Loads the current user slot: read 0x1000 bytes from SYS+0x7C8 + 4 x (slot - 1), then decode the field list 0x40043F54. Same out-of-range quirk as settings_slot_save (stock: the caller's r1).", SER + """void func_40043f54(void);
""" + SLOT + """
void settings_slot_load(void)
{
    persistence_read(0, slot_addr(), SER_IMAGE, 0x1000);
    serializer_set_decode_mode();
    func_40043f54();
}
""")

# ---------------------------------------------------------------- calibration, G, H
r(0x40044680, 52, "calibration_save_wrapper", "Encodes the calibration fields and writes 0x5000 bytes of the image to the flash address SYS+0x7E0. (name: Hack)", SER + """void calibration_process_serialized_fields(void);

void calibration_save_wrapper(void)
{
    serializer_set_encode_mode();
    calibration_process_serialized_fields();
    persistence_write_read_modify_write(0, SYS32(0x7E0), SER_IMAGE, 0x5000);
}
""", src="settings; name: Hack")
r(0x400446B4, 52, "calibration_load_wrapper", "Reads 0x5000 bytes from SYS+0x7E0 into the image and decodes the calibration fields. (name: Hack)", SER + """void calibration_process_serialized_fields(void);

void calibration_load_wrapper(void)
{
    persistence_read(0, SYS32(0x7E0), SER_IMAGE, 0x5000);
    serializer_set_decode_mode();
    calibration_process_serialized_fields();
}
""", src="settings; name: Hack")
r(0x400446E8, 68, "settings_g_fields", "Field list of record G: the three words at 0x42E3F984 + 0x2F64 / 0x2F68 / 0x2F6C.", SER + """
void settings_g_fields(void)
{
    volatile unsigned char *b = (volatile unsigned char *)0x42E3F984u;
    serializer_set_cursor(0);
    serializer_process_field(b + 0x2F64, 4);
    serializer_process_field(b + 0x2F68, 4);
    serializer_process_field(b + 0x2F6C, 4);
}
""")
r(0x4004472C, 52, "settings_g_save", "Saves record G: encode, then write 0x1000 bytes to the flash address SYS+0x7E8.", SER + """void settings_g_fields(void);

void settings_g_save(void)
{
    serializer_set_encode_mode();
    settings_g_fields();
    persistence_write_read_modify_write(0, SYS32(0x7E8), SER_IMAGE, 0x1000);
}
""")
r(0x40044760, 52, "settings_g_load", "Loads record G from the flash address SYS+0x7E8.", SER + """void settings_g_fields(void);

void settings_g_load(void)
{
    persistence_read(0, SYS32(0x7E8), SER_IMAGE, 0x1000);
    serializer_set_decode_mode();
    settings_g_fields();
}
""")
r(0x40044794, 52, "settings_load_all", "Start-up load of every record: E (rebuilds defaults without the magic), F, the current slot, B, calibration, A, G, D, then C.", """
void settings_e_load(void);
void settings_f_load(void);
void settings_slot_load(void);
void settings_b_load(void);
void calibration_load_wrapper(void);
void settings_a_load(void);
void settings_g_load(void);
void settings_d_load(void);
void settings_c_load(void);

void settings_load_all(void)
{
    settings_e_load();
    settings_f_load();
    settings_slot_load();
    settings_b_load();
    calibration_load_wrapper();
    settings_a_load();
    settings_g_load();
    settings_d_load();
    settings_c_load();
}
""")
r(0x4004480C, 124, "settings_h_fields", "Field list of record H: the words SYS_INFO+0x75C, 0x768, 0x760, 0x76C, 0x764, 0x770, 0x774, in that order.", SER + """
void settings_h_fields(void)
{
    serializer_set_cursor(0);
    serializer_process_field(&SYS32(0x75C), 4);
    serializer_process_field(&SYS32(0x768), 4);
    serializer_process_field(&SYS32(0x760), 4);
    serializer_process_field(&SYS32(0x76C), 4);
    serializer_process_field(&SYS32(0x764), 4);
    serializer_process_field(&SYS32(0x770), 4);
    serializer_process_field(&SYS32(0x774), 4);
}
""")

# ---------------------------------------------------------------- time-base index helpers
r(0x4003BE50, 40, "timebase_index_set", "SYS+0x588 = v, then 0x4003BCF0, then the halfwords SYS+0x86E and +0x870 = 1.", """
void func_4003bcf0(void);

void timebase_index_set(u32 v)
{
    SYS8(0x588) = v;
    func_4003bcf0();
    SYS16(0x86E) = 1;
    SYS16(0x870) = 1;
}
""", src="timebase")
r(0x4003BE98, 16, "sys_word_pair_570_set", "SYS+0x570 = lo, SYS+0x574 = hi (a 64-bit value in two words).", """
void sys_word_pair_570_set(u32 lo, u32 hi)
{
    SYS32(0x570) = lo;
    SYS32(0x574) = hi;
}
""", src="timebase")
r(0x400424C4, 16, "sys_word_pair_578_set", "SYS+0x578 = lo, SYS+0x57C = hi.", """
void sys_word_pair_578_set(u32 lo, u32 hi)
{
    SYS32(0x578) = lo;
    SYS32(0x57C) = hi;
}
""", src="timebase")
r(0x40041F5C, 96, "zoom_views_update", "Unless the byte 0x42E68BC7 is set, and only when SYS+0x2AA, SYS+0x28 and SYS+0x652 are all 0: 0x40041BA0, 0x40041D38, 0x40041E34.", """
void func_40041ba0(void);
void func_40041d38(void);
void func_40041e34(void);

void zoom_views_update(void)
{
    if (*(volatile unsigned char *)0x42E68BC7u)
        return;
    if (SYS8(0x2AA) || SYS8(0x28) || SYS8(0x652))
        return;
    func_40041ba0();
    func_40041d38();
    func_40041e34();
}
""", src="timebase")
r(0x40041FF8, 36, "zoom_entry_select", "Copies the 64-bit entry SYS+0xBD0 + 8 x SYS+0x598 to SYS+0x5A0.", """
void zoom_entry_select(void)
{
    volatile u32 *s = &SYS32(0xBD0 + 8 * SYS8(0x598));
    u32 lo = s[0], hi = s[1];
    SYS32(0x5A0) = lo;
    SYS32(0x5A4) = hi;
}
""", src="timebase")
r(0x40042350, 104, "zoom_mode_check",
  "Clears SYS+0x2AA and returns 1 when SYS+0x28 is set with SYS+0x3C clear, when SYS+0x652 is set, or when SYS+0x5B0 equals SYS+0x2A8; otherwise returns 0. Then clamps SYS+0x598 to at most SYS+0x5B0 - 1 (signed compare).", """
u32 zoom_mode_check(void)
{
    u32 ret = 0, lim, cur;
    if (SYS8(0x28) && SYS8(0x3C) == 0) {
        SYS8(0x2AA) = 0;
        ret = 1;
    }
    cur = SYS8(0x598);
    if (SYS8(0x652)) {
        SYS8(0x2AA) = 0;
        ret = 1;
    }
    lim = SYS8(0x5B0);
    if (lim == SYS8(0x2A8)) {
        SYS8(0x2AA) = 0;
        ret = 1;
    }
    if ((int)(lim - 1) < (int)cur)
        SYS8(0x598) = lim - 1;
    return ret;
}
""", src="timebase")
r(0x4004245C, 20, "zoom_index_add", "SYS+0x598 += d, then 0x400423B8.", """
void func_400423b8(void);

void zoom_index_add(u32 d)
{
    SYS8(0x598) = SYS8(0x598) + d;
    func_400423b8();
}
""", src="timebase")
r(0x40042470, 72, "zoom_index_sub", "When SYS+0x614 < SYS16+0x2AC x SYS+0x554 and d < SYS+0x598: SYS+0x598 -= d. Then 0x400423B8.", """
void func_400423b8(void);

void zoom_index_sub(u32 d)
{
    u32 b;
    if (SYS32(0x614) < SYS16(0x2AC) * SYS32(0x554)) {
        b = SYS8(0x598);
        if (d < b)
            SYS8(0x598) = b - d;
    }
    func_400423b8();
}
""", src="timebase")
r(0x400424B8, 12, "zoom_index_set", "SYS+0x598 = v, then 0x400423B8.", """
void func_400423b8(void);

void zoom_index_set(u32 v)
{
    SYS8(0x598) = v;
    func_400423b8();
}
""", src="timebase")
r(0x400424D4, 76, "zoom_reset", "timebase_index_set(SYS+0x5B0), zoom_index_set(SYS+0x5B0 - 1), sys_word_pair_570_set(SYS64+0x5E0), copies SYS+0x570/574 to +0x578/57C, then sys_word_pair_578_set(0, 0) (so +0x578/57C end as 0).", """
void timebase_index_set(u32 v);
void zoom_index_set(u32 v);
void sys_word_pair_570_set(u32 lo, u32 hi);
void sys_word_pair_578_set(u32 lo, u32 hi);

void zoom_reset(void)
{
    u32 lo, hi;
    timebase_index_set(SYS8(0x5B0));
    zoom_index_set((SYS8(0x5B0) - 1) & 0xFF);
    sys_word_pair_570_set(SYS32(0x5E0), SYS32(0x5E4));
    lo = SYS32(0x570);
    hi = SYS32(0x574);
    SYS32(0x578) = lo;
    SYS32(0x57C) = hi;
    sys_word_pair_578_set(0, 0);
}
""", src="timebase")
