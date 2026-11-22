/* Functions of nonmatching/generator/ (one file per function, named after it).
 * Addresses are of build 2025-06-19. Names are DERIVED unless the C file says otherwise. */
#ifndef GENERATOR_H
#define GENERATOR_H

#include "hw/fpga.h"
#include "rt/bits.h"

void gen_state_backup(void);                                                                     /* 0x4003F034 */
void gen_state_restore(void);                                                                    /* 0x4003F088 */
void gen_limits(u32 r0, u32 r1, u32 lo, u32 hi);                                                 /* 0x4004CF34 */
void gen_table_repeat_default(void);                                                             /* 0x4004D094 */
void gen_table_repeat(u32 off, u32 r1, volatile touch_t *t);                                     /* 0x4004D09C */
void gen_load_samples(void);                                                                     /* 0x4004D12C */
void Gen_Init(void);                                                                             /* 0x4004D164 */
void gen_update(void);                                                                           /* 0x4004D18C */
void gen_send_params(void);                                                                      /* 0x4004D1B4 */
void gen_decimate(int n);                                                                        /* 0x4004D350 */
void gen_normalize(void);                                                                        /* 0x4004D390 */
void gen_resample(void);                                                                         /* 0x4004D49C */
void gen_decimate2(int n);                                                                       /* 0x4004D5CC */
void gen_normalize2(void);                                                                       /* 0x4004D618 */
void gen_resample2(void);                                                                        /* 0x4004D730 */
void gen_wave_build(void);                                                                       /* 0x4004D880 */
void gen_fpga_settings_write(void);                                                              /* 0x4004ED18 */
void gen_output_calibrate(void);                                                                 /* 0x4004EF10 */
int sine_table_lookup(int x);                                                                    /* 0x4005D220 */
void sine_table_build(double a0);                                                                /* 0x4005D27C */

#endif
