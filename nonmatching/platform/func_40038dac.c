/*
 * level_label_db -- stock 0x40038DAC (64 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * 0x40038C00(x, y, (u8)c, d0), then 0x4002CE7C((u16)(u16 0x42E68BE4 + x + 4), y, the text 'dB' at 0x40038E3C).
 */
#include "hw/platform.h"

#include "hw/fpga.h"

void func_40038c00(u32 x, u32 y, u32 c, double v);
void func_4002ce7c(u32 x, u32 y, const char *s);

void level_label_db(u32 x, u32 y, u32 c, double v)
{
    func_40038c00(x, y, c & 0xFF, v);
    func_4002ce7c((*(volatile unsigned short *)0x42E68BE4u + x + 4) & 0xFFFF, y, (const char *)0x40038E3Cu);
}
