/*
 * reload_chain_400373e0 -- stock 0x400373E0 (56 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * 0x40036B00, 0x40036BDC, 0x4003709C, 0x40036864, 0x40036DE4, 0x40036D88(u16 at 0x40545E04), 0x40036C54, 0x40036D10.
 */
#include "hw/platform.h"

#include "hw/fpga.h"

void func_40036b00(void);
void list_c_data_load(void);
void func_4003709c(void);
void func_40036864(void);
void func_40036de4(void);
void list_fa4_insert_758(u32 a);
void list_c_index_save(void);
void list_c_data_save(void);

void reload_chain_400373e0(void)
{
    func_40036b00();
    list_c_data_load();
    func_4003709c();
    func_40036864();
    func_40036de4();
    list_fa4_insert_758(*(volatile unsigned short *)0x40545E04u);
    list_c_index_save();
    list_c_data_save();
}
