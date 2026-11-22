/*
 * spi_flash_write_paged -- stock 0x4004380C (104 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Writes len bytes from buf to the flash at addr through the page program 0x4004372C, split so no chunk crosses a 256-byte page. Loop exit is on chunk == (u16) remaining length, as in the stock code.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "drivers/spi.h"

/* Same functions, declared the way the stock code calls them (see the definitions for the
 * full interface): the C must pass exactly these registers. */
void spi_flash_page_program_raw(u32 dev, u32 addr, u32 buf, u32 n) __asm__("spi_flash_page_program");

void spi_flash_write_paged(u32 dev, u32 addr, u32 buf, u32 len)
{
    int n = (int)len, room = 0x100 - (int)(addr & 0xFF);
    if (n > room)
        n = room;
    for (;;) {
        spi_flash_page_program_raw(dev, addr, buf, (u32)n);
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
