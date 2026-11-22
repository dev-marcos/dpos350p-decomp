/* Functions of nonmatching/drivers/display/ (one file per function, named after it).
 * Addresses are of build 2025-06-19. Names are DERIVED unless the C file says otherwise. */
#ifndef DRIVERS_DISPLAY_H
#define DRIVERS_DISPLAY_H

#include "rt/bits.h"

void display_start(void);                                                                        /* 0x400344E0 */
u32 lcd_readl(u32 addr);                                                                         /* 0x4003DFFC */
void lcd_writel(u32 addr, u32 v);                                                                /* 0x4003E004 */
void tcon_clock_off(void);                                                                       /* 0x4003E00C */
void de_clock_off(void);                                                                         /* 0x4003E05C */
void de_commit_base(u32 de);                                                                     /* 0x4003E304 */
void display_init(u32 width, u32 height, u32 fb);                                                /* 0x4003E5D8 */
void lcd_pins_init(void);                                                                        /* 0x4003E63C */
void lcd_start(void);                                                                            /* 0x4003E688 */
u32 de_readl(u32 addr);                                                                          /* 0x40062F9C */
void de_writel(u32 addr, u32 v);                                                                 /* 0x40062FA4 */
int de_pipe_enable(int pipe, int on);                                                            /* 0x40062FAC */
int de_route(int pipe, u32 ch);                                                                  /* 0x4006301C */
int de_ui_set_layer(int ch, int layer, u32 x, u32 y, u32 w, u32 h, u32 addr, u32 pitch, signed char fmt); /* 0x40063070 */
int de_ui_set_size(int ch, u32 w, u32 h);                                                        /* 0x40063108 */
int de_setup_ui(void);                                                                           /* 0x4006314C */

#endif
