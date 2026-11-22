/* Functions of nonmatching/drivers/gic/ (one file per function, named after it).
 * Addresses are of build 2025-06-19. Names are DERIVED unless the C file says otherwise. */
#ifndef DRIVERS_GIC_H
#define DRIVERS_GIC_H

#include "hw/platform.h"
#include "rt/bits.h"

u32 gic_readl(u32 addr);                                                                         /* 0x400668D0 */
void gic_writel(u32 addr, u32 v);                                                                /* 0x400668D8 */
void default_irq_handler(void);                                                                  /* 0x400668E0 */
void irq_dispatch(void);                                                                         /* 0x400668E4 */
void gic_enable_irq(gic_t *g, int irq);                                                          /* 0x40066950 */
void gic_disable_irq(gic_t *g, int irq);                                                         /* 0x40066994 */
int gic_cpu_id(void);                                                                            /* 0x400669D8 */
void gic_dist_init(u32 base);                                                                    /* 0x400669E0 */
void gic_cpu_init(u32 base);                                                                     /* 0x40066B10 */
void local_irq_enable(void);                                                                     /* 0x40066B88 */
void gic_init(void);                                                                             /* 0x40066B8C */
void irq_register(int irq, void (*handler)(void), int prio, int enable);                         /* 0x40066BD4 */
void local_irq_enable_body(void);                                                                /* 0x40066C6C */

#endif
