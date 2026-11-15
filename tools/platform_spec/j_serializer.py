# Field serialiser (big-endian settings image at SYS_INFO+0x360404), FatFs code-page helpers
# (CP437), strlen, the USB mass-storage disk veneers, touch drag helpers and USB mode switches.
# exec'd by gen_platform.py


def s(addr, size, name, desc, body, src="serializer"):
    f(addr, size, name, src, desc, '#include "hw/fpga.h"' + "\n\n" + body.strip("\n"))


SER = """#define SER_CURSOR (*(volatile u32 *)0x4091F38Cu)            /* SYS_INFO+0x3DA524 */
#define SER_MODE   (*(volatile unsigned char *)0x4091F390u)  /* 0 = encode, 1 = decode */
#define SER_IMAGE  ((volatile unsigned char *)0x408A526Cu)   /* SYS_INFO+0x360404 */
"""

# ---------------------------------------------------------------- serialiser
s(0x40067134, 16, "serializer_set_encode_mode", "Serialiser mode byte 0x4091F390 = 0 (encode). (name: Hack)", SER + """
void serializer_set_encode_mode(void)
{
    SER_MODE = 0;
}
""", src="serializer; name: Hack")
s(0x40067144, 16, "serializer_set_decode_mode", "Serialiser mode byte 0x4091F390 = 1 (decode). (name: Hack)", SER + """
void serializer_set_decode_mode(void)
{
    SER_MODE = 1;
}
""", src="serializer; name: Hack")
s(0x40067154, 12, "serializer_get_cursor", "Returns the serialiser cursor 0x4091F38C.", SER + """
u32 serializer_get_cursor(void)
{
    return SER_CURSOR;
}
""")
s(0x40067160, 280, "serializer_encode_scalar_be",
  "Stores a 1/2/4/8-byte value big-endian at image+off (image = SYS_INFO+0x360404); other sizes do nothing. The source is re-read for every byte, as in the stock code. (name: Hack)", SER + """
void serializer_encode_scalar_be(u32 off, const volatile void *src, u32 size)
{
    volatile unsigned char *d = SER_IMAGE + off;
    if (size == 1) {
        d[0] = *(const volatile unsigned char *)src;
    } else if (size == 2) {
        const volatile unsigned short *p = src;
        d[0] = *p >> 8;
        d[1] = *p;
    } else if (size == 4) {
        const volatile u32 *p = src;
        d[0] = *p >> 24;
        d[1] = *p >> 16;
        d[2] = *p >> 8;
        d[3] = *p;
    } else if (size == 8) {
        const volatile unsigned long long *p = src;
        d[0] = *p >> 56;
        d[1] = *p >> 48;
        d[2] = *p >> 40;
        d[3] = *p >> 32;
        d[4] = *p >> 24;
        d[5] = *p >> 16;
        d[6] = *p >> 8;
        d[7] = *p;
    }
}
""", src="serializer; name: Hack")
s(0x400672A8, 376, "serializer_decode_scalar_be",
  "Loads a 1/2/4/8-byte big-endian value from image+off into *dst (the stock code stores each partial result). (name: Hack)", SER + """
void serializer_decode_scalar_be(u32 off, volatile void *dst, u32 size)
{
    volatile unsigned char *b = SER_IMAGE + off;
    if (size == 1) {
        *(volatile unsigned char *)dst = b[0];
    } else if (size == 2) {
        volatile unsigned short *p = dst;
        u32 v = (u32)b[0] << 8;
        *p = v;
        *p = v | b[1];
    } else if (size == 4) {
        volatile u32 *p = dst;
        u32 v = (u32)b[0] << 8;
        *p = v;
        v = (v | b[1]) << 8;
        *p = v;
        v = (v | b[2]) << 8;
        *p = v;
        *p = v | b[3];
    } else if (size == 8) {
        volatile unsigned long long *p = dst;
        unsigned long long v = (unsigned long long)b[0] << 8;
        int i;
        *p = v;
        for (i = 1; i < 7; i++) {
            v = (v | b[i]) << 8;
            *p = v;
        }
        *p = v | b[7];
    }
}
""", src="serializer; name: Hack")
s(0x40067278, 48, "serializer_encode_field", "Encodes size bytes from src at the cursor, then advances the cursor. (name: Hack)", SER + """
void serializer_encode_scalar_be(u32 off, const volatile void *src, u32 size);

void serializer_encode_field(const volatile void *src, u32 size)
{
    serializer_encode_scalar_be(SER_CURSOR, src, size);
    SER_CURSOR = SER_CURSOR + size;
}
""", src="serializer; name: Hack")
s(0x40067420, 48, "serializer_decode_field", "Decodes size bytes at the cursor into dst, then advances the cursor. (name: Hack)", SER + """
void serializer_decode_scalar_be(u32 off, volatile void *dst, u32 size);

void serializer_decode_field(volatile void *dst, u32 size)
{
    serializer_decode_scalar_be(SER_CURSOR, dst, size);
    SER_CURSOR = SER_CURSOR + size;
}
""", src="serializer; name: Hack")
s(0x40067450, 20, "serializer_process_field", "Encodes or decodes one field, by the mode byte. (name: Hack)", SER + """
void serializer_encode_field(const volatile void *src, u32 size);
void serializer_decode_field(volatile void *dst, u32 size);

void serializer_process_field(volatile void *p, u32 size)
{
    if (SER_MODE == 0)
        serializer_encode_field(p, size);
    else
        serializer_decode_field(p, size);
}
""", src="serializer; name: Hack")

# ---------------------------------------------------------------- FatFs code page (CP437 only), strlen
CP = """#define CP437_HI   ((const volatile unsigned short *)0x4008C50Cu)   /* Unicode of OEM 0x80..0xFF */
"""
s(0x40069D90, 100, "ff_uni2oem",
  "Unicode to OEM code (FatFs ffunicode.c, CP437 only): ASCII passes, 0x80..0xFF is looked up, anything else or another code page gives 0. An unmatched character also gives 0 ((0x80 + 0x80) & 0xFF). (name: Hack)", CP + """
u32 ff_uni2oem(u32 uni, u32 cp)
{
    u32 c;
    if (uni < 0x80)
        return uni & 0xFFFF;
    if (uni >= 0x100 || cp != 437)
        return 0;
    for (c = 0; c < 0x80; c++)
        if (CP437_HI[c] == uni)
            break;
    return (c + 0x80) & 0xFF;
}
""", src="fatfs; name: Hack")
s(0x40069DF4, 68, "ff_oem2uni",
  "OEM code to Unicode (CP437 only). Codes below 0x80, compared signed, are returned unchanged and without truncation. (name: Hack)", CP + """
u32 ff_oem2uni(int oem, u32 cp)
{
    if (oem < 0x80)
        return oem;
    if (cp == 437 && oem < 0x100)
        return CP437_HI[oem - 0x80];
    return 0;
}
""", src="fatfs; name: Hack")
s(0x40069E38, 272, "ff_wtoupper",
  "Unicode upper-case conversion (FatFs R0.13b ff_wtoupper, tables at 0x400877AC and 0x4008C6E0). Values of 0x10000 and above pass unchanged. (name: Hack)", """
u32 ff_wtoupper(u32 uni)
{
    const volatile unsigned short *p;
    u32 uc, bc, nc, cmd;
    if (uni >= 0x10000)
        return uni;
    uc = uni & 0xFFFF;
    p = uc < 0x1000 ? (const volatile unsigned short *)0x400877ACu : (const volatile unsigned short *)0x4008C6E0u;
    for (;;) {
        bc = *p++;
        if (bc == 0 || uc < bc)
            break;
        nc = *p++;
        cmd = nc >> 8;
        nc &= 0xFF;
        if (uc < bc + nc) {
            switch (cmd) {
            case 0: uc = p[uc - bc]; break;
            case 1: uc -= (uc - bc) & 1; break;
            case 2: uc -= 16; break;
            case 3: uc -= 32; break;
            case 4: uc -= 48; break;
            case 5: uc -= 26; break;
            case 6: uc += 8; break;
            case 7: uc -= 80; break;
            case 8: uc -= 0x1C60; break;
            }
            break;
        }
        if (cmd == 0)
            p += nc;
    }
    return uc & 0xFFFF;
}
""", src="fatfs; name: Hack")
s(0x4006A6A8, 84, "strlen", "Length of a NUL-terminated string (the stock code scans a word at a time once aligned). (name: Hack)", """
u32 strlen(const char *str)
{
    const volatile char *p = str;
    while (*p)
        p++;
    return (u32)(p - str);
}
""", src="libc; name: Hack")

# ---------------------------------------------------------------- USB disk veneers, mode switches
s(0x4006A964, 20, "usb_mass_disk_read",
  "Reads through 0x4006A738 when the SD card state (0x40095490) is 1; otherwise returns its first argument unchanged. Stock quirks kept: the callee also receives r3 = 0x40095490. (name: Hack)", """
u32 func_4006a738(u32 a, u32 b, u32 c, u32 d);

u32 usb_mass_disk_read(u32 a, u32 b, u32 c)
{
    if (*(volatile u32 *)0x40095490u == 1)
        return func_4006a738(a, b, c, 0x40095490u);   /* r3 still holds the state address */
    return a;
}
""", src="usb; name: Hack")
s(0x4006A978, 20, "usb_mass_disk_write",
  "Writes through 0x4006A868 when the SD card state is 1; otherwise returns its first argument unchanged. (name: Hack)", """
u32 func_4006a868(u32 a, u32 b, u32 c, u32 d);

u32 usb_mass_disk_write(u32 a, u32 b, u32 c)
{
    if (*(volatile u32 *)0x40095490u == 1)
        return func_4006a868(a, b, c, 0x40095490u);   /* r3 still holds the state address */
    return a;
}
""", src="usb; name: Hack")
s(0x40069018, 28, "usb_mode_enter", "Updates the USB disk geometry, then usb_device_init(4).", """
void usb_disk_geometry_update(void);
void usb_device_init(u32 mode);

void usb_mode_enter(void)
{
    usb_disk_geometry_update();
    usb_device_init(4);
}
""", src="usb")
s(0x40069034, 24, "usb_mode_exit", "usb_device_exit, then disk_remount.", """
void usb_device_exit(void);
void disk_remount(void);

void usb_mode_exit(void)
{
    usb_device_exit();
    disk_remount();
}
""", src="usb")
s(0x40068E74, 28, "lradc_data_read", "LRADC data register 0x01C2280C, low 6 bits (V3s LRADC block at 0x01C22800; read by the battery service).", """
u32 readl_c(u32 addr);

u32 lradc_data_read(void)
{
    return readl_c(0x01C2280Cu) & 0x3F;
}
""", src="platform")

# ---------------------------------------------------------------- touch drag helpers
s(0x40069CF8, 96, "touch_drag_delta",
  "Drag distance from the touch block: dx = x1 - x0, dy = y1 - y0 (signed 16-bit fields), divided by 10 when SYS_INFO+0x671 is set; stored at 0x42E68998+0x30 / +0x34.", """
void touch_drag_delta(void)
{
    volatile short *t = (volatile short *)0x42E68BB0u;
    int dx = t[2] - t[0], dy = t[3] - t[1];
    if (SYS8(0x671)) {
        dx /= 10;
        dy /= 10;
    }
    *(volatile int *)(0x42E68998u + 0x34) = dy;
    *(volatile int *)(0x42E68998u + 0x30) = dx;
}
""", src="touch")
s(0x4006904C, 56, "touch_moved_flag", "0x42E68998 byte = 0x12 when the touch block's second point is not (0, 0), else 0.", """
void touch_moved_flag(void)
{
    volatile short *t = (volatile short *)0x42E68BB0u;
    if (t[2] == 0 && t[3] == 0)
        *(volatile unsigned char *)0x42E68998u = 0;
    else
        *(volatile unsigned char *)0x42E68998u = 0x12;
}
""", src="touch")
