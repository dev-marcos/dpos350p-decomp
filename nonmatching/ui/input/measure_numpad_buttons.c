/*
 * measure_numpad_buttons -- stock 0x40056530 (500 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Measurement window numpad fields, buttons 0xB92..0xBC4: each clears byte 0x42E68A8A and opens the numpad (mode 3 at x = 2 / 0x92, mode 1 at 0x124, 0x1B6, 0x248, 0x28B; y = 0xE9) with the callback 0x400617AC, flag 0x42E68AD4 = 1 and a target in block 0x42E68668: the doubles +0x148 / +0x140 (word 0x42E68AB0), or the s64 +0x120, +0x128, +0x130, +0x138 (word 0x42E68AB4, with mode +0x18C = 1..4). Button 0xBCE toggles byte +3. Except for the first two, the four target words are always rewritten.
 */
#include "hw/platform.h"

#include "hw/fpga.h"

#define TB(o) (*(volatile unsigned char *)(0x42E61FF8u + (o)))
void ui_window_close(u32 id);
void ui_window_open(u32 id);
void ui_window_invalidate_from(u32 id, u32 r1);
void inv_left(u32 id) __asm__("ui_window_invalidate_from");

void touch_group_0B92(void);
void numpad_open_mode1(u32 x, u32 y);
void numpad_open_mode3(u32 x, u32 y);
#define NP8(o) (*(volatile unsigned char *)(0x42E68A88u + (o)))
#define NP32(o) (*(volatile u32 *)(0x42E68A88u + (o)))
#define MB8(o) (*(volatile unsigned char *)(0x42E68668u + (o)))
#define MB32(o) (*(volatile u32 *)(0x42E68668u + (o)))
#define CB 0x400617ACu

void measure_numpad_buttons(void)
{
    u32 fl, cb, tgt, mode;
    touch_group_0B92();
    if (TB(0xB92) == 2) {
        TB(0xB92) = 0;
        TB(0) = 0;
        NP8(2) = 0;
        numpad_open_mode3(2, 0xE9);
        NP32(0x28) = 0x42E68668u + 0x148;
        NP32(0x50) = CB;
        NP8(0x4C) = 1;
        return;
    }
    fl = NP8(0x4C);
    cb = NP32(0x50);
    if (TB(0xB9C) == 2) {
        TB(0xB9C) = 0;
        TB(0) = 0;
        NP8(2) = 0;
        numpad_open_mode3(0x92, 0xE9);
        NP32(0x28) = 0x42E68668u + 0x140;
        NP32(0x50) = CB;
        NP8(0x4C) = 1;
        return;
    }
    if (TB(0xBA6) == 2) {
        TB(0xBA6) = 0;
        TB(0) = 0;
        NP8(2) = 0;
        numpad_open_mode1(0x124, 0xE9);
        tgt = 0x42E68668u + 0x120, fl = 1, cb = CB, mode = 1;
    } else {
        tgt = NP32(0x2C);
        mode = MB32(0x18C);
        if (TB(0xBB0) == 2) {
            TB(0xBB0) = 0;
            TB(0) = 0;
            NP8(2) = 0;
            numpad_open_mode1(0x1B6, 0xE9);
            tgt = 0x42E68668u + 0x128, fl = 1, cb = CB, mode = 2;
        } else if (TB(0xBBA) == 2) {
            TB(0xBBA) = 0;
            TB(0) = 0;
            NP8(2) = 0;
            numpad_open_mode1(0x248, 0xE9);
            tgt = 0x42E68668u + 0x130, fl = 1, cb = CB, mode = 3;
        } else if (TB(0xBC4) == 2) {
            TB(0xBC4) = 0;
            TB(0) = 0;
            NP8(2) = 0;
            numpad_open_mode1(0x28B, 0xE9);
            tgt = 0x42E68668u + 0x138, fl = 1, cb = CB, mode = 4;
        } else if (TB(0xBCE) == 2) {
            TB(0xBCE) = 0;
            TB(0) = 0;
            MB8(3) = MB8(3) == 0;
        }
    }
    NP32(0x2C) = tgt;
    MB32(0x18C) = mode;
    NP32(0x50) = cb;
    NP8(0x4C) = fl;
}
