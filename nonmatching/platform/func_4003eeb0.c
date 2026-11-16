/*
 * state_blocks_init -- stock 0x4003EEB0 (108 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * 0x4003EB40, the three block initialisers, 0x40030060(0, k) for k = 7, 6, 3, 4, 5, 2, then state_backup_all.
 */
#include "hw/platform.h"

#include "hw/fpga.h"

void func_4003eb40(void);
void state_block_3f984_init(void);
void state_block_180c8_init(void);
void state_block_68668_init(void);
void func_40030060(u32 a, u32 k);
void state_backup_all(void);

void state_blocks_init(void)
{
    func_4003eb40();
    state_block_3f984_init();
    state_block_180c8_init();
    state_block_68668_init();
    func_40030060(0, 7);
    func_40030060(0, 6);
    func_40030060(0, 3);
    func_40030060(0, 4);
    func_40030060(0, 5);
    func_40030060(0, 2);
    state_backup_all();
}
