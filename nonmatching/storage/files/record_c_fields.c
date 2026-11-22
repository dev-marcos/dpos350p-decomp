/*
 * record_c_fields -- stock 0x40043B0C (348 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * The field list of settings record C, the FFT spur lists at 0x42E3C000: cursor 0, the u16 counts +6 / +4 / +2 / +0, the float means +0x14 / +0x10 / +0xC / +8, then 200 / 400 / 650 / 1200 pairs (float value, u16 bin) from +0x2340 / +0x1D00 / +0x12D8 / +0x18 and +0x37F4 / +0x34D4 / +0x2FC0 / +0x2660.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "storage/files.h"
#include "storage/settings.h"

#define SYS 0x40544E68u
#define B8(a) (*(volatile unsigned char *)(a))
#define H16(a) (*(volatile unsigned short *)(a))
#define W32(a) (*(volatile u32 *)(a))

#define SPUR 0x42E3C000u

static void pairs(u32 vals, u32 idxs, u32 n)
{
    u32 i;
    for (i = 0; i < n; i++) {
        serializer_process_field((volatile void *)(SPUR + vals + 4 * i), 4);
        serializer_process_field((volatile void *)(SPUR + idxs + 2 * i), 2);
    }
}

void record_c_fields(void)
{
    serializer_set_cursor(0);
    serializer_process_field((volatile void *)(SPUR + 6), 2);
    serializer_process_field((volatile void *)(SPUR + 4), 2);
    serializer_process_field((volatile void *)(SPUR + 2), 2);
    serializer_process_field((volatile void *)SPUR, 2);
    serializer_process_field((volatile void *)(SPUR + 0x14), 4);
    serializer_process_field((volatile void *)(SPUR + 0x10), 4);
    serializer_process_field((volatile void *)(SPUR + 0xC), 4);
    serializer_process_field((volatile void *)(SPUR + 8), 4);
    pairs(0x2340, 0x37F4, 200);
    pairs(0x1D00, 0x34D4, 400);
    pairs(0x12D8, 0x2FC0, 650);
    pairs(0x18, 0x2660, 1200);
}
