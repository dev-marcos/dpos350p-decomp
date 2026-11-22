/*
 * ccu_gate_on -- stock 0x40042914 (100 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * CCU: set a bus-gating bit and a reset bit, both encoded in one word (gate reg = bits 31:21, gate bit = 20:16, reset reg = 15:5, reset bit = 4:0), then wait 100 us.
 */
#include "hw/drivers.h"
#include "drivers/ccu.h"
#include "drivers/mmio.h"
#include "drivers/timer.h"

#define CCU 0x01C20000u
#define PLL_LOCK 0x10000000u

void ccu_gate_on(s32 code)
{
    u32 g = CCU + (((u32)code >> 21) & 0x7FF), r = CCU + (((u32)code >> 5) & 0x7FF);
    writel_e(g, readl_e(g) | (1u << ((code >> 16) & 0x1F)));
    writel_e(r, readl_e(r) | (1u << (code & 0x1F)));
    delay_us(100);
}
