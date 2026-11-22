/*
 * ui_text_language_apply -- stock 0x400457B0 (236 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * For the language column L = SYS+0x7AC (1..4; anything else does nothing), every one of the 141 text entries gets +0 = pointer to its column L string (+4 + 0x64 x (L - 1)) and the two bytes +0x194/+0x195 = that column's pair at +0x196 + 2 x (L - 1). The language reading of SYS+0x7AC is DERIVED.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "ui/text.h"

#define TXT(k) (0x42C2FE74u + (k) * 0x1A0u)   /* 141 entries of 0x1A0 bytes */
#define TB(a)   (*(volatile unsigned char *)(a))

void ui_text_language_apply(void)
{
    u32 L = SYS8(0x7AC), k, e;
    if (L < 1 || L > 4)
        return;
    L -= 1;
    for (k = 0; k < 141; k++) {
        e = TXT(k);
        *(volatile u32 *)e = e + 4 + 0x64 * L;
        TB(e + 0x194) = TB(e + 0x196 + 2 * L);
        TB(e + 0x195) = TB(e + 0x197 + 2 * L);
    }
}
