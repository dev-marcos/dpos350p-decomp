/* Functions of nonmatching/selftest/ (one file per function, named after it).
 * Addresses are of build 2025-06-19. Names are DERIVED unless the C file says otherwise. */
#ifndef SELFTEST_H
#define SELFTEST_H

#include "rt/bits.h"

void selftest_4f5e4(void);                                                                       /* 0x4004F5E4 */
void gen_selftest_4f89c(void);                                                                   /* 0x4004F89C */
void factory_channel_gen_check(void);                                                            /* 0x4004FB44 */
void gen_selftest_504b8(void);                                                                   /* 0x400504B8 */
void gen_selftest_50688(void);                                                                   /* 0x40050688 */
void gen_selftest_50898(void);                                                                   /* 0x40050898 */
void gen_selftest_50bac(void);                                                                   /* 0x40050BAC */
void gen_selftest_50ddc(void);                                                                   /* 0x40050DDC */
void factory_test_board(void);                                                                   /* 0x40050FE0 */
void factory_test_tester(void);                                                                  /* 0x40051578 */
void touch_targets_test(void);                                                                   /* 0x400519E4 */
void factory_steps_run(void);                                                                    /* 0x40051CF0 */
void calib_run_4_channels(void);                                                                 /* 0x40060D68 */
int gen_selftest_64c28(void);                                                                    /* 0x40064C28 */
int gen_selftest_64cf0(void);                                                                    /* 0x40064CF0 */

#endif
