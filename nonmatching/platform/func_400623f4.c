/*
 * levels_update_chain -- stock 0x400623F4 (76 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Unless touch byte +0x17 is set: when byte +4 of block 0x42E68668 is set runs 0x40061DB0 and 0x40061A70; then 0x4006053C, level_stats_5f010, level_stats_60008 and 0x4006233C.
 */
#include "hw/platform.h"

#include "hw/fpga.h"

void func_40061db0(void);
void func_40061a70(void);
void func_4006053c(void);
void level_stats_5f010(void);
void level_stats_60008(void);
void func_4006233c(void);

void levels_update_chain(void)
{
    if (*(volatile unsigned char *)(0x42E68BB0u + 0x17))
        return;
    if (*(volatile unsigned char *)(0x42E68668u + 4)) {
        func_40061db0();
        func_40061a70();
    }
    func_4006053c();
    level_stats_5f010();
    level_stats_60008();
    func_4006233c();
}
