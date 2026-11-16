/*
 * applied_state_invalidate -- stock 0x4003EF1C (144 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Fills the 'last applied' copy at SYS+0xBB0..0xBCA with the complement of the current values (bytes SYS+0x6E4..0x6E7, word +0x6E8, V/div bytes +0x41 / +0x105, byte +0x100) and clears +0xBB8..+0xBC7, so that everything is applied again; then 0x40063BC0 and outputs_apply.
 */
#include "hw/platform.h"

#include "hw/fpga.h"

void func_40063bc0(void);
void outputs_apply(void);

void applied_state_invalidate(void)
{
    SYS8(0xBB0) = ~SYS8(0x6E4);
    SYS8(0xBB1) = ~SYS8(0x6E5);
    SYS8(0xBB2) = ~SYS8(0x6E6);
    SYS8(0xBB3) = ~SYS8(0x6E7);
    SYS32(0xBB4) = ~SYS32(0x6E8);
    SYS32(0xBB8) = 0;
    SYS32(0xBBC) = 0;
    SYS32(0xBC0) = 0;
    SYS32(0xBC4) = 0;
    SYS8(0xBC9) = ~SYS8(0x105);
    SYS8(0xBC8) = ~SYS8(0x41);
    SYS8(0xBCA) = ~SYS8(0x100);
    func_40063bc0();
    outputs_apply();
}
