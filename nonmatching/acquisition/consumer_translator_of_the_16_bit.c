/*
 * consumer_translator_of_the_16_bit -- stock 0x4003FB90 (304 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Trigger delay in samples (SYS_INFO+0x63C): SYS+0x290 - SYS+0x2BC/2 - k (k as in trigger_offsets_update), raised by whole records L until >= 1; 1 when SYS+0x274 or +0x275 is 0. Then the trigger position to the FPGA and SYS+0x2A0 = SYS+0x2B4/2 + low-word product. (name: Hack FPGA.md)
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "acquisition.h"
#include "sys_info.h"

void consumer_translator_of_the_16_bit(void)
{
    long long t, d;
    u32 L, h2, k2;
    if (SI(f_274) == 0 || SI(f_275) == 0) {
        SI(runtime_quotient) = 1;
        return;
    }
    t = __aeabi_ldivmod((long long)SI(f_2bc), 2);
    d = (long long)SI(f_290) - t;
    t = __aeabi_ldivmod((long long)SYS64(0x570), (long long)SI(f_590));
    t = __aeabi_ldivmod(t, (long long)SI(f_564));
    t = (long long)((unsigned long long)t * SI(f_560));
    t = (long long)((unsigned long long)t * SI(f_558));
    t = __aeabi_ldivmod(t, (long long)SI(repeat_factor));
    d = d - t;
    L = SI(f_520);
    while (d < 1)
        d += L;
    SI(runtime_quotient) = (u32)d;
    trigger_pos_update();
    h2 = (u32)__aeabi_ldivmod((long long)SI(f_2b4), 2);
    t = __aeabi_ldivmod((long long)SYS64(0x570), (long long)SI(f_590));
    t = __aeabi_ldivmod(t, (long long)SI(f_564));
    k2 = (u32)t * SI(f_560);
    k2 = k2 * SI(f_558);
    SI(f_2a0) = h2 + k2;
}
