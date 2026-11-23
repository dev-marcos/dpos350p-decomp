/*
 * capture_with_length -- stock 0x400406A4 (36 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * SYS_INFO+0x63C = 20, SYS16+0x640 = len + 20, then capture_block(v, buf).
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "acquisition.h"
#include "sys_info.h"

void capture_with_length(u32 v, u32 len, u32 buf)
{
    SI(runtime_quotient) = 0x14;
    SI(f_640) = len + 0x14;
    capture_block(v, buf);
}
