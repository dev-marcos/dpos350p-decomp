/* Functions that stay in assembly (docs/STATUS.md section 2), declared the way the C callers use them.
 * Addresses are of build 2025-06-19. Names are DERIVED unless the C file says otherwise. */
#ifndef STOCK_ASM_H
#define STOCK_ASM_H

#include "rt/bits.h"

void func_400300e8(u32, int);                                                                    /* 0x400300E8 */
void func_4003f2f4(void);                                                                        /* 0x4003F2F4 */
void sys_mmu_init(void);                                                                         /* 0x40042A60 */
void power_off(void);                                                                            /* 0x4004A1B4 */
void __aeabi_memset_entry(void *, u32, u32);                                                     /* 0x4005C604 */
void ms5351m_b(u32, u32);                                                                        /* 0x4005F1F0 */
void func_40061f10(void);                                                                        /* 0x40061F10 */
void dcache_clean_inv_all(void);                                                                 /* 0x400632F8 */
int csi_set_fmt(u32, void *, void *);                                                            /* 0x40065D04 */
void irq_enable(void);                                                                           /* 0x40066B88 */
void irq_disable(void);                                                                          /* 0x40066B8C */

#endif
