/* The colour table at SYS_INFO + 0x3DD530 (build 2025-06-19): 22 words that
 * theme_colours_init fills. The names are DERIVED from the screens that use them
 * (docs/DRIVERS.md: toolbars, menus, option buttons); THEME_WORD(o) reaches the others. */
#ifndef THEME_H
#define THEME_H

#define THEME_WORD(o) (*(volatile u32 *)(0x40544E68u + 0x3DD530u + (o)))

#define THEME_TOOLBAR_FILL     THEME_WORD(0x00)   /* toolbar background */
#define THEME_TOOLBAR_ICON     THEME_WORD(0x04)   /* toolbar icons */
#define THEME_TOOLBAR_TEXT     THEME_WORD(0x08)   /* toolbar labels */
#define THEME_PRESSED          THEME_WORD(0x1C)   /* a pressed button (SI theme_pressed) */
#define THEME_MENU_BG          THEME_WORD(0x20)   /* menu background (SI theme_background) */
#define THEME_MENU_TEXT        THEME_WORD(0x24)   /* menu text (SI theme_text) */
#define THEME_MENU_ROW_PRESSED THEME_WORD(0x28)   /* a pressed menu row */
#define THEME_OPTION           THEME_WORD(0x2C)   /* an option button */
#define THEME_OPTION_SELECTED  THEME_WORD(0x30)   /* the selected option (drawn with black text) */
#define THEME_OPTION_TEXT      THEME_WORD(0x34)   /* text of an option button */

#endif
