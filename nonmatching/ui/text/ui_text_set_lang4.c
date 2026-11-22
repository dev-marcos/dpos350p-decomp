/*
 * ui_text_set_lang4 -- stock 0x4004592C (48 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Text entry k, language column 4: str_copy(entry + 0x130, s), then the bytes +0x19C / +0x19D = a / b.
 */
#include "hw/platform.h"

#include "hw/fpga.h"

#define TXT(k) (0x42C2FE74u + (k) * 0x1A0u)   /* 141 entries of 0x1A0 bytes */
#define TB(a)   (*(volatile unsigned char *)(a))
unsigned char *str_copy(unsigned char *d, const unsigned char *s);

void ui_text_set_lang4(u32 k, const unsigned char *s, u32 a, u32 b)
{
    u32 e = TXT(k);
    str_copy((unsigned char *)(e + 0x130), s);
    TB(e + 0x19C) = a;
    TB(e + 0x19D) = b;
}
