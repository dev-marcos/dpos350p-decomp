/*
 * fpga_params_6e4_sync -- stock 0x4003F160 (404 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Pushes the FPGA parameters SYS+0x6E4..+0x6E8 that differ from their cached copies SYS+0xBB0..+0xBB4, updating the copy first: bytes +0x6E4 / +0x6E5 / +0x6E6 each go through fpga_param_updater(0x21 / 0x22 / 0x23, 1, the unaligned word at the copy, the byte), smec_commit(0xC1 / 0xC2 / 0xC3) and the FPGA selector 4 / 5 / 6 with the byte; byte +0x6E7 only through selector 2; the word +0x6E8 through selector 0xD as four bytes, high first. When anything was sent: selector 7 with 1, then 0 (a strobe), then delay_ms(2). The word change alone sends the strobe too.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "drivers/fpga_bus.h"
#include "drivers/timer.h"
#include "fpga.h"
#include "sys_info.h"
#include "shared/access.h"

#define SYS 0x40544E68u

typedef u32 u32_unaligned __attribute__((aligned(1)));
#define WU(a) (*(volatile u32_unaligned *)(a))

void fpga_params_6e4_sync(void)
{
    int sent = 0, i;
    u32 v;
    for (i = 0; i < 3; i++) {
        v = B8(SYS + 0x6E4 + i);
        if (B8(SYS + 0xBB0 + i) != v) {
            B8(SYS + 0xBB0 + i) = (unsigned char)v;
            sent = 1;
            fpga_param_updater(0x21 + i, 1, WU(SYS + 0xBB0 + i), v);
            smec_commit(0xC1 + i);
            fpga_selector_command_write(4 + i);
            fpga_data_byte_write(B8(SYS + 0xBB0 + i));
        }
    }
    v = SI(f_6e7);
    if (SI(f_bb3) != v) {
        SI(f_bb3) = (unsigned char)v;
        fpga_selector_command_write(2);
        sent = 1;
        fpga_data_byte_write(SI(f_bb3));
    }
    v = SI(f_6e8);
    if (SI(f_bb4) != v) {
        SI(f_bb4) = v;
        fpga_selector_command_write(0xD);
        fpga_data_byte_write(SI(f_bb4) >> 24);
        fpga_data_byte_write((SI(f_bb4) << 8) >> 24);
        fpga_data_byte_write((SI(f_bb4) << 16) >> 24);
        fpga_data_byte_write(SI(f_bb4) & 0xFF);
    } else if (!sent)
        return;
    fpga_selector_command_write(7);
    fpga_data_byte_write(1);
    fpga_selector_command_write(7);
    fpga_data_byte_write(0);
    delay_ms(2);
}
