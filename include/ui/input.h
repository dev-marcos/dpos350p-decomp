/* Functions of nonmatching/ui/input/ (one file per function, named after it).
 * Addresses are of build 2025-06-19. Names are DERIVED unless the C file says otherwise. */
#ifndef UI_INPUT_H
#define UI_INPUT_H

#include "rt/bits.h"

void ui_colour_buttons_draw(void);                                                               /* 0x40028084 */
void touch_button_set_rect(u32 x, u32 y, u32 wd, u32 ht, volatile unsigned char *b);             /* 0x4002F160 */
void touch_button_disable(volatile unsigned char *b);                                            /* 0x4002F180 */
void ui_window_touch_handle(u32 id);                                                             /* 0x4002F228 */
void touch_button_update(volatile unsigned char *b);                                             /* 0x4002F2BC */
void main_menu_buttons_update(void);                                                                     /* 0x4002F324 */
void side_menu_buttons_update(void);                                                                     /* 0x4002F378 */
void status_bar_buttons_update(void);                                                                     /* 0x4002F3C4 */
void plot_area_button_update(void);                                                             /* 0x4002F408 */
void system_settings_buttons_update(void);                                                                     /* 0x4002F410 */
void display_menu_buttons_update(void);                                                                     /* 0x4002F490 */
void measure_select_buttons_update(void);                                                                     /* 0x4002F4F0 */
void file_lists_open_buttons_update(void);                                                                     /* 0x4002F5EC */
void channel1_settings_buttons_update(void);                                                                     /* 0x4002F614 */
void channel2_settings_buttons_update(void);                                                                     /* 0x4002F684 */
void timebase_menu_buttons_update(void);                                                                     /* 0x4002F6FC */
void output_select_buttons_update(void);                                                                     /* 0x4002F86C */
void trigger_option_buttons_update(void);                                                                     /* 0x4002F8E0 */
void touch_buttons_6xx_update(void);                                                             /* 0x4002F968 */
void trigger_level_drag_buttons_update(void);                                                             /* 0x4002FAA4 */
void list_a_thumbs_buttons_update(void);                                                             /* 0x4002FAAC */
void list_a_delete_button_update(void);                                                             /* 0x4002FB40 */
void screenshot_view_buttons_update(void);                                                             /* 0x4002FB48 */
void list_a_window_buttons_update(void);                                                                     /* 0x4002FB50 */
void list_b_window_buttons_update(void);                                                             /* 0x4002FB98 */
void list_b_delete_button_update(void);                                                             /* 0x4002FC30 */
void list_c_window_buttons_update(void);                                                             /* 0x4002FC38 */
void list_c_delete_button_update(void);                                                             /* 0x4002FCCC */
void colour_column_buttons_update(void);                                                                     /* 0x4002FCD4 */
void config_menu_buttons_update(void);                                                                     /* 0x4002FD10 */
void settings_slot_buttons_update(void);                                                                     /* 0x4002FD3C */
void system_calibration_button_update(void);                                                             /* 0x4002FD78 */
void language_select_buttons_update(void);                                                                     /* 0x4002FD80 */
void factory_reset_button_update(void);                                                             /* 0x4002FDB4 */
void format_button_update(void);                                                             /* 0x4002FDBC */
void usb_mode_exit_button_update(void);                                                             /* 0x4002FDC4 */
void numpad_key_buttons_update(void);                                                             /* 0x4002FDCC */
void bode_toolbar_buttons_update(void);                                                                     /* 0x4002FE3C */
void bode_settings_numpad_buttons_update(void);                                                                     /* 0x4002FE98 */
void bode_cursor_drag_button_update(void);                                                             /* 0x4002FEE8 */
void bode_calibrate_button_update(void);                                                             /* 0x4002FEF0 */
void levels_toolbar_buttons_update(void);                                                                     /* 0x4002FEF8 */
void measure_numpad_buttons_update(void);                                                                     /* 0x4002FF48 */
void levels_marker_drag_button_update(void);                                                             /* 0x4002FFA4 */
void levels_options_buttons_update(void);                                                                     /* 0x4002FFAC */
void calibration_button_update(void);                                                             /* 0x40030008 */
void key_repeat_update(void);                                                                    /* 0x40032D3C */
void touch_state_clear(void);                                                                    /* 0x40049A38 */
void touch_init(void);                                                                           /* 0x40049A74 */
void touch_service(void);                                                                        /* 0x40049A9C */
void disk_format_and_reload(void);                                                               /* 0x40051C04 */
void screenshot_button_poll(void);                                                               /* 0x40051E24 */
void window_buttons_dispatch(u32 id);                                                            /* 0x40051E88 */
void window_touch_poll(void);                                                                    /* 0x40052098 */
void main_menu_buttons(void);                                                                    /* 0x40052100 */
void side_menu_buttons(void);                                                                  /* 0x40052280 */
void status_bar_buttons(void);                                                              /* 0x400524B4 */
void plot_touch_handle(void);                                                             /* 0x400525D4 */
void system_settings_buttons(void);                                                              /* 0x400525EC */
void display_menu_buttons(void);                                                                  /* 0x400527C0 */
void measure_select_buttons(void);                                                               /* 0x400529F0 */
void file_lists_open_buttons(void);                                                              /* 0x40052F08 */
void channel1_settings_buttons(void);                                                            /* 0x40052FC4 */
void channel2_settings_buttons(void);                                                            /* 0x400531B0 */
void timebase_menu_buttons(void);                                                                /* 0x400533A0 */
void output_select_buttons(void);                                                                /* 0x400537C4 */
void trigger_option_buttons(void);                                                               /* 0x40053940 */
void gen_panel_buttons(void);                                                                    /* 0x40053B78 */
void trigger_level_drag_buttons(void);                                                              /* 0x40054540 */
void plot_touch_drag(void);                                                                      /* 0x400545F8 */
void list_a_thumbs_buttons(void);                                                                /* 0x40054C84 */
void list_a_delete_button_handler(void);                                                         /* 0x40054EB8 */
void screenshot_view_buttons(void);                                                               /* 0x40054F08 */
void list_a_window_buttons(void);                                                                /* 0x40054F50 */
void list_b_window_buttons(void);                                                                /* 0x40055114 */
void list_b_delete_button_handler(void);                                                         /* 0x40055344 */
void list_c_window_buttons(void);                                                                /* 0x400553A0 */
void list_c_delete_button_handler(void);                                                         /* 0x400555E8 */
void colour_column_buttons(void);                                                                  /* 0x40055634 */
void config_menu_buttons(void);                                                                  /* 0x40055720 */
void settings_slot_buttons(void);                                                                /* 0x40055798 */
void system_calibration_button_handler(void);                                                    /* 0x40055978 */
void language_select_buttons(void);                                                              /* 0x40055AD4 */
void factory_reset_button(void);                                                               /* 0x40055B98 */
void format_button_handler(void);                                                                /* 0x40055BD4 */
void usb_mode_exit_button(void);                                                               /* 0x40055C78 */
void numpad_key_buttons(void);                                                                   /* 0x40055CC4 */
void bode_toolbar_buttons(void);                                                                  /* 0x40055F2C */
void bode_settings_numpad_buttons(void);                                                                   /* 0x40056080 */
void bode_cursor_drag_buttons(void);                                                              /* 0x40056214 */
void bode_calibrate_button_handler(void);                                                       /* 0x400562F4 */
void levels_toolbar_buttons(void);                                                                  /* 0x40056404 */
void measure_numpad_buttons(void);                                                               /* 0x40056530 */
void levels_marker_drag_buttons(void);                                                              /* 0x40056724 */
void levels_options_buttons(void);                                                                  /* 0x40056804 */
void calibration_button_handler(void);                                                           /* 0x40056974 */
void numpad_open_mode1(u32 x, u32 y);                                                            /* 0x4005F9F0 */
void numpad_open_mode3(u32 x, u32 y);                                                            /* 0x4005FA34 */
void numpad_open_mode4(u32 x, u32 y);                                                            /* 0x4005FA78 */
void numpad_open_mode5(u32 x, u32 y);                                                            /* 0x4005FABC */
void numpad_open_mode6(u32 x, u32 y);                                                            /* 0x4005FB00 */
void numpad_close(u32 r0, u32 r1, u32 r2);                                                       /* 0x4005FB44 */
void numpad_reset(void);                                                                         /* 0x4005FB4C */
void numpad_append_char(u32 c);                                                                  /* 0x4005FB94 */
void numpad_backspace(void);                                                                     /* 0x4005FBB4 */
void numpad_sign_apply(void);                                                                    /* 0x4005FC04 */
void numpad_value_parse(u32 unit);                                                               /* 0x4005FC84 */
void numpad_key(u32 k);                                                                          /* 0x4005FE94 */
void levels_markers_draw_gated(void);                                                          /* 0x4006068C */
void measure_setup_gated(void);                                                          /* 0x40062E00 */
void touch_moved_flag(void);                                                                     /* 0x4006904C */
void touch_drag_delta_b(void);                                                                   /* 0x40069084 */
void touch_hit_pair_a(void);                                                                     /* 0x400690E4 */
void touch_pick_a(void);                                                                         /* 0x40069150 */
void touch_drag_delta_c(void);                                                                   /* 0x400691D4 */
void touch_hit_pair_b(void);                                                                     /* 0x40069234 */
void touch_pick_b(void);                                                                         /* 0x400692A0 */
void touch_drag_delta_d(void);                                                                   /* 0x40069324 */
void touch_hit_cursor_x(void);                                                                   /* 0x40069384 */
void touch_hit_cursor_y(void);                                                                   /* 0x40069428 */
void touch_hit_x_b(void);                                                                        /* 0x40069524 */
void trace_hit_main(u32 ch);                                                                     /* 0x400695D0 */
void trace_hit_zoom(u32 ch);                                                                     /* 0x40069858 */
void trace_hit_ch1(void);                                                                        /* 0x40069AEC */
void trace_hit_ch2(void);                                                                        /* 0x40069AF4 */
void trace_hit_zoom_ch1(void);                                                                   /* 0x40069AFC */
void trace_hit_zoom_ch2(void);                                                                   /* 0x40069B04 */
void touch_drag_pick(void);                                                                      /* 0x40069B0C */
void touch_drag_delta(void);                                                                     /* 0x40069CF8 */

#endif
