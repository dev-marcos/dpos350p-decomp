/*
 * trigger_offsets_update -- stock 0x4003FAE8 (168 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Trigger offsets: k = (SYS64+0x570 / SYS64+0x590 / SYS+0x564) x SYS+0x560 x SYS+0x558 / SYS+0x1B1 (signed 64-bit divisions); a = SYS+0x2BC / 2 + k; SYS+0x288 = a mod L, SYS+0x28C = (SYS+0x2BC - a) mod L; then 0x400636BC, 0x40063714.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "acquisition.h"
#include "sys_info.h"

void trigger_offsets_update(void)
{
    u32 r4 = SI(f_2bc), L, a, b;
    long long t;
    u32 half = (u32)__aeabi_ldivmod((long long)r4, 2);
    t = __aeabi_ldivmod((long long)SYS64(0x570), (long long)SI(timebase_period));
    t = __aeabi_ldivmod(t, (long long)SI(f_564));
    t = (long long)((unsigned long long)t * SI(f_560));
    t = (long long)((unsigned long long)t * SI(bank_count));
    t = __aeabi_ldivmod(t, (long long)SI(repeat_factor));
    L = SI(record_length);
    a = (u32)t + half;
    b = r4 - a;
    SI(trigger_offset_1b) = a - (a / L) * L;
    SI(trigger_offset_1c) = b - (b / L) * L;
    fpga_cmd1b_write_veneer();
    fpga_cmd1c_write_veneer();
}
