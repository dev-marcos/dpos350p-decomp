/*
 * ui_text_set_lang3 -- stock 0x400458FC (48 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Text entry k, language column 3: str_copy(entry + 0xCC, s), then the bytes +0x19A / +0x19B = a / b.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "ui/text.h"
#include "util.h"

#define TXT(k) (0x42C2FE74u + (k) * 0x1A0u)   /* 141 entries of 0x1A0 bytes */
#define TB(a)   (*(volatile unsigned char *)(a))

void ui_text_set_lang3(u32 k, const unsigned char *s, u32 a, u32 b)
{
    u32 e = TXT(k);
    str_copy((unsigned char *)(e + 0xCC), s);
    TB(e + 0x19A) = a;
    TB(e + 0x19B) = b;
}
