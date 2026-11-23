/* Functions of nonmatching/ui/screens/ (one file per function, named after it).
 * Addresses are of build 2025-06-19. Names are DERIVED unless the C file says otherwise. */
#ifndef UI_SCREENS_H
#define UI_SCREENS_H

#include "rt/bits.h"

void main_toolbar_draw(void);                                                                    /* 0x40019DB8 */
void side_menu_column_draw(void);                                                                /* 0x4001A708 */
void status_bar_draw(void);                                                                      /* 0x4001AD50 */
void system_menu_draw(void);                                                                     /* 0x4001C3BC */
void display_menu_draw(void);                                                                    /* 0x4001CE30 */
void measure_select_menu_draw(void);                                                             /* 0x4001D608 */
void menu_three_rows_draw(void);                                                                 /* 0x4001E8E0 */
void ch1_menu_draw(void);                                                                        /* 0x4001EBE4 */
void ch2_menu_draw(void);                                                                        /* 0x4001F5A8 */
void timebase_menu_draw(void);                                                                   /* 0x4001FF74 */
void hires_menu_draw(void);                                                                      /* 0x40021630 */
void trigger_menu_draw(void);                                                                    /* 0x40021E68 */
void gen_screen_draw(void);                                                                      /* 0x40022958 */
void plot_markers_draw_gated(void);                                                             /* 0x40023E90 */
void plot_markers_draw(void);                                                                    /* 0x40023EA4 */
void trace_markers_draw_gated(void);                                                             /* 0x40024C4C */
void trace_markers_draw(void);                                                                   /* 0x40024C60 */
void plot_or_trace_markers_draw(void);                                                            /* 0x40024E14 */
void ui_delete_confirm_12c(void);                                                                /* 0x40024E38 */
void screenshot_browser_draw(void);                                                              /* 0x400250DC */
void screenshot_view_frame_draw(void);                                                           /* 0x40025DB4 */
void ui_delete_confirm_164(void);                                                                /* 0x400262F8 */
void waveform_browser_draw(void);                                                                /* 0x400265BC */
void ui_delete_confirm_180(void);                                                                /* 0x400271B0 */
void record_browser_draw(void);                                                                  /* 0x40027498 */
void time_cursors_draw_gated(void);                                                             /* 0x40028350 */
void time_cursors_draw(void);                                                                    /* 0x40028364 */
void config_menu_draw(void);                                                                     /* 0x40028740 */
void config_slot_menu_draw(void);                                                                /* 0x40028AB4 */
void ui_unplug_notice_19c(void);                                                                 /* 0x4002903C */
void ui_language_menu_draw(void);                                                                /* 0x400292E4 */
void ui_factory_reset_confirm(void);                                                             /* 0x4002967C */
void ui_clear_data_warning(void);                                                                /* 0x40029920 */
void ui_usb_connect_screen(void);                                                                /* 0x40029BC0 */
void ui_connect_generator_notice(void);                                                          /* 0x40029EC0 */
void bode_toolbar_draw(void);                                                                    /* 0x4002A184 */
void bode_sweep_bar_draw(void);                                                                  /* 0x4002A7F4 */
void levels_toolbar_draw(void);                                                                  /* 0x4002AE60 */
void levels_param_bar_draw(void);                                                                /* 0x4002B43C */
void fft_settings_menu_draw(void);                                                               /* 0x4002BC40 */
void ui_unplug_notice_252(void);                                                                 /* 0x4002C32C */
void xy_grid_draw(void);                                                                         /* 0x4002C5F8 */
void plot_grid_draw(void);                                                                       /* 0x4002C7AC */
void plot_or_xy_grid_draw(void);                                                            /* 0x4002CC54 */
void measure_item_draw(u32 x, u32 y, u32 ch, u32 kind);                                          /* 0x40030AF8 */
void measure_panel_draw(void);                                                                   /* 0x40030F8C */
void manual_cursors_draw(void);                                                                  /* 0x4003117C */
void cursor_panel_draw(void);                                                                    /* 0x40031794 */
void key_repeat_and_measure_channels(void);                                                                   /* 0x40034058 */
void message_box_draw(int x, int y, u32 text, int align);                                        /* 0x40034134 */
void bode_cursors_draw_gated(void);                                                             /* 0x40037C28 */
void bode_cursors_draw(void);                                                                    /* 0x40037C3C */
void ui_layout_and_text_init(void);                                                                    /* 0x4003D454 */
void zoom_ratio_window_update(void);                                                             /* 0x4004201C */
void split_window_marker_draw(void);                                                             /* 0x40042520 */
void numpad_draw(void);                                                                          /* 0x4005FF88 */
void levels_markers_draw(void);                                                                  /* 0x400606A0 */
void levels_graph_draw(void);                                                                    /* 0x40062448 */
void levels_max_panel_draw(void);                                                                /* 0x40062A18 */
void levels_shade_draw(void);                                                                    /* 0x40062B1C */

#endif
