/* Functions of nonmatching/ui/windows/ (one file per function, named after it).
 * Addresses are of build 2025-06-19. Names are DERIVED unless the C file says otherwise. */
#ifndef UI_WINDOWS_H
#define UI_WINDOWS_H

#include "rt/bits.h"

void ui_window_05_draw(void);                                                                    /* 0x4001C348 */
void ui_windows_close_untouched(void);                                                           /* 0x4002F194 */
void ui_frame_chain(void);                                                                       /* 0x400346C0 */
void ui_window_redraw_dirty(void);                                                               /* 0x4003CA88 */
void ui_window_stack_top_update(void);                                                           /* 0x4003CD0C */
void ui_window_invalidate_from(u32 id, u32 r1);                                                  /* 0x4003CD44 */
void ui_windows_mark_dirty(void);                                                                /* 0x4003CDC0 */
void ui_window_open(u32 id);                                                                     /* 0x4003CDFC */
void ui_window_close(u32 id, u32 r1, u32 r2);                                                    /* 0x4003CE50 */
void ui_windows_reset(void);                                                                     /* 0x4003CF0C */
void ui_frame_update(void);                                                                      /* 0x4003CF58 */
void ui_layout_init(void);                                                                       /* 0x4003CF88 */
void ui_reveal_bottom_up(volatile unsigned short *r);                                            /* 0x4003D46C */
void ui_reveal_slide_down(volatile unsigned short *r);                                           /* 0x4003D594 */
void ui_windows_open_1_5(void);                                                                  /* 0x4003D6AC */
void ui_windows_invalidate_1_5(u32 r0, u32 r1);                                                  /* 0x4003D6E4 */
void ui_window_01_redraw(void);                                                                  /* 0x4003D71C */
void ui_window_02_redraw(void);                                                                  /* 0x4003D734 */
void ui_window_03_redraw(void);                                                                  /* 0x4003D74C */
void ui_window_05_redraw(void);                                                                  /* 0x4003D764 */
void ui_window_04_redraw(void);                                                                  /* 0x4003D77C */
void ui_window_06_redraw(u32 anim);                                                              /* 0x4003D794 */
void ui_window_10_redraw(u32 anim);                                                              /* 0x4003D7CC */
void ui_window_11_redraw(u32 anim);                                                              /* 0x4003D81C */
void ui_window_12_redraw(u32 anim);                                                              /* 0x4003D86C */
void ui_window_13_redraw(u32 anim);                                                              /* 0x4003D8A4 */
void ui_window_14_redraw(u32 anim);                                                              /* 0x4003D8DC */
void ui_window_15_redraw(u32 anim);                                                              /* 0x4003D914 */
void ui_window_16_redraw(u32 anim);                                                              /* 0x4003D950 */
void ui_window_17_redraw(u32 anim);                                                              /* 0x4003D98C */
void ui_window_18_redraw(u32 anim);                                                              /* 0x4003D9C8 */
void ui_window_19_redraw(void);                                                                  /* 0x4003DA00 */
void ui_window_20_redraw(u32 anim);                                                              /* 0x4003DA18 */
void ui_window_21_redraw(u32 anim);                                                              /* 0x4003DA50 */
void ui_window_22_redraw(u32 anim);                                                              /* 0x4003DA88 */
void ui_window_23_redraw(void);                                                                  /* 0x4003DAC0 */
void ui_window_24_redraw(u32 anim);                                                              /* 0x4003DAD8 */
void ui_window_25_redraw(u32 anim);                                                              /* 0x4003DB10 */
void ui_window_26_redraw(u32 anim);                                                              /* 0x4003DB48 */
void ui_window_27_redraw(u32 anim);                                                              /* 0x4003DB80 */
void ui_window_28_redraw(void);                                                                  /* 0x4003DBB8 */
void ui_window_07_redraw(u32 anim);                                                              /* 0x4003DBD0 */
void ui_window_08_redraw(u32 anim);                                                              /* 0x4003DC08 */
void ui_window_09_redraw(u32 anim);                                                              /* 0x4003DC40 */
void ui_window_29_redraw(u32 anim);                                                              /* 0x4003DC78 */
void ui_window_30_redraw(u32 anim);                                                              /* 0x4003DCB0 */
void ui_window_31_redraw(u32 anim);                                                              /* 0x4003DCEC */
void ui_window_32_redraw(u32 anim);                                                              /* 0x4003DD24 */
void ui_window_33_redraw(u32 anim);                                                              /* 0x4003DD5C */
void ui_window_34_redraw(void);                                                                  /* 0x4003DDB0 */
void ui_window_35_redraw(void);                                                                  /* 0x4003DDC8 */
void ui_window_36_redraw(void);                                                                  /* 0x4003DDE0 */
void ui_window_37_redraw(u32 anim);                                                              /* 0x4003DDF8 */
void ui_window_38_redraw(void);                                                                  /* 0x4003DE34 */
void ui_window_39_redraw(void);                                                                  /* 0x4003DE4C */
void ui_window_40_redraw(void);                                                                  /* 0x4003DE64 */
void ui_window_41_redraw(u32 anim);                                                              /* 0x4003DE7C */
void ui_window_42_redraw(u32 anim);                                                              /* 0x4003DEB4 */
void window_64_recalc(void);                                                                     /* 0x40061680 */
void ui_window_40_draw(void);                                                                    /* 0x40062EE8 */

#endif
