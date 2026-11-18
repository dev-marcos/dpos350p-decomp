/*
 * reload_chain_400373e0 -- stock 0x400373E0 (56 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * 0x40036B00, 0x40036BDC, 0x4003709C, 0x40036864, 0x40036DE4, 0x40036D88(u16 at 0x40545E04), 0x40036C54, 0x40036D10.
 */
#include "hw/platform.h"

#include "hw/fpga.h"

void list_c_index_load(void);
void list_c_data_load(void);
void list_c_next_free(void);
void saved_wave_trace_build(void);
void record_append(void);
void list_fa4_insert_758(u32 a);
void list_c_index_save(void);
void list_c_data_save(void);

void reload_chain_400373e0(void)
{
    list_c_index_load();
    list_c_data_load();
    list_c_next_free();
    saved_wave_trace_build();
    record_append();
    list_fa4_insert_758(*(volatile unsigned short *)0x40545E04u);
    list_c_index_save();
    list_c_data_save();
}
