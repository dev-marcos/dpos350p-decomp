/*
 * fpga_params_6e4_sync -- stock 0x4003F160 (404 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Pushes the FPGA parameters SYS+0x6E4..+0x6E8 that differ from their cached copies SYS+0xBB0..+0xBB4, updating the copy first: bytes +0x6E4 / +0x6E5 / +0x6E6 each go through fpga_param_updater(0x21 / 0x22 / 0x23, 1, the unaligned word at the copy, the byte), smec_commit(0xC1 / 0xC2 / 0xC3) and the FPGA selector 4 / 5 / 6 with the byte; byte +0x6E7 only through selector 2; the word +0x6E8 through selector 0xD as four bytes, high first. When anything was sent: selector 7 with 1, then 0 (a strobe), then delay_ms(2). The word change alone sends the strobe too.
 */
#include "hw/platform.h"

#include "hw/fpga.h"

#define SYS 0x40544E68u
#define GEN 0x42E3F984u
#define TOUCH 0x42E68BB0u
#define B8(a) (*(volatile unsigned char *)(a))
#define H16(a) (*(volatile unsigned short *)(a))
#define W32(a) (*(volatile u32 *)(a))
#define D64(a) (*(volatile unsigned long long *)(a))

void smec_commit(u32 id);
void delay_ms(u32 ms);
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
    v = B8(SYS + 0x6E7);
    if (B8(SYS + 0xBB3) != v) {
        B8(SYS + 0xBB3) = (unsigned char)v;
        fpga_selector_command_write(2);
        sent = 1;
        fpga_data_byte_write(B8(SYS + 0xBB3));
    }
    v = W32(SYS + 0x6E8);
    if (W32(SYS + 0xBB4) != v) {
        W32(SYS + 0xBB4) = v;
        fpga_selector_command_write(0xD);
        fpga_data_byte_write(W32(SYS + 0xBB4) >> 24);
        fpga_data_byte_write((W32(SYS + 0xBB4) << 8) >> 24);
        fpga_data_byte_write((W32(SYS + 0xBB4) << 16) >> 24);
        fpga_data_byte_write(W32(SYS + 0xBB4) & 0xFF);
    } else if (!sent)
        return;
    fpga_selector_command_write(7);
    fpga_data_byte_write(1);
    fpga_selector_command_write(7);
    fpga_data_byte_write(0);
    delay_ms(2);
}
