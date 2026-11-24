/* The UI text table at 0x42C2FE74 (build 2025-06-19), see docs/DRIVERS.md "UI text table".
 *
 * 141 entries of 0x1A0 bytes. +0 points to the current string; four 0x64-byte string columns sit at
 * +4, +0x68, +0xCC and +0x130 (one per language, DERIVED); +0x194 / +0x195 are the x / y offsets
 * the current language draws its text with, and +0x196.. hold one pair per column.
 * ui_text_language_apply copies the selected column's pointer and pair into +0 and +0x194.
 *
 * The macros take an entry address e (UI_TEXT(n), or a u32 holding one), as the stock code does. */
#ifndef UI_TEXTS_H
#define UI_TEXTS_H

#define UI_TEXT_BASE 0x42C2FE74u
#define UI_TEXT(n)   (UI_TEXT_BASE + 0x1A0 * (n))                       /* address of entry n */
#define TEXT_STR(e)  ((const char *)(*(volatile u32 *)(e)))              /* current string */
#define TEXT_DX(e)   (*(volatile signed char *)((e) + 0x194))           /* x offset of the text */
#define TEXT_DY(e)   (*(volatile signed char *)((e) + 0x195))           /* y offset of the text */

#endif
