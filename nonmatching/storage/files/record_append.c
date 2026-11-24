/*
 * record_append -- stock 0x40036DE4 (192 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Stores the record being built into the first free slot of the record list at SYS+0xD744 (u16 SYS+0x758 slots of SI(capture_cells) cells; a slot is free when its id, the low bytes of cells 0 and 1, is 0). Cells 0..1 get the id u16 SYS+0xF9C (high byte, low byte), cells 2..3 the length word SYS+0x3DA53C (bits 8..15, bits 0..7), then copy_u16 copies that many cells from SYS+0x3DA548. With no free slot the stock code uses whatever the caller left in r2 as the cell index; here that is the third argument.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "storage/files.h"
#include "util.h"
#include "sys_info.h"
#include "shared/access.h"

#define SYS 0x40544E68u
#define RECS (SYS + 0xD744u)                /* u16 cells, one byte per cell */
#define RSIZE W32(SYS + 0x3DA544u)          /* record size in cells */
#define RCOUNT H16(SYS + 0x758)             /* number of records */

/* record id: low bytes of cells 0 and 1 */
static u32 rec_id(u32 p)
{
    return (H16(p) << 8 & 0xFF00) | B8(p + 2);
}

void record_append(u32 a0, u32 a1, u32 r2_in)
{
    int i = 0, lim = (int)(RSIZE * RCOUNT);
    u32 p = RECS, idx = r2_in, d;
    (void)a0;
    (void)a1;
    for (; i < lim; i += (int)RSIZE, p += 2 * RSIZE)
        if (rec_id(p) == 0) {
            idx = (u32)i;
            break;
        }
    d = RECS + 2 * idx;
    H16(d) = (unsigned short)(SI(f_f9c) >> 8 & 0xFF);
    H16(d + 2) = B8(SYS + 0xF9C);
    H16(d + 4) = (unsigned short)(SI(capture_len) >> 8 & 0xFF);
    H16(d + 6) = B8(SYS + 0x3DA53Cu);
    copy_u16((unsigned short *)(d + 8), (const unsigned short *)(SYS + 0x3DA548u), SI(capture_len));
}
