/*
 * ui_text_init -- stock 0x4004579C (20 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Fills the text table (0x40045974, 13.5 KB of str_copy calls), then falls into ui_text_language_apply.
 */
#include "hw/platform.h"

#include "hw/fpga.h"

void ui_texts_init(void);
void ui_text_language_apply(void);

void ui_text_init(void)
{
    ui_texts_init();
    ui_text_language_apply();
}
