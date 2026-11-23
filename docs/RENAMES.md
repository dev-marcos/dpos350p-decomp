# Renamed functions (readability step 4)

Build 2025-06-19. Old names were mechanical (callee address, first touch-record offset).
Every new name is DERIVED: from the function it calls, from the handler that uses a touch
group, or from the LEVELS / BODES fields a handler writes (see `docs/DRIVERS.md`).

| Address | Old name | New name |
|---|---|---|
| `0x40023E90` | `func_40023ea4_unless_t17` | `plot_markers_draw_gated` |
| `0x40024C4C` | `func_40024c60_unless_t17` | `trace_markers_draw_gated` |
| `0x40024E14` | `func_40023ea4_or_40024c60` | `plot_or_trace_markers_draw` |
| `0x40028350` | `func_40028364_unless_t17` | `time_cursors_draw_gated` |
| `0x4002CC54` | `func_4002c7ac_or_4002c5f8` | `plot_or_xy_grid_draw` |
| `0x4002CC6C` | `sys_rect_redraw_unless_t17` | `sys_rect_redraw_gated` |
| `0x4002D6D4` | `func_4002d82c_flag1` | `number_draw_mode1` |
| `0x4002D6F8` | `func_4002d82c_flag0` | `number_draw_mode0` |
| `0x4002D71C` | `func_4002d2d4_d0` | `number_text_width_mode0` |
| `0x4002EBA8` | `func_4002ecc4_span` | `hspan_fill` |
| `0x4002F324` | `touch_group_0002` | `main_menu_buttons_update` |
| `0x4002F408` | `touch_button_update_00F2` | `plot_area_button_update` |
| `0x4002F410` | `touch_group_017E` | `system_settings_buttons_update` |
| `0x4002F4F0` | `touch_group_0250` | `measure_select_buttons_update` |
| `0x4002F5EC` | `touch_group_0340` | `file_lists_open_buttons_update` |
| `0x4002F614` | `touch_group_035E` | `channel1_settings_buttons_update` |
| `0x4002F684` | `touch_group_03C2` | `channel2_settings_buttons_update` |
| `0x4002F6FC` | `touch_group_0426` | `timebase_menu_buttons_update` |
| `0x4002F86C` | `touch_group_055C` | `output_select_buttons_update` |
| `0x4002F8E0` | `touch_group_05B6` | `trigger_option_buttons_update` |
| `0x4002FB40` | `touch_button_update_07FA` | `list_a_delete_button_update` |
| `0x4002FB50` | `touch_group_080E` | `list_a_window_buttons_update` |
| `0x4002FC30` | `touch_button_update_0912` | `list_b_delete_button_update` |
| `0x4002FCCC` | `touch_button_update_09EE` | `list_c_delete_button_update` |
| `0x4002FD3C` | `touch_group_0124` | `settings_slot_buttons_update` |
| `0x4002FD78` | `touch_button_update_0188` | `system_calibration_button_update` |
| `0x4002FD80` | `touch_group_0156` | `language_select_buttons_update` |
| `0x4002FDB4` | `touch_button_update_01EC` | `factory_reset_button_update` |
| `0x4002FDBC` | `touch_button_update_0200` | `format_button_update` |
| `0x4002FDC4` | `touch_button_update_019C` | `usb_mode_exit_button_update` |
| `0x4002FE3C` | `touch_group_0AC0` | `bode_toolbar_buttons_update` |
| `0x4002FE98` | `touch_group_0B06` | `bode_settings_numpad_buttons_update` |
| `0x4002FEE8` | `touch_button_update_0B42` | `bode_cursor_drag_button_update` |
| `0x4002FEF8` | `touch_group_0B56` | `levels_toolbar_buttons_update` |
| `0x4002FF48` | `touch_group_0B92` | `measure_numpad_buttons_update` |
| `0x4002FFA4` | `touch_button_update_0BD8` | `levels_marker_drag_button_update` |
| `0x4002FFAC` | `touch_group_0BE2` | `levels_options_buttons_update` |
| `0x40030008` | `touch_button_update_0C28` | `calibration_button_update` |
| `0x40034058` | `func_40032e0c_both` | `key_repeat_and_measure_channels` |
| `0x40037A60` | `cursor_ac64_clamp` | `bode_cursors_clamp` |
| `0x40037C28` | `func_40037c3c_unless_t17` | `bode_cursors_draw_gated` |
| `0x4003C550` | `traces_e10c_enabled` | `enabled_trace_layers_clear` |
| `0x4003C790` | `trace_c5cc_full` | `trace_segments_gather_full` |
| `0x4003C7A0` | `trace_c5cc_window` | `trace_segments_gather_window` |
| `0x4003D454` | `ui_cf88_then_579c` | `ui_layout_and_text_init` |
| `0x400525D4` | `touch_00f2_then_400545f8` | `plot_touch_handle` |
| `0x40055B98` | `touch_btn_01ec_release` | `factory_reset_button` |
| `0x40055C78` | `touch_btn_019c_release` | `usb_mode_exit_button` |
| `0x40055F2C` | `buttons_ac0_handler` | `bode_toolbar_buttons` |
| `0x40056080` | `numpad_buttons_b06` | `bode_settings_numpad_buttons` |
| `0x40056214` | `cursor_drag_buttons_b42` | `bode_cursor_drag_buttons` |
| `0x40056404` | `buttons_b56_handler` | `levels_toolbar_buttons` |
| `0x40056724` | `marker_drag_buttons_bd8` | `levels_marker_drag_buttons` |
| `0x40056804` | `buttons_be2_handler` | `levels_options_buttons` |
| `0x4006068C` | `func_400606a0_unless_locked` | `levels_markers_draw_gated` |
| `0x40062E00` | `func_40062e14_unless_locked` | `measure_setup_gated` |
| `0x40063CB0` | `func_40063c34_all8` | `fpga_counter_step_all` |
