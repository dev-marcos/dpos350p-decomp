/* Functions of nonmatching/ui/draw/ (one file per function, named after it).
 * Addresses are of build 2025-06-19. Names are DERIVED unless the C file says otherwise. */
#ifndef UI_DRAW_H
#define UI_DRAW_H

#include "rt/bits.h"

void colour_table_3dd530_init(void);                                                             /* 0x40019AF4 */
void switch_on_off_draw(u32 x, u32 y, u32 on);                                                   /* 0x40019B8C */
void icon_19c7c_draw(u32 x, u32 y);                                                              /* 0x40019C7C */
void slider_draw(u32 x, u32 y, unsigned char v);                                                 /* 0x40019CF8 */
void battery_icon_draw(void);                                                                    /* 0x4001C200 */
void window_frame_23d04_draw(void);                                                              /* 0x40023D04 */
void image_42468a58_show(void);                                                                  /* 0x40025D08 */
void sys_rect_fill(void);                                                                        /* 0x4002C5C4 */
void sys_rect_redraw_unless_t17(void);                                                           /* 0x4002CC6C */
void word_42e68c74_set(u32 a);                                                                   /* 0x4002CCCC */
void fonts_init(void);                                                                           /* 0x4002CCD8 */
void glyph_14x14_draw(u32 x, u32 y, unsigned short code);                                        /* 0x4002CDC4 */
void text_draw(u32 x, u32 y, const char *s);                                                     /* 0x4002CE7C */
void text_shadow_draw(u32 x, u32 y, const char *s, u32 n, unsigned char m, unsigned char alpha); /* 0x4002D0D4 */
u32 text_width(const char *s);                                                                   /* 0x4002D164 */
u32 number_text_width(long long v, u32 dec, u32 mode);                                           /* 0x4002D2D4 */
void func_4002d82c_flag1(u32 a, u32 b, u32 c, u32 d, u32 e);                                     /* 0x4002D6D4 */
void func_4002d82c_flag0(u32 a, u32 b, u32 c, u32 d, u32 e);                                     /* 0x4002D6F8 */
void func_4002d2d4_d0(u32 a, u32 b, u32 c);                                                      /* 0x4002D71C */
void text_d6f8_shadow_draw(u32 x, u32 y, u32 a, u32 b, unsigned char e, unsigned char n, unsigned char m, unsigned char alpha); /* 0x4002D724 */
void number_draw(u32 x, u32 y, long long v, unsigned char dec, signed char mode);                /* 0x4002D82C */
void half_42e68be2_set(u32 a);                                                                   /* 0x4002DC40 */
void draw_state_defaults(void);                                                                  /* 0x4002DC4C */
void byte_42e68be0_set(u32 a);                                                                   /* 0x4002DC74 */
void font_metrics_build(volatile unsigned char *d);                                              /* 0x4002DCAC */
unsigned short font_glyph_index(u32 c);                                                          /* 0x4002DD70 */
void font_char_draw(u32 x, u32 y, u32 ch);                                                       /* 0x4002E6D8 */
unsigned short char_to_digit_glyph(u32 c);                                                       /* 0x4002E7B0 */
void font_char_tint_draw(u32 x, u32 y, u32 ch);                                                  /* 0x4002E878 */
void draw_colour_set(u32 c);                                                                     /* 0x4002EA38 */
void pixel_blend(u32 x, u32 y);                                                                  /* 0x4002EA88 */
void pixel_put(u32 x, u32 y);                                                                    /* 0x4002EB70 */
void func_4002ecc4_span(u32 a, u32 b, u32 c);                                                    /* 0x4002EBA8 */
void vline_draw(u32 x, u32 y0, u32 y1);                                                          /* 0x4002EBC8 */
void rect_outline_draw(u32 x, u32 y, u32 w, u32 h);                                              /* 0x4002EC4C */
void rect_fill(u32 x, u32 y, u32 w, u32 h);                                                      /* 0x4002ECC4 */
void line_draw(int x0, int y0, int x1, int y1);                                                  /* 0x4002ED90 */
void glyph_alpha_draw(u32 x, u32 y, const unsigned char *g);                                     /* 0x4002EE78 */
void draw_text_shadowed(u32 x, u32 y, u32 s, u32 dx, u32 dy, u32 col);                           /* 0x4002F0E8 */
void colour_toggle_42e68d1a(void);                                                               /* 0x40031710 */
void rect_42e68c38_draw(void);                                                                   /* 0x40034284 */
void draw_target_default(void);                                                                  /* 0x400342CC */
void fb_target_back(void);                                                                       /* 0x400342DC */
void fb_target_offscreen(void);                                                                  /* 0x400342EC */
void fb_rect_copy_to_e4e84(u32 x0, u32 y0, u32 x1, u32 y1);                                      /* 0x400342FC */
void fb_rect_copy_from_e4e84(u32 x0, u32 y0, u32 x1, u32 y1);                                    /* 0x4003436C */
void fb_present_rect(u32 x0, u32 y0, u32 x1, u32 y1);                                            /* 0x400343C4 */
void fb_present(void);                                                                           /* 0x4003442C */
void fb_clear_current(void);                                                                     /* 0x40034454 */
void fb_clear_all(void);                                                                         /* 0x4003446C */
void number_text_88c0(int n);                                                                    /* 0x40037160 */
void phase_deg_draw(u32 x, u32 y, u32 sg, double d);                                             /* 0x40038788 */
void voltage_uv_draw(u32 x, u32 y, u32 sg, double d);                                            /* 0x40038974 */
void level_value_draw(u32 x, u32 y, u32 sg, double d);                                           /* 0x40038C00 */
void level_label_db(u32 x, u32 y, u32 c, double v);                                              /* 0x40038DAC */
void level_label_dbmv(u32 x, u32 y, u32 c, double v);                                            /* 0x40038DEC */
u32 frequency_draw(u32 x, u32 y, u32 measure, double d);                                         /* 0x40038E40 */
void fft_traces_draw(void);                                                                      /* 0x40041E34 */
void intensity_palettes_init(void);                                                              /* 0x4005DB9C */
void trace_intensity_draw(u32 tr);                                                               /* 0x4005DDD4 */
void trace_intensity_draw_window(u32 tr);                                                        /* 0x4005DF5C */
void layer_clear(u32 n);                                                                         /* 0x4005E10C */
void trace_scale_segment(u32 tr, u32 start, u32 len);                                            /* 0x4005F778 */
void level_columns_draw_b(void);                                                                 /* 0x40061194 */
void level_columns_draw(void);                                                                   /* 0x40062C98 */

#endif
