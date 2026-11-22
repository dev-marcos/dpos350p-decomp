/* Functions of nonmatching/drivers/csi/ (one file per function, named after it).
 * Addresses are of build 2025-06-19. Names are DERIVED unless the C file says otherwise. */
#ifndef DRIVERS_CSI_H
#define DRIVERS_CSI_H

#include "rt/bits.h"

u32 csi_readl(u32 addr);                                                                         /* 0x4006582C */
void csi_writel(u32 addr, u32 v);                                                                /* 0x40065834 */
void csi_setbits(u32 addr, u32 bits);                                                            /* 0x4006583C */
void csi_clrbits(u32 addr, u32 bits);                                                            /* 0x40065864 */
void csi_writel_t(u32 addr, u32 v);                                                              /* 0x4006588C */
u32 csi_readl_t(u32 addr);                                                                       /* 0x40065890 */
void csi_clrsetbits(u32 addr, u32 clr, u32 set);                                                 /* 0x40065894 */
int csi_set_base_addr(u32 sel, u32 addr);                                                        /* 0x400658BC */
void csi_enable(u32 sel);                                                                        /* 0x400658DC */
void csi_disable(u32 sel);                                                                       /* 0x400658EC */
void csi_if_cfg(u32 sel, const signed char *cfg);                                                /* 0x400658FC */
void csi_timing_cfg(u32 sel, const signed char *cfg);                                            /* 0x40065954 */
void csi_fmt_cfg(u32 sel, u32 ch, const signed char *fmt);                                       /* 0x400659D0 */
void csi_set_buffer_address(u32 sel, u32 ch, u32 buf, u64 addr);                                 /* 0x40065A60 */
void csi_capture_start(u32 sel, u32 ch_total, u32 mode);                                         /* 0x40065A98 */
void csi_set_size(u32 sel, u32 ch, u32 hor_len, u32 ver_len, u32 buf_len_y, u32 buf_len_c);      /* 0x40065AE8 */
void csi_set_offset(u32 sel, u32 ch, u32 hor_start, u32 ver_start);                              /* 0x40065B70 */
void csi_int_enable(u32 sel, u32 ch, u32 bits);                                                  /* 0x40065BB8 */
void csi_int_get_status(u32 sel, u32 ch, unsigned char *st);                                     /* 0x40065BD4 */
void csi_int_clear_status(u32 sel, u32 ch, u32 bits);                                            /* 0x40065C50 */
void csi_enable_t(u32 sel);                                                                      /* 0x40065C64 */
void csi_reset(u32 sel);                                                                         /* 0x40065C68 */
int csi_init_cfg(u32 sel, const signed char *cfg);                                               /* 0x40065C80 */
void csi_capture_start_t(u32 sel, u32 ch_total, u32 mode);                                       /* 0x40065EA0 */
void csi_int_enable_t(u32 sel, u32 ch, u32 bits);                                                /* 0x40065EA4 */
void csi_int_get_status_t(u32 sel, u32 ch, unsigned char *st);                                   /* 0x40065EA8 */
void csi_int_clear_status_t(u32 sel, u32 ch, u32 bits);                                          /* 0x40065EAC */
void csi_isr(void);                                                                              /* 0x40065EB0 */
void csi_pins_init(void);                                                                        /* 0x40065EF8 */
void csi_clk_init(void);                                                                         /* 0x40065F48 */
int csi_init_structs(u32 rate);                                                                  /* 0x40065FA8 */
void csi_set_buffer(u32 addr);                                                                   /* 0x40066030 */
void csi_irq_setup(void);                                                                        /* 0x40066054 */
void enable_csi_capture(void);                                                                   /* 0x400660AC */
void csi_pins_init_0(void);                                                                      /* 0x400660C0 */
void csi_pins_init_t(void);                                                                      /* 0x400660C4 */
void csi_full_init(u32 rate);                                                                    /* 0x400660C8 */
void csi_clear_frame_flag(void);                                                                 /* 0x40066108 */
void csi_set_frame_flag(u32 v);                                                                  /* 0x4006610C */

#endif
