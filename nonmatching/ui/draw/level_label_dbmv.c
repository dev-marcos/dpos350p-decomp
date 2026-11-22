/*
 * level_label_dbmv -- stock 0x40038DEC (64 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * 0x40038C00(x, y, (u8)c, d0), then 0x4002CE7C((u16)(u16 0x42E68BE4 + x + 4), y, the text 'dBmV' at 0x4008FFC0).
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "ui/draw.h"

void level_label_dbmv(u32 x, u32 y, u32 c, double v)
{
    level_value_draw(x, y, c & 0xFF, v);
    text_draw((*(volatile unsigned short *)0x42E68BE4u + x + 4) & 0xFFFF, y, (const char *)0x4008FFC0u);
}
