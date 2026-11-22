/*
 * full_acquisition_entry -- stock 0x400413C8 (1052 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Full acquisition. Mode byte SYS+0x27A: 1 clears SYS+0x101 and the halves SYS+0x584 / +0x586 (one word store); 2 (roll) moves both history buffers 0x42AC7CF4 and + 0x62250 up by the segment word SYS+0x2B0 samples (copy_u16 through 0x416431E8, 0x31128 - segment samples), grows the count SYS+0x586 up to SYS+0x582, steps the start SYS+0x584 (0 after SYS+0x582), refills the per-block tables (start / middle / length at SYS+0x2C4 / +0x38C / +0x454) and runs one block; other modes clear the halves. Then for each block i < SYS+0x550: acquisition_block_bank_selector((SYS+0x584 + i) & 0xFF), acquisition_prepare, acq_wait_done (stop when SYS+0x38 is clear), the bank copy steps, the block table entry, SYS+0x29C = 0, timebase_apply_2b4; for CH1 (SYS+0x3E) and CH2 (SYS+0x102) the stream geometry and SYS+0x644 chunks of SYS+0x640 samples (word SYS+0x63C advanced per chunk; mode SYS+0x100 picks the bank capture and the unpack / conversion into the buffers SYS+0x7B4 / +0x7B8); then the measurement chain for both channels and channel_to_display_copy(ch, i). While touch byte 0x42E68BB0 + 0x15 is set the loop ends early once (i + 1) x SYS+0x558 passes SYS+0x540 / SYS+0x1B0 / 10 (/ 3 unless a coupling byte SYS+0x51C / +0x51D is 2), setting the block counts SYS+0x550 / +0x5D0 to i + 1. At the end (roll mode: both counts = SYS+0x586) channel_filters_by_timebase, fft_input_prepare, and in mode 1 with SYS+0x38 set SYS+0x101 = SYS+0x3C = 1 and ui_windows_mark_dirty.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "acquisition.h"
#include "processing.h"
#include "ui/screens.h"
#include "ui/windows.h"
#include "util.h"

/* Same functions, declared the way the stock code calls them (see the definitions for the
 * full interface): the C must pass exactly these registers. */
void bank_unpack_0_3_n_raw(u32 dst, u32 ch, u32 n) __asm__("bank_unpack_0_3_n");
void first_conversion_wrapper_raw(u32 dst, u32 ch, u32 n) __asm__("first_conversion_wrapper");
void second_conversion_wrapper_raw(u32 dst, u32 ch, u32 n) __asm__("second_conversion_wrapper");

#define SYS 0x40544E68u
#define B8(a) (*(volatile unsigned char *)(a))
#define H16(a) (*(volatile unsigned short *)(a))
#define W32(a) (*(volatile u32 *)(a))
#define SEG W32(SYS + 0x2B0)
#define HIST 0x42AC7CF4u
#define TMP 0x416431E8u
#define ROLL_N 0x31128u

static void table(u32 i)
{
    u32 e = SYS + 0x2C4 + 4 * i, v;

    W32(e) = SEG * i;
    v = SEG;
    W32(e + 0x190) = v;
    W32(e + 0xC8) = W32(e) + (v >> 1);
}

static void roll_shift(u32 h)
{
    copy_u16((unsigned short *)TMP, (const unsigned short *)h, ROLL_N - SEG);
    copy_u16((unsigned short *)(h + 2 * SEG), (const unsigned short *)TMP, ROLL_N - SEG);
}

static void chunks(u32 ch, u32 base)
{
    u32 k, s;

    for (k = 0; (int)k < (int)H16(SYS + 0x644); k++) {
        s = k * H16(SYS + 0x640);
        W32(SYS + 0x63C) = s + base;
        if (B8(SYS + 0x100) == 0) {
            if (ch) {
                capture_banks_4_7();
                second_conversion_wrapper_raw(W32(SYS + 0x7B8) + 2 * s, 1, H16(SYS + 0x640));
            } else {
                capture_banks_0_3();
                bank_unpack_0_3_n_raw(W32(SYS + 0x7B4) + 2 * s, 0, H16(SYS + 0x640));
            }
        } else if (B8(SYS + 0x100) == ch + 1) {
            capture_banks_all();
            first_conversion_wrapper_raw(W32(ch ? SYS + 0x7B8 : SYS + 0x7B4) + 2 * s, ch, H16(SYS + 0x640));
        }
    }
}

void full_acquisition_entry(void)
{
    u32 i, j, base, lim, mode = B8(SYS + 0x27A);
    unsigned short c582, n;

    if (mode == 1) {
        B8(SYS + 0x101) = 0;
        W32(SYS + 0x584) = 0;
    } else if (mode == 2) {
        roll_shift(HIST);
        roll_shift(HIST + 0x62250u);
        c582 = H16(SYS + 0x582);
        H16(SYS + 0x586) = H16(SYS + 0x586) < c582 ? H16(SYS + 0x586) + 1 : c582;
        n = H16(SYS + 0x584);
        H16(SYS + 0x584) = c582 < n ? 0 : n + 1;
        for (j = 0; (int)j < (int)H16(SYS + 0x586); j++)
            table(j);
        H16(SYS + 0x550) = 1;
    } else
        W32(SYS + 0x584) = 0;

    for (i = 0; (int)i < (int)H16(SYS + 0x550); i++) {
        acquisition_block_bank_selector((H16(SYS + 0x584) + i) & 0xFF);
        acquisition_prepare();
        acq_wait_done();
        if (!B8(SYS + 0x38))
            break;
        veneer_40063798();
        consumer_translator_of_the_16_bit();
        base = W32(SYS + 0x63C);
        table(i);
        W32(SYS + 0x29C) = 0;
        timebase_apply_2b4();
        if (B8(SYS + 0x3E)) {
            stream_geometry_fpga_selector_0x22_configuration();
            chunks(0, base);
        }
        if (B8(SYS + 0x102)) {
            stream_geometry_fpga_selector_0x22_configuration();
            chunks(1, base);
        }
        channel_minmax_stats(0);
        channel_minmax_stats(1);
        channel_freq_measure(0);
        channel_freq_measure(1);
        moving_average_and_crossing_alignment_search();
        timebase_apply_2b0();
        channel_post_process(0);
        channel_post_process(1);
        channel_moving_average(0);
        channel_moving_average(1);
        channel_step_limiter(0);
        channel_step_limiter(1);
        trigger_window_crossing(0);
        trigger_window_crossing(1);
        timebase_apply_2b0();
        func_40032e0c_both();
        channel_to_display_copy(0, i & 0xFFFF);
        channel_to_display_copy(1, i & 0xFFFF);
        if (B8(0x42E68BB0u + 0x15)) {
            lim = W32(SYS + 0x540) / B8(SYS + 0x1B0);
            if (B8(SYS + 0x51C) == 2 || B8(SYS + 0x51D) == 2)
                lim /= 10;
            else
                lim /= 3;
            if (lim < (i + 1) * W32(SYS + 0x558)) {
                H16(SYS + 0x5D0) = (unsigned short)(i + 1);
                H16(SYS + 0x550) = (unsigned short)(i + 1);
                break;
            }
        }
    }
    if (B8(SYS + 0x27A) == 2) {
        n = H16(SYS + 0x586);
        H16(SYS + 0x550) = n;
        H16(SYS + 0x5D0) = n;
    }
    channel_filters_by_timebase();
    fft_input_prepare();
    if (B8(SYS + 0x27A) == 1 && B8(SYS + 0x38)) {
        B8(SYS + 0x101) = 1;
        B8(SYS + 0x3C) = 1;
        ui_windows_mark_dirty();
    }
}
