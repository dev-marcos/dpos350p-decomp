/*
 * reload_chain_40035744 -- stock 0x40035744 (60 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * 0x400358B8, 0x400363CC, SYS+0xF90 = 0, u16 SYS+0xFA4 = 0, u16 SYS+0xF9E = 0, then 0x400362F4, 0x4003656C, 0x400365F4.
 */
#include "hw/platform.h"

#include "hw/fpga.h"

void func_400358b8(void);
void func_400363cc(void);
void list_cursor_clamp_400362f4(void);
void func_4003656c(void);
void func_400365f4(void);

void reload_chain_40035744(void)
{
    func_400358b8();
    func_400363cc();
    SYS8(0xF90) = 0;
    SYS16(0xFA4) = 0;
    SYS16(0xF9E) = 0;
    list_cursor_clamp_400362f4();
    func_4003656c();
    func_400365f4();
}
