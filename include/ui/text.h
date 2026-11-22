/* Functions of nonmatching/ui/text/ (one file per function, named after it).
 * Addresses are of build 2025-06-19. Names are DERIVED unless the C file says otherwise. */
#ifndef UI_TEXT_H
#define UI_TEXT_H

#include "rt/bits.h"

void ui_text_init(void);                                                                         /* 0x4004579C */
void ui_text_language_apply(void);                                                               /* 0x400457B0 */
void ui_text_set_lang1(u32 k, const unsigned char *s, u32 a, u32 b);                             /* 0x4004589C */
void ui_text_set_lang2(u32 k, const unsigned char *s, u32 a, u32 b);                             /* 0x400458CC */
void ui_text_set_lang3(u32 k, const unsigned char *s, u32 a, u32 b);                             /* 0x400458FC */
void ui_text_set_lang4(u32 k, const unsigned char *s, u32 a, u32 b);                             /* 0x4004592C */
void ui_texts_init(void);                                                                        /* 0x40045974 */

#endif
