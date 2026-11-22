/*
 * ccu_gate_off -- stock 0x40042978 (108 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * CCU: clear the gating and reset bits encoded as in ccu_gate_on, then wait 100 us.
 */
#include "hw/drivers.h"
#include "drivers/ccu.h"
#include "drivers/mmio.h"
#include "drivers/timer.h"

#define CCU 0x01C20000u
#define PLL_LOCK 0x10000000u

void ccu_gate_off(s32 code)
{
    u32 g = CCU + (((u32)code >> 21) & 0x7FF), r = CCU + (((u32)code >> 5) & 0x7FF);
    writel_e(g, ~(1u << ((code >> 16) & 0x1F)) & readl_e(g));
    writel_e(r, ~(1u << (code & 0x1F)) & readl_e(r));
    delay_us(100);
}
