# Settings records (field lists over the batch 15 serialiser, saved to / loaded from the SPI
# flash through the persistence layer) and the small SPI flash command helpers. exec'd by gen_platform.py


def g(addr, size, name, desc, body, src="settings"):
    f(addr, size, name, src, desc, '#include "hw/fpga.h"' + "\n\n" + body.strip("\n"))


SER = """void serializer_set_cursor(u32 c);
void serializer_set_encode_mode(void);
void serializer_set_decode_mode(void);
void serializer_process_field(volatile void *p, u32 size);
void persistence_read(u32 dev, u32 addr, volatile void *buf, u32 len);
void persistence_write_read_modify_write(u32 dev, u32 addr, volatile void *buf, u32 len);
#define SER_IMAGE ((volatile void *)0x408A526Cu)   /* SYS_INFO+0x360404 */
"""


def rec(letter, fields_fn, save_a, load_a, addr_off, length, what):
    g(save_a, 52, f"settings_{letter}_save",
      f"Saves settings record {letter.upper()} ({what}): encode the field list, then write 0x{length:X} bytes of the image to the flash address SYS_INFO+0x{addr_off:X}.", SER + f"""
void {fields_fn}(void);

void settings_{letter}_save(void)
{{
    serializer_set_encode_mode();
    {fields_fn}();
    persistence_write_read_modify_write(0, SYS32(0x{addr_off:X}), SER_IMAGE, 0x{length:X});
}}
""")
    if load_a:
        g(load_a, 12 if load_a == 0x40043C9C else 52, f"settings_{letter}_load",
          f"Loads settings record {letter.upper()}: read 0x{length:X} bytes from the flash address SYS_INFO+0x{addr_off:X} into the image, then decode the field list." + (" The stock entry is 12 bytes and falls through into 0x40043CA8." if load_a == 0x40043C9C else ""), SER + f"""
void {fields_fn}(void);

void settings_{letter}_load(void)
{{
    persistence_read(0, SYS32(0x{addr_off:X}), SER_IMAGE, 0x{length:X});
    serializer_set_decode_mode();
    {fields_fn}();
}}
""")


# ---------------------------------------------------------------- record A: 64 bytes at 0x405454EC
g(0x400439A0, 56, "settings_a_fields", "Field list of record A: the 64 bytes at 0x405454EC (SYS_INFO+0x684), one byte each.", SER + """
void settings_a_fields(void)
{
    volatile unsigned char *p = (volatile unsigned char *)0x405454ECu;
    int i;
    serializer_set_cursor(0);
    for (i = 0; i < 0x40; i++)
        serializer_process_field(p++, 1);
}
""")
rec("a", "settings_a_fields", 0x400439D8, 0x40043A0C, 0x7F4, 0x1000, "64 bytes at SYS_INFO+0x684")

# ---------------------------------------------------------------- record B: SYS+0x7AC, +0x658, +0
g(0x40043A40, 64, "settings_b_fields", "Field list of record B: SYS_INFO+0x7AC (1 byte), +0x658 (4), +0 (4).", SER + """
void settings_b_fields(void)
{
    serializer_set_cursor(0);
    serializer_process_field((volatile void *)(0x40544E68u + 0x7AC), 1);
    serializer_process_field((volatile void *)(0x40544E68u + 0x658), 4);
    serializer_process_field((volatile void *)0x40544E68u, 4);
}
""")
rec("b", "settings_b_fields", 0x40043A80, None, 0x7E4, 0x1000, "SYS_INFO+0x7AC, +0x658, +0")
g(0x40043AB4, 88, "settings_b_load",
  "Loads record B; when the loaded SYS_INFO+0 differs from the value before, sets SYS+0x7AC = 2 and runs 0x400447C8.", SER + """
void settings_b_fields(void);
void factory_reset_keep_language(void);

void settings_b_load(void)
{
    u32 old = SYS32(0);
    persistence_read(0, SYS32(0x7E4), SER_IMAGE, 0x1000);
    serializer_set_decode_mode();
    settings_b_fields();
    if (SYS32(0) != old) {
        SYS8(0x7AC) = 2;
        factory_reset_keep_language();
    }
}
""")

# ---------------------------------------------------------------- record C: list at 0x40043B0C (not rewritten)
rec("c", "record_c_fields", 0x40043C68, 0x40043C9C, 0x7F0, 0x4000, "field list 0x40043B0C")

# ---------------------------------------------------------------- record D: 0x42E554B0
g(0x40043CD0, 132, "settings_d_fields",
  "Field list of record D at 0x42E554B0: four words, then 300 entries of (word +0x10[i], double +0x4C0[i], double +0xE20[i]).", SER + """
void settings_d_fields(void)
{
    volatile unsigned char *b = (volatile unsigned char *)0x42E554B0u;
    volatile unsigned char *w = b + 0x10, *d = b + 0x4C0;
    int i;
    serializer_set_cursor(0);
    serializer_process_field(b, 4);
    serializer_process_field(b + 4, 4);
    serializer_process_field(b + 8, 4);
    serializer_process_field(b + 0xC, 4);
    for (i = 0; i < 300; i++) {
        serializer_process_field(w, 4);
        serializer_process_field(d, 8);
        serializer_process_field(d + 0x960, 8);
        d += 8;
        w += 4;
    }
}
""")
rec("d", "settings_d_fields", 0x40043D54, 0x40043D88, 0x7EC, 0x2000, "0x42E554B0, 300 entries")

# ---------------------------------------------------------------- record E: 0x42E68AE0 (with the magic)
g(0x40043DBC, 68, "settings_e_fields", "Field list of record E: the four words at 0x42E68AE0 (the first is the magic 0x5A5555A5).", SER + """
void settings_e_fields(void)
{
    volatile unsigned char *p = (volatile unsigned char *)0x42E68AE0u;
    serializer_set_cursor(0);
    serializer_process_field(p, 4);
    serializer_process_field(p + 4, 4);
    serializer_process_field(p + 8, 4);
    serializer_process_field(p + 0xC, 4);
}
""")
rec("e", "settings_e_fields", 0x40043E00, None, 0x7DC, 0x1000, "four words at 0x42E68AE0")
g(0x40043E34, 148, "settings_e_load",
  "Loads record E. Without the magic 0x5A5555A5 (first start or erased flash) it rebuilds the defaults: 0x4003EFAC, then settings_slot_save for slots 1..5 (SYS_INFO+0x7C0), slot 1 saved in record F, SYS+0x7AC = 2 and record B saved.", SER + """
void settings_e_fields(void);
void settings_f_save(void);
void settings_b_save(void);
void factory_defaults(void);
void settings_slot_save(void);

void settings_e_load(void)
{
    u32 i;
    persistence_read(0, SYS32(0x7DC), SER_IMAGE, 0x1000);
    serializer_set_decode_mode();
    settings_e_fields();
    if (*(volatile u32 *)0x42E68AE0u == 0x5A5555A5u)
        return;
    factory_defaults();
    for (i = 1; i <= 5; i++) {
        SYS8(0x7C0) = i;
        settings_slot_save();
    }
    SYS8(0x7C0) = 1;
    settings_f_save();
    SYS8(0x7AC) = 2;
    settings_b_save();
}
""")

# ---------------------------------------------------------------- record F: 1 byte at 0x40545628
g(0x40043EC8, 36, "settings_f_fields", "Field list of record F: the byte at 0x40545628 (SYS_INFO+0x7C0, the current slot).", SER + """
void settings_f_fields(void)
{
    serializer_set_cursor(0);
    serializer_process_field((volatile void *)0x40545628u, 1);
}
""")
rec("f", "settings_f_fields", 0x40043EEC, 0x40043F20, 0x7C4, 0x1000, "the slot byte SYS_INFO+0x7C0")

# ---------------------------------------------------------------- SPI flash helpers
SPI = """void sys_spi_select(u32 dev);
void sys_spi_deselect(u32 dev);
int  sys_spi_write_then_read(u32 dev, const volatile unsigned char *tx, u32 ntx, volatile unsigned char *rx, u32 nrx);
"""
g(0x4004341C, 80, "spi0_pins_init", "When mode is 0: PC0..PC3 to function 3 (SPI0), drive level 3.", """
void spi0_pins_init(u32 mode)
{
    u32 pin;
    if (mode != 0)
        return;
    for (pin = 0; pin < 4; pin++) {
        pio_cfg_pin((void *)0x01C20848u, pin, 3, 0);
        pio_set_drive((void *)0x01C20848u, pin, 3);
    }
}
""", src="spi")
g(0x4004346C, 8, "spi0_clock_on", "CCU gate for SPI0 (ccu_gate_on(0x0C145814)).", """
void ccu_gate_on(u32 id);

void spi0_clock_on(void)
{
    ccu_gate_on(0x0C145814u);
}
""", src="spi")
g(0x40043474, 32, "spi0_init", "SPI0 pins, clock gate, then sys_spi_init(mode).", """
void spi0_pins_init(u32 mode);
void spi0_clock_on(void);
void sys_spi_init(u32 mode);

void spi0_init(u32 mode)
{
    spi0_pins_init(mode);
    spi0_clock_on();
    sys_spi_init(mode);
}
""", src="spi")
g(0x40043494, 96, "spi_flash_cmd_read1", "Sends a one-byte command and reads one byte back (chip select around it).", SPI + """
u32 spi_flash_cmd_read1(u32 dev, u32 cmd)
{
    volatile unsigned char c = cmd, rx;
    sys_spi_select(dev);
    sys_spi_write_then_read(dev, &c, 1, &rx, 1);
    sys_spi_deselect(dev);
    return rx;
}
""", src="spi")
g(0x400434F4, 32, "spi_flash_wait_ready", "Polls the flash status register (command 0x05) until the busy bit is clear.", """
u32 spi_flash_cmd_read1(u32 dev, u32 cmd);

void spi_flash_wait_ready(u32 dev)
{
    while (spi_flash_cmd_read1(dev, 5) & 1)
        ;
}
""", src="spi")
g(0x40043514, 80, "spi_flash_write_enable", "Write enable (command 0x06); returns 0 when the transfer reports 1 byte, else -1.", SPI + """
int spi_flash_write_enable(u32 dev)
{
    volatile unsigned char c = 6;
    int r;
    sys_spi_select(dev);
    r = sys_spi_transfer(dev, (const unsigned char *)&c, 0, 1) == 1 ? 0 : -1;
    sys_spi_deselect(dev);
    return r;
}
""", src="spi")
