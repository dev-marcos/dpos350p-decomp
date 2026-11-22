/* Functions of nonmatching/storage/settings/ (one file per function, named after it).
 * Addresses are of build 2025-06-19. Names are DERIVED unless the C file says otherwise. */
#ifndef STORAGE_SETTINGS_H
#define STORAGE_SETTINGS_H

#include "rt/bits.h"

int persistence_read(u32 dev, u32 addr, volatile unsigned char *buf, u32 len);                   /* 0x40043620 */
void persistence_write_read_modify_write(u32 dev, u32 addr, const volatile unsigned char *buf, u32 len); /* 0x40043874 */
void settings_a_fields(void);                                                                    /* 0x400439A0 */
void settings_a_save(void);                                                                      /* 0x400439D8 */
void settings_a_load(void);                                                                      /* 0x40043A0C */
void settings_b_fields(void);                                                                    /* 0x40043A40 */
void settings_b_save(void);                                                                      /* 0x40043A80 */
void settings_b_load(void);                                                                      /* 0x40043AB4 */
void settings_c_save(void);                                                                      /* 0x40043C68 */
void settings_c_load(void);                                                                      /* 0x40043C9C */
void settings_c_load_tail(volatile unsigned char *p, u32 off, u32 unused, u32 len);              /* 0x40043CA8 */
void settings_d_fields(void);                                                                    /* 0x40043CD0 */
void settings_d_save(void);                                                                      /* 0x40043D54 */
void settings_d_load(void);                                                                      /* 0x40043D88 */
void settings_e_fields(void);                                                                    /* 0x40043DBC */
void settings_e_save(void);                                                                      /* 0x40043E00 */
void settings_e_load(void);                                                                      /* 0x40043E34 */
void settings_f_fields(void);                                                                    /* 0x40043EC8 */
void settings_f_save(void);                                                                      /* 0x40043EEC */
void settings_f_load(void);                                                                      /* 0x40043F20 */
void slot_fields(void);                                                                          /* 0x40043F54 */
void settings_slot_save(void);                                                                   /* 0x4004435C */
void settings_slot_load(void);                                                                   /* 0x400443D4 */
void calibration_process_serialized_fields(void);                                                /* 0x40044450 */
void calibration_save_wrapper(void);                                                             /* 0x40044680 */
void calibration_load_wrapper(void);                                                             /* 0x400446B4 */
void settings_g_fields(void);                                                                    /* 0x400446E8 */
void settings_g_save(void);                                                                      /* 0x4004472C */
void settings_g_load(void);                                                                      /* 0x40044760 */
void settings_load_all(void);                                                                    /* 0x40044794 */
void settings_h_fields(void);                                                                    /* 0x4004480C */
void settings_image_fields(void);                                                                /* 0x400448B4 */
void settings_i_fields(void);                                                                    /* 0x40045580 */
void settings_j_fields(void);                                                                    /* 0x400455E4 */
void settings_h_decode(void);                                                                    /* 0x40045670 */
void settings_block_448b4_decode(void);                                                          /* 0x40045688 */
void settings_i_decode(void);                                                                    /* 0x400456AC */
void settings_j_decode(void);                                                                    /* 0x400456D0 */
void settings_image_decode(void);                                                                /* 0x400456F4 */
void settings_image_encode(void);                                                                /* 0x40045714 */
void sd_update_check(void);                                                                      /* 0x4004F134 */
void serializer_set_cursor(u32 v);                                                               /* 0x40067128 */
void serializer_set_encode_mode(void);                                                           /* 0x40067134 */
void serializer_set_decode_mode(void);                                                           /* 0x40067144 */
u32 serializer_get_cursor(void);                                                                 /* 0x40067154 */
void serializer_encode_scalar_be(u32 off, const volatile void *src, u32 size);                   /* 0x40067160 */
void serializer_encode_field(const volatile void *src, u32 size);                                /* 0x40067278 */
void serializer_decode_scalar_be(u32 off, volatile void *dst, u32 size);                         /* 0x400672A8 */
void serializer_decode_field(volatile void *dst, u32 size);                                      /* 0x40067420 */
void serializer_process_field(volatile void *p, u32 size);                                       /* 0x40067450 */

#endif
