/*
 * reload_chain_400378e0 -- stock 0x400378E0 (56 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * 0x40036B00, 0x40036BDC, 0x400377F8, 0x4003771C, then SYS+0xF90 = 0, u16 SYS+0xFA4 = 0, u16 SYS+0xF9E = 0.
 */
#include "hw/platform.h"

#include "hw/fpga.h"

void func_40036b00(void);
void func_40036bdc(void);
void func_400377f8(void);
void list_cursor_clamp_4003771c(void);

void reload_chain_400378e0(void)
{
    func_40036b00();
    func_40036bdc();
    func_400377f8();
    list_cursor_clamp_4003771c();
    SYS8(0xF90) = 0;
    SYS16(0xFA4) = 0;
    SYS16(0xF9E) = 0;
}
