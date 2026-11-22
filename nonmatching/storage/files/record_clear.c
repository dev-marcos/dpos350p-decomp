/*
 * record_clear -- stock 0x40037020 (116 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Finds the record with id `id` in the record list at SYS+0xD744 and calls copy_u16(slot, 0, record size), which copies the record size in cells from address 0 over it (the stock code passes 0 as the source, probably meant as a fill). With no such record the stock code uses whatever the caller left in r3 as the cell index; here that is the fourth argument.
 */
#include "hw/platform.h"

#include "hw/fpga.h"

void copy_u16(unsigned short *d, const unsigned short *s, u32 n);
#define SYS 0x40544E68u
#define H16(a) (*(volatile unsigned short *)(a))
#define W32(a) (*(volatile u32 *)(a))
#define B8(a) (*(volatile unsigned char *)(a))
#define RECS (SYS + 0xD744u)                /* u16 cells, one byte per cell */
#define RSIZE W32(SYS + 0x3DA544u)          /* record size in cells */
#define RCOUNT H16(SYS + 0x758)             /* number of records */

/* record id: low bytes of cells 0 and 1 */
static u32 rec_id(u32 p)
{
    return (H16(p) << 8 & 0xFF00) | B8(p + 2);
}

void record_clear(u32 id, u32 a1, u32 a2, u32 r3_in)
{
    int i = 0, lim = (int)(RSIZE * RCOUNT);
    u32 p = RECS, idx = r3_in, n = RSIZE;
    (void)a1;
    (void)a2;
    for (; i < lim; i += (int)n, p += 2 * n)
        if (rec_id(p) == id) {
            idx = (u32)i;
            break;
        }
    copy_u16((unsigned short *)(RECS + 2 * idx), (const unsigned short *)0, n);
}
