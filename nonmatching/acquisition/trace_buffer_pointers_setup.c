/*
 * trace_buffer_pointers_setup -- stock 0x4003FFE4 (32 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * SYS+0x7BC = n; with a = (s16) SYS+0x29C, b = (s16) SYS+0x29E: SYS+0x7B4 = 0x42C62AF4 + 2a + 2b + 2 x ((0xC738 - n) >> 1) and SYS+0x7B8 = that + 2 x 0xC738. (The symbol table splits this function at 0x40040004; the entry here covers both pieces.)
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "acquisition.h"
#include "sys_info.h"

void trace_buffer_pointers_setup(u32 n)
{
    u32 a = (u32)(int)(short)SI(f_29c), b = (u32)(int)(short)SI(f_29e);
    u32 p;
    SI(capture_samples) = n;
    p = 0x42C62AF4u + 2u * a + 2u * b + 2u * ((0xC738u - n) >> 1);
    SI(ch1_buffer) = p;
    SI(ch2_buffer) = p + 2u * 0xC738u;
}
