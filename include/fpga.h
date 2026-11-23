/* Functions of nonmatching/fpga/ (one file per function, named after it).
 * Addresses are of build 2025-06-19. Names are DERIVED unless the C file says otherwise. */
#ifndef FPGA_H
#define FPGA_H

#include "rt/bits.h"

void fpga_params_6e4_sync(void);                                                                 /* 0x4003F160 */
void pulse_fpga_selector_0x08_0_1(void);                                                         /* 0x4003F8E0 */
void write_sys_info_0x6f0_to_fpga(void);                                                         /* 0x4003F910 */
void fpga_sample_count_write(u32 n);                                                             /* 0x4003F948 */
void fpga_trigger_pos_write(u32 v);                                                              /* 0x4003F988 */
void pe0_set(void);                                                                              /* 0x4003F9F8 */
void pe0_clr(void);                                                                              /* 0x4003FA04 */
void fpga_write_sel_0x1f(u32 v);                                                                 /* 0x4003FA94 */
void fpga_pe_mode_select(u32 mode);                                                              /* 0x40049094 */
void FPGA_Start(void);                                                                           /* 0x40049188 */
void perform_fpga_identification_probe(void);                                                    /* 0x400492E4 */
void security_chip_check(void);                                                                  /* 0x40049E00 */
void fpga_params_push(void);                                                                     /* 0x40049E30 */
void fpga_param_updater(u32 idx, u32 size, u32 v32, u32 vsmall);                                 /* 0x40049EC4 */
void smec_commit(u32 idx);                                                                       /* 0x40049F1C */
void smec_timing_reset(void);                                                                    /* 0x40049F44 */
void selector_0x6a_one_byte_job_index(u32 v);                                                    /* 0x40049F60 */
void selector_0x6b_32_bit_job_value(u32 v);                                                      /* 0x40049F7C */
void selector_0x69_16_bit_timing_retry(u32 v);                                                   /* 0x40049FB8 */
void smec_start_job(void);                                                                       /* 0x40049FDC */
u32 smec_job_valid(void);                                                                        /* 0x4004A020 */
void smec_wait_ready(void);                                                                      /* 0x4004A040 */
u32 smec_job_echo(void);                                                                         /* 0x4004A064 */
u32 smec_job_result(void);                                                                       /* 0x4004A080 */
u32 smec_run_job(u32 idx, u32 val);                                                              /* 0x4004A0B0 */
void smec_reset_cmd(void);                                                                       /* 0x4004A13C */
void SMEC_Init(void);                                                                            /* 0x4004A15C */
void fpga_channel_mode_apply(void);                                                              /* 0x4004AA24 */
void fpga_write_sel_0x62(void);                                                                  /* 0x4004AA70 */
void fpga_write_sel_0x2a(void);                                                                  /* 0x4004AA94 */
void fpga_write_sel_0x2f(void);                                                                  /* 0x4004AB60 */
void fpga_write_sel_0x29(void);                                                                  /* 0x4004AB84 */
void fpga_write_sel_0x2c(void);                                                                  /* 0x4004ABA8 */
void fpga_write_sel_0x2d(void);                                                                  /* 0x4004ABCC */
void fpga_write_sel_0x30(void);                                                                  /* 0x4004ACA4 */
u32 lane_level_sum(u32 start, u32 lane, u32 ch, u32 n, unsigned char *buf);                      /* 0x4004ACC8 */
u32 lane_level_mean(u32 start, u32 lane, u32 ch, u32 n, unsigned char *buf);                     /* 0x4004ADCC */
void lane_gain_calibrate(void);                                                                  /* 0x4004B0A0 */
void fpga_sel42_43_fill(u32 n);                                                                  /* 0x4004EC98 */
void fpga_cmd63_write(void);                                                                     /* 0x4004EEEC */
void fpga_probe_and_copy_7fc(void);                                                              /* 0x4004F59C */
void security_check_and_copy(void);                                                              /* 0x4004F5C0 */
void fpga_config_push(void);                                                                     /* 0x40063328 */
void fpga_cmd0c_write(void);                                                                     /* 0x40063354 */
void fpga_cmd15_write(void);                                                                     /* 0x40063430 */
void fpga_cmd16_write(void);                                                                     /* 0x40063460 */
void fpga_params_0e_13_write(void);                                                              /* 0x400634F0 */
void fpga_cmd61_write(void);                                                                     /* 0x400635D0 */
void fpga_cmd61_pulse(void);                                                                     /* 0x400635F4 */
void fpga_cmd17_write(void);                                                                     /* 0x40063628 */
void fpga_cmd1a_write(void);                                                                     /* 0x4006366C */
void fpga_cmd1b_write(void);                                                                     /* 0x400636BC */
void fpga_cmd1c_write(void);                                                                     /* 0x40063714 */
void fpga_cmd18_read(void);                                                                      /* 0x40063770 */
void fpga_position_290_update(void);                                                             /* 0x40063798 */
void fpga_counters_init(void);                                                                   /* 0x40063BC0 */
void fpga_counter_step(u32 ch);                                                                  /* 0x40063C34 */
void fpga_counter_step_all(void);                                                                   /* 0x40063CB0 */
void smec_byte_params_sync(void);                                                                /* 0x40063CD0 */
void adc_lanes_config_for_rate(void);                                                            /* 0x40063F04 */
int lane_trim_search8(u32 row, u32 bank, u32 step);                                              /* 0x4006453C */
int lane_trim_search4x2(u32 row, u32 bank, u32 step);                                            /* 0x400647F4 */
void lane_phase_measure8(int *m);                                                                /* 0x40064E0C */
void lane_phase_measure4x2(int *m1, int *m2);                                                    /* 0x4006529C */

#endif
