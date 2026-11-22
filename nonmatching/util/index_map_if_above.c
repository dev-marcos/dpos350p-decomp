/*
 * index_map_if_above -- stock 0x4005C918 (60 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * When SYS+0x554 < d: simple_integer_index_mapping_nearest_neighbour(a, b, c, d, e, g) (e, g = the two stack words passed on).
 */
#include "hw/platform.h"

#include "hw/fpga.h"

void index_map_if_above(u32 a, u32 b, u32 c, u32 d, u32 e, u32 g)
{
    if (SYS32(0x554) < d)
        simple_integer_index_mapping_nearest_neighbour(a, b, c, d, e, g);
}
