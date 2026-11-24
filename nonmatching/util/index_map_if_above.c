/*
 * index_map_if_above -- stock 0x4005C918 (60 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * When SYS+0x554 < m: simple_integer_index_mapping_nearest_neighbour(src_a, center_a, w, m, shift_a,
 * unused) (the last two are the stack words, passed on).
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "processing.h"
#include "util.h"
#include "sys_info.h"

void index_map_if_above(u32 src_a, u32 center_a, u32 w, u32 m, u32 shift_a, u32 unused)
{
    if (SI(f_554) < m)
        simple_integer_index_mapping_nearest_neighbour(src_a, center_a, w, m, shift_a, unused);
}
