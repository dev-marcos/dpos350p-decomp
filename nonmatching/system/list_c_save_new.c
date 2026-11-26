/*
 * list_c_save_new -- stock 0x400373E0 (56 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * 0x40036B00, 0x40036BDC, 0x4003709C, 0x40036864, 0x40036DE4, 0x40036D88(u16 at 0x40545E04), 0x40036C54, 0x40036D10.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "storage/files.h"
#include "system.h"

/* Same functions, declared the way the stock code calls them (see the definitions for the
 * full interface): the C must pass exactly these registers. */
void record_append_raw(void) __asm__("record_append");

void list_c_save_new(void)
{
    list_c_index_load();
    list_c_data_load();
    list_c_next_free();
    saved_wave_trace_build();
    record_append_raw();
    list_c_insert(*(volatile unsigned short *)0x40545E04u);
    list_c_index_save();
    list_c_data_save();
}
