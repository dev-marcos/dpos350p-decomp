/* Functions of nonmatching/drivers/mmio/ (one file per function, named after it).
 * Addresses are of build 2025-06-19. Names are DERIVED unless the C file says otherwise. */
#ifndef DRIVERS_MMIO_H
#define DRIVERS_MMIO_H

#include "rt/bits.h"

u32 readl_e(u32 addr);                                                                           /* 0x400427A0 */
void writel_e(u32 addr, u32 v);                                                                  /* 0x400427A8 */
u32 readl_d(u32 addr);                                                                           /* 0x40042B50 */
void writel_d(u32 addr, u32 v);                                                                  /* 0x40042B58 */
u32 readb_f(u32 addr);                                                                           /* 0x40042E6C */
u32 readl_f(u32 addr);                                                                           /* 0x40042E74 */
void writeb_f(u32 addr, u32 v);                                                                  /* 0x40042E7C */
void writel_f(u32 addr, u32 v);                                                                  /* 0x40042E84 */
u32 readl_g(u32 addr);                                                                           /* 0x40066C80 */
void writel_g(u32 addr, u32 v);                                                                  /* 0x40066C88 */
u32 readl_b(u32 addr);                                                                           /* 0x40067478 */
void writel_b(u32 addr, u32 v);                                                                  /* 0x40067480 */
u32 readl_a(u32 addr);                                                                           /* 0x40067598 */
void writel_a(u32 addr, u32 v);                                                                  /* 0x400675A0 */
u32 readl_c(u32 addr);                                                                           /* 0x40068E64 */
void writel_c(u32 addr, u32 v);                                                                  /* 0x40068E6C */

#endif
