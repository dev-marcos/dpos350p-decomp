/*
 * consumer_translator_of_the_16_bit -- stock 0x4003FB90 (304 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Trigger delay in samples (SYS_INFO+0x63C): SYS+0x290 - SYS+0x2BC/2 - k (k as in trigger_offsets_update), raised by whole records L until >= 1; 1 when SYS+0x274 or +0x275 is 0. Then the trigger position to the FPGA and SYS+0x2A0 = SYS+0x2B4/2 + low-word product. (name: Hack FPGA.md)
 */
#include "hw/platform.h"

#include "hw/fpga.h"

void consumer_translator_of_the_16_bit(void)
{
    long long t, d;
    u32 L, h2, k2;
    if (SYS8(0x274) == 0 || SYS8(0x275) == 0) {
        SYS32(0x63C) = 1;
        return;
    }
    t = __aeabi_ldivmod((long long)SYS32(0x2BC), 2);
    d = (long long)SYS32(0x290) - t;
    t = __aeabi_ldivmod((long long)SYS64(0x570), (long long)SYS64(0x590));
    t = __aeabi_ldivmod(t, (long long)SYS32(0x564));
    t = (long long)((unsigned long long)t * SYS32(0x560));
    t = (long long)((unsigned long long)t * SYS32(0x558));
    t = __aeabi_ldivmod(t, (long long)SYS8(0x1B1));
    d = d - t;
    L = SYS32(0x520);
    while (d < 1)
        d += L;
    SYS32(0x63C) = (u32)d;
    trigger_pos_update();
    h2 = (u32)__aeabi_ldivmod((long long)SYS32(0x2B4), 2);
    t = __aeabi_ldivmod((long long)SYS64(0x570), (long long)SYS64(0x590));
    t = __aeabi_ldivmod(t, (long long)SYS32(0x564));
    k2 = (u32)t * SYS32(0x560);
    k2 = k2 * SYS32(0x558);
    SYS32(0x2A0) = h2 + k2;
}
