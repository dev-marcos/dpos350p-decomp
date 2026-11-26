/* Functions of nonmatching/processing/ (one file per function, named after it).
 * Addresses are of build 2025-06-19. Names are DERIVED unless the C file says otherwise. */
#ifndef PROCESSING_H
#define PROCESSING_H

#include "rt/bits.h"

void marker_value_add(u32 mode, int v);                                                          /* 0x40030060 */
void value_si_scale(u32 kind, u32 unused, long long v, long long *out, int *dec, int *prefix);   /* 0x400301E0 */
void channel_minmax_stats(u32 ch);                                                               /* 0x40032A58 */
void channel_freq_measure(u32 ch);                                                               /* 0x40032B5C */
void measure_channel(u32 ch_arg);                                                                /* 0x40032E0C */
void time_cursors_clamp(void);                                                                     /* 0x4003407C */
void level_cursors_clamp(void);                                                                     /* 0x400340D4 */
void capture_samples_decimate(int n);                                                               /* 0x400374EC */
void trace_cursor_limits(void);                                                                  /* 0x40037944 */
void bode_cursors_clamp(void);                                                                    /* 0x40037A60 */
void trace_update_chain(void);                                                                   /* 0x40037AB8 */
void trace_defaults_37b48(void);                                                                 /* 0x40037B48 */
void trace_cursor_values(void);                                                                  /* 0x400381E8 */
void trace_view_bounds(u32 tr);                                                                  /* 0x4003BB6C */
void trace_shift_samples(u32 tr);                                                                /* 0x4003C158 */
void trace_zoom_segments(u32 tr);                                                                /* 0x4003C314 */
void enabled_trace_layers_clear(void);                                                                  /* 0x4003C550 */
void trace_segments_gather(u32 tr, u32 start, u32 n);                                            /* 0x4003C5CC */
void trace_segments_gather_full(u32 tr);                                                                    /* 0x4003C790 */
void trace_segments_gather_window(u32 tr);                                                                  /* 0x4003C7A0 */
void trace_process_full(u32 tr);                                                                 /* 0x4003C7E4 */
void channel_traces_process_full(u32 ch);                                                        /* 0x4003C85C */
void channels_process_full(void);                                                                /* 0x4003C8D0 */
void channels_process_full_ordered(volatile unsigned char *p);                                   /* 0x4003C8D4 */
void traces_process_single(void);                                                                /* 0x4003C910 */
void traces_update(void);                                                                        /* 0x4003C970 */
void trace_process_window(u32 tr);                                                               /* 0x4003C9BC */
void per_channel_processing(void);                                                               /* 0x4003C9F4 */
void glyph14_table_build(void);                                                               /* 0x4003CA38 */
void amplitude_statistical_normalization(u32 ch);                                                /* 0x4003FD10 */
void final_per_channel_normalization(u32 ch);                                                    /* 0x4003FE14 */
void channel_post_process(u32 ch);                                                               /* 0x4004006C */
void moving_average_and_crossing_alignment_search(void);                                         /* 0x40040114 */
void channel_moving_average(u32 ch);                                                             /* 0x40040F18 */
void channel_step_limiter(u32 ch);                                                               /* 0x40040FC4 */
void channel_to_display_copy(u32 ch, u32 k);                                                     /* 0x40041094 */
void frame_deviation_blend(u32 ch, u32 n, float fr);                                             /* 0x40041118 */
void channel_filters_by_timebase(void);                                                          /* 0x40041324 */
void fft_input_prepare(void);                                                                    /* 0x40041A4C */
void fft_hann_spectrum(void);                                                                    /* 0x40041BA0 */
void fft_db_traces_scale(void);                                                                  /* 0x40041D38 */
void zoom_views_update(void);                                                                    /* 0x40041F5C */
void zoom_entry_select(void);                                                                    /* 0x40041FF8 */
void view_bounds_update(void);                                                                   /* 0x40042280 */
u32 zoom_mode_check(void);                                                                       /* 0x40042350 */
void zoom_index_fit(void);                                                                       /* 0x400423B8 */
void zoom_index_add(u32 d);                                                                      /* 0x4004245C */
void zoom_index_sub(u32 d);                                                                      /* 0x40042470 */
void zoom_index_set(u32 v);                                                                      /* 0x400424B8 */
void zoom_reset(void);                                                                           /* 0x400424D4 */
u32 channel_level_mean(u32 start, u32 ch, u32 n, unsigned char *buf);                            /* 0x4004ADF0 */
void simple_integer_index_mapping_nearest_neighbour(u32 src_a, u32 center_a, u32 w, u32 m, u32 shift_a, u32 unused); /* 0x4005C678 */
void workspace_copy_and_resampling(u32 ch, u32 src, u32 half, u32 len, u32 n);                   /* 0x4005C7BC */
void spline_coeffs(int n, int end1, int end2, const volatile float *x, const volatile float *y, volatile float *b, volatile float *c, volatile float *d, volatile u32 *iflag, float slope1, float slope2); /* 0x4005C95C */
float spline_eval(u32 unused, int n, const volatile float *xk, const volatile float *a, const volatile float *b, const volatile float *c, const volatile float *d, volatile int *idx, float x); /* 0x4005CEA4 */
void spline_resample(int n, const volatile float *xk, const volatile float *yk, u32 m, const volatile float *xin, volatile float *yout); /* 0x4005CF78 */
void floating_point_interpolation_resampling(u32 ws, u32 half, u32 len, u32 m, long long shift); /* 0x4005D048 */
void fixed_point_weighted_or_filtered_resampling(u32 ws, u32 half, u32 len, u32 m, long long shift); /* 0x4005D2E4 */
void trace_intensity_accumulate_lines(u32 tr);                                                   /* 0x4005E12C */
void trace_intensity_accumulate_window(u32 tr);                                                  /* 0x4005E838 */
void trace_intensity_accumulate(u32 tr);                                                         /* 0x4005EDC0 */
void trace_span_full(u32 tr);                                                                    /* 0x4005F8D8 */
void trace_span_window(u32 tr);                                                                  /* 0x4005F8E8 */
void trace_span_full_veneer(u32 tr);                                                             /* 0x4005F90C */
void trace_span_window_veneer(u32 tr);                                                           /* 0x4005F910 */
void marker_values_compute(void);                                                                /* 0x4006053C */
void fft_spur_capture(void);                                                                     /* 0x40060BA4 */
void fft_spur_cancel(void);                                                                      /* 0x40060DC8 */
void auto_level_search(void);                                                                    /* 0x40060FE8 */
void levels_view_layout(void);                                                                   /* 0x400612FC */
void fft_input_prepare_b(void);                                                                  /* 0x40061580 */
void peak_hold_decimate(volatile short *buf, u32 n, double step);                                /* 0x40061608 */
void measure_values_clamp(void);                                                                 /* 0x400617AC */
void levels_samples_window(void);                                                                /* 0x40061A70 */
void fft_levels_db(void);                                                                        /* 0x40061DB0 */
void levels_plot_stats(void);                                                                    /* 0x400621EC */
void levels_panel_a_stats(void);                                                                    /* 0x40062284 */
void levels_panel_b_stats(void);                                                                    /* 0x4006233C */
void levels_update_chain(void);                                                                  /* 0x400623F4 */
void measure_setup(void);                                                                  /* 0x40062E14 */
void trigger_level_clamp(void);                                                                     /* 0x40063484 */
void cvec_magnitude(const volatile float *src, volatile float *dst, int n);                      /* 0x40066538 */
void fft_radix2(int n, volatile float *x);                                                       /* 0x400665A8 */

#endif
