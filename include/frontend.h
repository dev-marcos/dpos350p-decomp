/* Functions of nonmatching/frontend/ (one file per function, named after it).
 * Addresses are of build 2025-06-19. Names are DERIVED unless the C file says otherwise. */
#ifndef FRONTEND_H
#define FRONTEND_H

#include "rt/bits.h"

void rate_table_init_entry(void);                                                                /* 0x4004A50C */
void frontend_apply_all(void);                                                                   /* 0x4004A510 */
void rate_table_init(void);                                                                      /* 0x4004A550 */
void vscale_update(void);                                                                        /* 0x4004A608 */
void offset_clamp(void);                                                                         /* 0x4004A7C4 */
void vdiv_label(u32 ch, u32 r1, int d, int unit);                                                /* 0x4004A884 */
void vdiv_labels_update(void);                                                                   /* 0x4004AA04 */
void ch1_frontend_apply(void);                                                                   /* 0x4004AAC0 */
void ch2_frontend_apply(void);                                                                   /* 0x4004AC04 */
int post_channel_mux_check(u32 vdiv);                                                            /* 0x4004AF0C */
void frontend_offset_calibrate(u32 r);                                                           /* 0x4004BFC8 */
void auto_set(void);                                                                             /* 0x4004C7C4 */
void auto_set_from(u32 box_base);                                                                /* 0x4004C7C8 */

#endif
