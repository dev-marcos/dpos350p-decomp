/* Functions of nonmatching/system/ (one file per function, named after it).
 * Addresses are of build 2025-06-19. Names are DERIVED unless the C file says otherwise. */
#ifndef SYSTEM_H
#define SYSTEM_H

#include "rt/bits.h"

int main(void);                                                                                  /* 0x40034700 */
void reload_chain_40035744(void);                                                                /* 0x40035744 */
void reload_chain_400373e0(void);                                                                /* 0x400373E0 */
void reload_chain_400378e0(void);                                                                /* 0x400378E0 */
void outputs_apply(void);                                                                        /* 0x4003E708 */
void system_defaults_init(void);                                                                 /* 0x4003E738 */
void state_block_3f984_init(void);                                                               /* 0x4003ED28 */
void state_block_180c8_init(void);                                                               /* 0x4003ED80 */
void state_block_68668_init(void);                                                               /* 0x4003EDF4 */
void state_blocks_init(void);                                                                    /* 0x4003EEB0 */
void applied_state_invalidate(void);                                                             /* 0x4003EF1C */
void factory_defaults(void);                                                                     /* 0x4003EFAC */
void sys_info_backup(void);                                                                      /* 0x4003F024 */
void state_c_backup(void);                                                                       /* 0x4003F044 */
void state_d_backup(void);                                                                       /* 0x4003F054 */
void sys_info_restore(void);                                                                     /* 0x4003F064 */
void state_c_restore(void);                                                                      /* 0x4003F098 */
void state_d_restore(void);                                                                      /* 0x4003F0A8 */
void state_backup_all(void);                                                                     /* 0x4003F0C0 */
void state_restore_all(void);                                                                    /* 0x4003F0E0 */
void mmu_map_sections(volatile u32 *tbl, unsigned long long va, unsigned long long pa, unsigned long long size, unsigned char attr); /* 0x400429E4 */
void factory_reset_keep_language(void);                                                          /* 0x400447C8 */
void retained_inside_current_sc1_3_t6(void);                                                     /* 0x4004C544 */
int system_calibration(void);                                                                    /* 0x4004C6B4 */
u32 percent_at_least(int v, int total, int min_pct);                                             /* 0x4004C794 */
void calibration_result_flags(void);                                                             /* 0x4004FAB0 */
void table_684_init(void);                                                                       /* 0x40063A50 */

#endif
