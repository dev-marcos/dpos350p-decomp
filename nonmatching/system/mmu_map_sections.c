/*
 * mmu_map_sections -- stock 0x400429E4 (124 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Writes size >> 20 first-level section descriptors (1 MB each) into the table from entry va >> 20: physical section pa >> 20 upwards, flags 0xC12 (AP = 3, section) with C/B from attr bits 0..1.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "system.h"

void mmu_map_sections(volatile u32 *tbl, unsigned long long va, unsigned long long pa, unsigned long long size,
                      unsigned char attr)
{
    unsigned long long n = size >> 20;
    u32 s = (u32)(pa >> 20), cb = (attr & 3) << 2;
    volatile u32 *p = tbl + (u32)(va >> 20);
    while (n) {
        *p++ = 0xC12 | (s << 20) | cb;
        s++;
        n--;
    }
}
