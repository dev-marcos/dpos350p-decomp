/* Functions of nonmatching/bode/ (one file per function, named after it).
 * Addresses are of build 2025-06-19. Names are DERIVED unless the C file says otherwise. */
#ifndef BODE_H
#define BODE_H

#include "rt/bits.h"

void bode_grid_draw(void);                                                                       /* 0x40038374 */
void bode_curves_draw(void);                                                                     /* 0x40039254 */
void bode_autoscale(void);                                                                       /* 0x4003963C */
void bode_curves_zoom(void);                                                                     /* 0x40039918 */
void bode_phase_measure(u32 idx);                                                                /* 0x40039C08 */
void bode_calibrate(void);                                                                       /* 0x40039EB8 */
void bode_point_measure(u32 f);                                                                  /* 0x4003A044 */
void bode_sweep_run(void);                                                                       /* 0x4003A300 */

#endif
