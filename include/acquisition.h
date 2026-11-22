/* Functions of nonmatching/acquisition/ (one file per function, named after it).
 * Addresses are of build 2025-06-19. Names are DERIVED unless the C file says otherwise. */
#ifndef ACQUISITION_H
#define ACQUISITION_H

#include "rt/bits.h"

void sys_3c_enter(void);                                                                         /* 0x400367A4 */
void sys_3c_leave(void);                                                                         /* 0x400367D0 */
void acquisition_reconfigure(void);                                                              /* 0x4003A62C */
void acquisition_apply(void);                                                                    /* 0x4003A654 */
void timebase_step_table_init(void);                                                             /* 0x4003A680 */
void timebase_period_load(void);                                                                 /* 0x4003A8BC */
void sample_divisor_setup(void);                                                                 /* 0x4003A908 */
void acquisition_timing_setup(void);                                                             /* 0x4003A984 */
void acquisition_rearm(void);                                                                    /* 0x4003B7B8 */
void timebase_divisors_compute(void);                                                            /* 0x4003B834 */
void timebase_index_clamp(void);                                                                 /* 0x4003BCF0 */
void timebase_index_step(u32 d);                                                                 /* 0x4003BDB0 */
void timebase_index_step_down(u32 d);                                                            /* 0x4003BDE8 */
void timebase_index_set(u32 v);                                                                  /* 0x4003BE50 */
void timebase_slow_flag_update(void);                                                            /* 0x4003BE78 */
void sys_word_pair_570_set(u32 lo, u32 hi);                                                      /* 0x4003BE98 */
void runtime_state_commit_force(void);                                                           /* 0x4003BEA8 */
void runtime_state_commit_copy(void);                                                            /* 0x4003BEB4 */
u32 mode_not_0_or_2(u32 v);                                                                      /* 0x4003BF58 */
u32 mode_not_0_or_1(u32 v);                                                                      /* 0x4003BF78 */
void runtime_counter_progress_advance(void);                                                     /* 0x4003BF98 */
void acquisition_geometry_setup(void);                                                           /* 0x4003C008 */
void scaled_pair_update(u32 mode);                                                               /* 0x4003C0AC */
void runtime_rearm_setup(void);                                                                  /* 0x4003C520 */
void acq_defaults_reset(void);                                                                   /* 0x4003EB40 */
void timebase_apply(void);                                                                       /* 0x4003F894 */
void runtime_ready_read(void);                                                                   /* 0x4003F8B8 */
void acq_done_read(void);                                                                        /* 0x4003F9D0 */
void acquisition_prepare(void);                                                                  /* 0x4003FA10 */
void veneer_400636bc(void);                                                                      /* 0x4003FAB0 */
void veneer_40063714(void);                                                                      /* 0x4003FAB4 */
void trigger_pos_update(void);                                                                   /* 0x4003FAB8 */
void veneer_40063798(void);                                                                      /* 0x4003FAE4 */
void trigger_offsets_update(void);                                                               /* 0x4003FAE8 */
void consumer_translator_of_the_16_bit(void);                                                    /* 0x4003FB90 */
void acq_wait_done(void);                                                                        /* 0x4003FCC0 */
void trace_buffer_pointers_setup(u32 n);                                                         /* 0x4003FFE4 */
void timebase_apply_2b4(void);                                                                   /* 0x40040030 */
void timebase_apply_2b0(void);                                                                   /* 0x4004003C */
void timebase_apply_2b0_x1_3(void);                                                              /* 0x40040048 */
void trigger_window_crossing(u32 ch);                                                            /* 0x400403AC */
void stream_geometry_fpga_selector_0x22_configuration(void);                                     /* 0x400405E0 */
void capture_block(u32 v, u32 buf);                                                              /* 0x40040638 */
void capture_with_length(u32 v, u32 len, u32 buf);                                               /* 0x400406A4 */
void capture_bank(u32 i);                                                                        /* 0x400406C8 */
void capture_banks_all(void);                                                                    /* 0x400406DC */
void capture_banks_0_3(void);                                                                    /* 0x400406FC */
void capture_banks_4_7(void);                                                                    /* 0x4004071C */
void calibration_gain_baseline_clamp_conversion(u32 b, u32 ch, volatile unsigned short *buf, u32 nb, u32 slot, u32 phase, u32 count); /* 0x4004073C */
void first_conversion_wrapper(volatile unsigned short *buf, u32 ch, u32 n);                      /* 0x400408FC */
void bank_unpack_0_3_n(volatile unsigned short *buf, u32 ch, u32 n);                             /* 0x40040A5C */
void second_conversion_wrapper(volatile unsigned short *buf, u32 ch, u32 n);                     /* 0x40040B20 */
void bank_unpack_all(volatile unsigned short *buf, u32 ch);                                      /* 0x40040BEC */
void bank_unpack_0_3(volatile unsigned short *buf, u32 ch);                                      /* 0x40040D74 */
void bank_unpack_4_7(volatile unsigned short *buf, u32 ch);                                      /* 0x40040E44 */
void full_acquisition_entry(void);                                                               /* 0x400413C8 */
void one_runtime_capture_step(void);                                                             /* 0x400417E4 */
void full_acquisition_veneer(void);                                                              /* 0x40041918 */
void runtime_sweep_path(void);                                                                   /* 0x4004191C */
void acquisition_dispatcher(void);                                                               /* 0x400419CC */
void sys_word_pair_578_set(u32 lo, u32 hi);                                                      /* 0x400424C4 */
void runtime_event_service(void);                                                                /* 0x40049B08 */
void acquisition_block_bank_selector(u32 i);                                                     /* 0x4006338C */
void trigger_level_auto(void);                                                                   /* 0x4006382C */

#endif
