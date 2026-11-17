/*
 * numpad_buttons_b06 -- stock 0x40056080 (396 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Buttons 0xB06..0xB2E open the numpad for field 1..5 (modes 4, 4, 1, 1, 5 at x = 0, 0xAB, 0x156, 0x201, 0x236, y = 0xE9) after clearing byte 0x42E68A8A, and set the numpad target: word 0x42E68AB8 = 0x42E18114 + 4k, byte 0x42E68AD4 = 0, word 0x42E68AD8 = 0, word 0x42E18110 = field. Button 0xB38 toggles byte 0x42E1810E. The four target words are always rewritten.
 */
#include "hw/platform.h"

#include "hw/fpga.h"

#define TB(o) (*(volatile unsigned char *)(0x42E61FF8u + (o)))
void ui_window_close(u32 id);
void ui_window_open(u32 id);

void touch_group_0B06(void);
void numpad_open_mode1(u32 x, u32 y);
void numpad_open_mode4(u32 x, u32 y);
void numpad_open_mode5(u32 x, u32 y);
#define TT8(o) (*(volatile unsigned char *)(0x42E180C8u + (o)))
#define TT32(o) (*(volatile u32 *)(0x42E180C8u + (o)))
#define NP8(o) (*(volatile unsigned char *)(0x42E68A88u + (o)))
#define NP32(o) (*(volatile u32 *)(0x42E68A88u + (o)))

void numpad_buttons_b06(void)
{
    u32 ptr, b, w, fld;
    touch_group_0B06();
    if (TB(0xB06) == 2) {
        TB(0xB06) = 0;
        TB(0) = 0;
        NP8(2) = 0;
        numpad_open_mode4(0, 0xE9);
        ptr = 0x42E180C8u + 0x4C, b = 0, fld = 1, w = 0;
    } else if (TB(0xB10) == 2) {
        TB(0xB10) = 0;
        TB(0) = 0;
        NP8(2) = 0;
        numpad_open_mode4(0xAB, 0xE9);
        ptr = 0x42E180C8u + 0x50, b = 0, fld = 2, w = 0;
    } else {
        ptr = NP32(0x30);
        b = NP8(0x4C);
        w = NP32(0x50);
        fld = TT32(0x48);
        if (TB(0xB1A) == 2) {
            TB(0xB1A) = 0;
            TB(0) = 0;
            NP8(2) = 0;
            numpad_open_mode1(0x156, 0xE9);
            ptr = 0x42E180C8u + 0x54, b = 0, fld = 3, w = 0;
        } else if (TB(0xB24) == 2) {
            TB(0xB24) = 0;
            TB(0) = 0;
            NP8(2) = 0;
            numpad_open_mode1(0x201, 0xE9);
            ptr = 0x42E180C8u + 0x58, b = 0, fld = 4, w = 0;
        } else if (TB(0xB2E) == 2) {
            TB(0xB2E) = 0;
            TB(0) = 0;
            NP8(2) = 0;
            numpad_open_mode5(0x236, 0xE9);
            ptr = 0x42E180C8u + 0x5C, b = 0, fld = 5, w = 0;
        } else if (TB(0xB38) == 2) {
            TB(0xB38) = 0;
            TB(0) = 0;
            TT8(0x46) = TT8(0x46) == 0;
        }
    }
    TT32(0x48) = fld;
    NP32(0x50) = w;
    NP8(0x4C) = b;
    NP32(0x30) = ptr;
}
