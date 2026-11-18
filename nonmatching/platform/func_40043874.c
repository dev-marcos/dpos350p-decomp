/*
 * persistence_write_read_modify_write -- stock 0x40043874 (260 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Writes len bytes from buf to the SPI flash at addr, one 4 KB sector at a time. Each sector is first read into the buffer 0x42E5E010. When the target bytes are all 0xFF they are programmed directly; otherwise the sector is erased, the new bytes are merged into the buffer and the whole sector is programmed back.
 */
#include "hw/platform.h"

#include "hw/fpga.h"

int persistence_read(u32 dev, u32 addr, volatile unsigned char *buf, u32 len);
int spi_flash_sector_erase(u32 dev, u32 addr);
void spi_flash_write_paged(u32 dev, u32 addr, u32 buf, u32 len);

#define SECTOR_BUF ((volatile unsigned char *)0x42E5E010u)

void persistence_write_read_modify_write(u32 dev, u32 addr, const volatile unsigned char *buf, u32 len)
{
    u32 off = addr & 0xFFF, sector = addr >> 12;
    int chunk = 0x1000 - (int)off, i;
    if ((u32)chunk >= len)
        chunk = (unsigned short)len;
    for (;;) {
        persistence_read(dev, sector << 12, SECTOR_BUF, 0x1000);
        for (i = 0; i < chunk; i++)
            if (SECTOR_BUF[off + i] != 0xFF)
                break;
        if (i < chunk) {
            spi_flash_sector_erase(dev, sector << 12);
            for (i = 0; i < chunk; i++)
                SECTOR_BUF[off + i] = buf[i];
            spi_flash_write_paged(dev, sector << 12, (u32)SECTOR_BUF, 0x1000);
        } else {
            spi_flash_write_paged(dev, addr, (u32)buf, (unsigned short)chunk);
        }
        if (len == (u32)chunk)
            return;
        len -= chunk;
        buf += chunk;
        addr += chunk;
        sector++;
        off = 0;
        chunk = len < 0x1001 ? (unsigned short)len : 0x1000;
    }
}
