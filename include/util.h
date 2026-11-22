/* Functions of nonmatching/util/ (one file per function, named after it).
 * Addresses are of build 2025-06-19. Names are DERIVED unless the C file says otherwise. */
#ifndef UTIL_H
#define UTIL_H

#include "rt/bits.h"

u32 value_in_table_405456e8(u32 mode, int v);                                                    /* 0x400301A8 */
unsigned char *str_copy(unsigned char *d, const unsigned char *s);                               /* 0x400342B4 */
void *memset_w(void *d, int v, unsigned int n);                                                  /* 0x400356E4 */
void fill_u16(unsigned short *d, unsigned short v, u32 n);                                       /* 0x400356F4 */
void fill_u32(u32 *d, u32 v, u32 n);                                                             /* 0x4003570C */
void *memcpy(void *d, const void *s, unsigned int n);                                            /* 0x40035724 */
void copy_u16(unsigned short *d, const unsigned short *s, u32 n);                                /* 0x40035728 */
int str_find_char_next(const signed char *s, int ch);                                            /* 0x40056BE4 */
int str_find_char(const signed char *s, int ch);                                                 /* 0x40056BE8 */
void index_map_if_above(u32 a, u32 b, u32 c, u32 d, u32 e, u32 g);                               /* 0x4005C918 */
void mem_clear(void *d, unsigned int n);                                                         /* 0x4005F914 */
int strcmp(const char *a, const char *b);                                                        /* 0x40068F78 */
u32 strlen(const char *str);                                                                     /* 0x4006A6A8 */

#endif
