# Start-up pieces: LCD pins and display bring-up, factory defaults and the state-block
# initialisers, the "previous state" cache invalidation, the acquisition geometry set-up, and two
# more SPI flash helpers (sector erase, page-split write). exec'd by gen_platform.py


def s(addr, size, name, desc, body, src="init"):
    f(addr, size, name, src, desc, '#include "hw/fpga.h"' + "\n\n" + body.strip("\n"))


LOCK = "(*(volatile unsigned char *)0x42E68BC7u)"

# ---------------------------------------------------------------- display bring-up
s(0x4003E63C, 76, "lcd_pins_init", "PD1..PD21 except PD12 to function 2 (LCD RGB / control), no pull.", """
void lcd_pins_init(void)
{
    u32 pin;
    for (pin = 1; pin < 22; pin++) {
        if (pin == 12)
            continue;
        pio_cfg_pin((void *)0x01C2086Cu, pin, 2, 0);
    }
}
""", src="display")
s(0x4003E688, 44, "lcd_start", "TCON and DE clocks off, LCD pins, then display_init(1022, 600, 0x41FB8FE8).", """
void tcon_clock_off(void);
void de_clock_off(void);
void lcd_pins_init(void);
void display_init(u32 width, u32 height, u32 fb);

void lcd_start(void)
{
    tcon_clock_off();
    de_clock_off();
    lcd_pins_init();
    display_init(0x3FE, 0x258, 0x41FB8FE8u);
}
""", src="display")

# ---------------------------------------------------------------- state initialisers
s(0x4003E708, 48, "outputs_apply", "Unless the byte 0x42E68BC7 is set: 0x4003A654, frontend_apply_all, 0x40063328.", """
void acquisition_apply(void);
void frontend_apply_all(void);
void fpga_config_push(void);

void outputs_apply(void)
{
    if (""" + LOCK + """)
        return;
    acquisition_apply();
    frontend_apply_all();
    fpga_config_push();
}
""")
s(0x4003ED28, 88, "state_block_3f984_init", "Defaults of the block at 0x42E3F984 (+1 = 0, +4 = pointer to +8, +8 = 0, +0xC = 0, +0x2EFA = 1, +0x2EFC/+0x2F00 = 1000, +0x2F04 = 0, +0x2F08 = 500, +0x2F60 = 0) and the byte 0x42E68BC6 = 1.", """
#define B(o) (*(volatile unsigned char *)(0x42E3F984u + (o)))
#define W(o) (*(volatile u32 *)(0x42E3F984u + (o)))

void state_block_3f984_init(void)
{
    B(1) = 0;
    W(4) = 0x42E3F984u + 8;
    B(0x2EFA) = 1;
    W(0x2EFC) = 1000;
    W(0x2F00) = 1000;
    W(8) = 0;
    W(0x2F04) = 0;
    B(0xC) = 0;
    B(0x2F60) = 0;
    W(0x2F08) = 500;
    *(volatile unsigned char *)0x42E68BC6u = 1;
}
""")
s(0x4003ED80, 116, "state_block_180c8_init", "Defaults of the block at 0x42E180C8 (+0x4C/+0x54 = 1000, +0x50 = 0, +0x58 = 50000000, +0x5C = 20, bytes +0xAC60 and +0xAD18 = 0, +0xAC64 = h / 5 and +0xAC68 = 4h / 5 with h = the halfword +4) and the byte 0x42E68BC6 = 1.", """
#define B(o) (*(volatile unsigned char *)(0x42E180C8u + (o)))
#define W(o) (*(volatile u32 *)(0x42E180C8u + (o)))

void state_block_180c8_init(void)
{
    u32 h;
    W(0x5C) = 20;
    h = *(volatile unsigned short *)(0x42E180C8u + 4);
    W(0x58) = 50000000;
    W(0x54) = 1000;
    W(0x4C) = 1000;
    B(0xAC60) = 0;
    W(0x50) = 0;
    B(0xAC60 + 0xB8) = 0;
    W(0xAC64) = h / 5u;
    W(0xAC68) = (u32)((int)(h * 4) / 5);
    *(volatile unsigned char *)0x42E68BC6u = 1;
}
""")
s(0x4003EDF4, 188, "state_block_68668_init", "Defaults of the block at 0x42E68668: +0xF4 / +0x188 = 2, +0x140 = 20.0, +0x148 = 60.0, the 64-bit values +0x120 = 200000, +0x128 = 500000000, +0x130 = 250100000, +0x138 = (+0x128) - 200000, +0x5C = (+0xB8) / 5 and +0x60 = 4 (+0xB8) / 5 (signed), several fields cleared; the byte 0x42E68BC6 = 1.", """
#define B(o) (*(volatile unsigned char *)(0x42E68668u + (o)))
#define H(o) (*(volatile unsigned short *)(0x42E68668u + (o)))
#define W(o) (*(volatile u32 *)(0x42E68668u + (o)))
#define D(o) (*(volatile double *)(0x42E68668u + (o)))

void state_block_68668_init(void)
{
    int v;
    W(0x188) = 2;
    W(0xF4) = 2;
    H(0) = 0;
    B(2) = 0;
    B(0x190) = 0;
    D(0x140) = 20.0;
    D(0x148) = 60.0;
    W(0x120) = 200000;
    W(0x124) = 0;
    W(0x128) = 500000000;
    W(0x12C) = 0;
    W(0x130) = 250100000;
    W(0x134) = 0;
    W(0x13C) = 0;
    W(0x138) = W(0x128) - 200000;
    v = (int)W(0xB8);
    W(0x18C) = 0;
    B(4) = 0;
    W(0x58) = 0;
    W(0x5C) = v / 5;
    W(0x60) = (int)((u32)v * 4u) / 5;
    *(volatile unsigned char *)0x42E68BC6u = 1;
}
""")
s(0x4003EEB0, 108, "state_blocks_init", "0x4003EB40, the three block initialisers, 0x40030060(0, k) for k = 7, 6, 3, 4, 5, 2, then state_backup_all.", """
void acq_defaults_reset(void);
void state_block_3f984_init(void);
void state_block_180c8_init(void);
void state_block_68668_init(void);
void marker_value_add(u32 a, u32 k);
void state_backup_all(void);

void state_blocks_init(void)
{
    acq_defaults_reset();
    state_block_3f984_init();
    state_block_180c8_init();
    state_block_68668_init();
    marker_value_add(0, 7);
    marker_value_add(0, 6);
    marker_value_add(0, 3);
    marker_value_add(0, 4);
    marker_value_add(0, 5);
    marker_value_add(0, 2);
    state_backup_all();
}
""")
s(0x4003EF1C, 144, "applied_state_invalidate", "Fills the 'last applied' copy at SYS+0xBB0..0xBCA with the complement of the current values (bytes SYS+0x6E4..0x6E7, word +0x6E8, V/div bytes +0x41 / +0x105, byte +0x100) and clears +0xBB8..+0xBC7, so that everything is applied again; then 0x40063BC0 and outputs_apply.", """
void fpga_counters_init(void);
void outputs_apply(void);

void applied_state_invalidate(void)
{
    SYS8(0xBB0) = ~SYS8(0x6E4);
    SYS8(0xBB1) = ~SYS8(0x6E5);
    SYS8(0xBB2) = ~SYS8(0x6E6);
    SYS8(0xBB3) = ~SYS8(0x6E7);
    SYS32(0xBB4) = ~SYS32(0x6E8);
    SYS32(0xBB8) = 0;
    SYS32(0xBBC) = 0;
    SYS32(0xBC0) = 0;
    SYS32(0xBC4) = 0;
    SYS8(0xBC9) = ~SYS8(0x105);
    SYS8(0xBC8) = ~SYS8(0x41);
    SYS8(0xBCA) = ~SYS8(0x100);
    fpga_counters_init();
    outputs_apply();
}
""")
s(0x4003EFAC, 120, "factory_defaults", "Clears SYS_INFO (0x3DD588 bytes), the block 0x42E3F984 (0x2F70), the window table (0x578), the window stack slots (0x14), 0x42E68BB0 (0x18) and 0x42E68A88 (0x58); then 0x4003E738 and state_blocks_init.", """
void system_defaults_init(void);
void state_blocks_init(void);

void factory_defaults(void)
{
    memset_w((void *)0x40544E68u, 0, 0x3DD588);
    memset_w((void *)0x42E3F984u, 0, 0x2F70);
    memset_w((void *)0x42E64520u, 0, 0x578);
    memset_w((void *)0x42E68BCAu, 0, 0x14);
    memset_w((void *)0x42E68BB0u, 0, 0x18);
    memset_w((void *)0x42E68A88u, 0, 0x58);
    system_defaults_init();
    state_blocks_init();
}
""")
s(0x4003C008, 164, "acquisition_geometry_setup",
  "Sets the capture geometry: SYS+0x38 = 1; step SYS16+0x6F8 = 2 at time-base index 0x21, else 1; SYS16+0x6F6 = SYS+0x558 x step; per-bank count SYS16+0x6F4 = SYS16+0x6F6 / SYS+0x1B1 + 2 (copied to +0x640 and +0x6F0); SYS+0x708 = SYS16+0x6F6 + 1; SYS+0x454 = SYS+0x2B0, SYS+0x38C = SYS+0x2B0 / 2; SYS+0x568 / +0x56C = -SYS16+0xC; clears SYS+0x6FC, +0x700, +0x2C4, +0x63C. The stock division by zero gives 0; the C does the same explicitly.", """
void acquisition_geometry_setup(void)
{
    u32 d, q, prod, L;
    int b;
    SYS8(0x38) = 1;
    d = SYS8(0x1B1);
    SYS16(0x6F8) = SYS8(0x588) == 0x21 ? 2 : 1;
    prod = (SYS32(0x558) * SYS16(0x6F8)) & 0xFFFF;
    SYS16(0x6F6) = prod;
    b = (int)d;
    q = b ? (u32)((int)prod / b) : 0;
    SYS16(0x6F4) = q + 2;
    SYS32(0x6FC) = 0;
    SYS32(0x700) = 0;
    SYS16(0x640) = SYS16(0x6F4);
    SYS16(0x6F0) = SYS16(0x6F4);
    L = SYS32(0x2B0);
    SYS32(0x2C4) = 0;
    SYS32(0x63C) = 0;
    SYS32(0x708) = SYS16(0x6F6) + 1;
    SYS32(0x454) = L;
    SYS32(0x38C) = L >> 1;
    SYS32(0x568) = -(u32)SYS16(0xC);
    SYS32(0x56C) = -(u32)SYS16(0xC);
}
""", src="acquisition")

# ---------------------------------------------------------------- SPI flash: erase, page-split write, record C tail
SPI = """void sys_spi_select(u32 dev);
void sys_spi_deselect(u32 dev);
int spi_flash_write_enable(u32 dev);
void spi_flash_wait_ready(u32 dev);
"""
s(0x40043564, 188, "spi_flash_sector_erase",
  "Erases the 4 KB sector at addr: write enable, wait, then command 0x20 + 3 address bytes, or 0x21 + 4 address bytes when the word 0x42E68CD4 (4-byte addressing) is set; waits again. Returns 0, or -1 when the write enable failed or the transfer did not send every byte.", SPI + """
int spi_flash_sector_erase(u32 dev, u32 addr)
{
    volatile unsigned char cmd[8];
    int ret = 0, n;
    if (*(volatile u32 *)0x42E68CD4u == 0) {
        cmd[0] = 0x20;
        cmd[1] = addr >> 16;
        cmd[2] = addr >> 8;
        cmd[3] = addr;
        n = 4;
    } else {
        cmd[0] = 0x21;
        cmd[1] = addr >> 24;
        cmd[2] = addr >> 16;
        cmd[3] = addr >> 8;
        cmd[4] = addr;
        n = 5;
    }
    if (spi_flash_write_enable(dev) < 0)
        ret = -1;
    spi_flash_wait_ready(dev);
    sys_spi_select(dev);
    if (sys_spi_transfer(dev, (const unsigned char *)cmd, 0, n) != n)
        ret = -1;
    sys_spi_deselect(dev);
    spi_flash_wait_ready(dev);
    return ret;
}
""", src="spi")
s(0x4004380C, 104, "spi_flash_write_paged",
  "Writes len bytes from buf to the flash at addr through the page program 0x4004372C, split so no chunk crosses a 256-byte page. Loop exit is on chunk == (u16) remaining length, as in the stock code.", """
void spi_flash_page_program(u32 dev, u32 addr, u32 buf, u32 n);

void spi_flash_write_paged(u32 dev, u32 addr, u32 buf, u32 len)
{
    int n = (int)len, room = 0x100 - (int)(addr & 0xFF);
    if (n > room)
        n = room;
    for (;;) {
        spi_flash_page_program(dev, addr, buf, (u32)n);
        if ((u32)n == (len & 0xFFFF))
            return;
        buf += n;
        addr += n;
        len -= n;
        n = (int)(len & 0xFFFF);
        if (n > 0x100)
            n = 0x100;
    }
}
""", src="spi")
s(0x40043CA8, 40, "settings_c_load_tail",
  "Second half of settings_c_load (the 12-byte entry falls through into it with p = SYS_INFO, off = 0x360404, len = 0x4000): read len bytes from the flash address p+0x7F0 into p + off, then decode the field list 0x40043B0C.", """
void persistence_read(u32 dev, u32 addr, volatile void *buf, u32 len);
void serializer_set_decode_mode(void);
void record_c_fields(void);

void settings_c_load_tail(volatile unsigned char *p, u32 off, u32 unused, u32 len)
{
    (void)unused;
    persistence_read(0, *(volatile u32 *)(p + 0x7F0), p + off, len);
    serializer_set_decode_mode();
    record_c_fields();
}
""", src="settings")
