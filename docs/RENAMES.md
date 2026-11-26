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

## Step 6a

Names that carried an address or an offset. Each new name is DERIVED from what the code
already says: the file list (A screenshots, B waveforms, C captures) a helper serves, the named
SYS_INFO field a setter or clamp writes, the callee of a veneer, or the doc that describes the
state (`docs/DRIVERS.md`, "Current font and state"; `include/state/`).

| Address | Old name | New name |
|---|---|---|
| `0x40019AF4` | `colour_table_3dd530_init` | `theme_colours_init` |
| `0x40024E38` | `ui_delete_confirm_12c` | `list_a_delete_confirm_draw` |
| `0x400262F8` | `ui_delete_confirm_164` | `list_b_delete_confirm_draw` |
| `0x400271B0` | `ui_delete_confirm_180` | `list_c_delete_confirm_draw` |
| `0x4002903C` | `ui_unplug_notice_19c` | `system_calibration_notice_draw` |
| `0x4002C32C` | `ui_unplug_notice_252` | `calibration_notice_draw` |
| `0x4002CCCC` | `word_42e68c74_set` | `font_set` |
| `0x4002D724` | `text_d6f8_shadow_draw` | `number_mode0_shadow_draw` |
| `0x4002DC40` | `half_42e68be2_set` | `text_spacing_set` |
| `0x4002DC74` | `byte_42e68be0_set` | `text_centring_set` |
| `0x400301A8` | `value_in_table_405456e8` | `marker_value_present` |
| `0x4003407C` | `cursor_808_clamp` | `time_cursors_clamp` |
| `0x400340D4` | `cursor_810_clamp` | `level_cursors_clamp` |
| `0x40034284` | `rect_42e68c38_draw` | `message_box_erase` |
| `0x400342FC` | `fb_rect_copy_to_e4e84` | `fb_rect_save` |
| `0x4003436C` | `fb_rect_copy_from_e4e84` | `fb_rect_restore` |
| `0x400349B4` | `list_fa4_insert_754` | `list_a_insert` |
| `0x40035248` | `list_cursor_clamp_40035248` | `list_a_cursor_clamp` |
| `0x400353A8` | `list_count_nonzero_400353a8` | `list_a_marks_count` |
| `0x400353F4` | `file_exists_40034b78` | `screenshot_file_exists` |
| `0x40035744` | `reload_chain_40035744` | `list_b_reload` |
| `0x40035A00` | `list_fa4_insert_756` | `list_b_insert` |
| `0x400362F4` | `list_cursor_clamp_400362f4` | `list_b_cursor_clamp` |
| `0x40036454` | `list_count_nonzero_40036454` | `list_b_marks_count` |
| `0x400364A0` | `file_exists_40035bc4` | `waveform_file_exists` |
| `0x40036D88` | `list_fa4_insert_758` | `list_c_insert` |
| `0x400373E0` | `reload_chain_400373e0` | `list_c_save_new` |
| `0x4003771C` | `list_cursor_clamp_4003771c` | `list_c_cursor_clamp` |
| `0x40037880` | `list_count_nonzero_40037880` | `list_c_marks_count` |
| `0x400378E0` | `reload_chain_400378e0` | `list_c_reload` |
| `0x4003BE98` | `sys_word_pair_570_set` | `position_ref_set` |
| `0x4003CA38` | `table_scatter_42cf64b4` | `glyph14_table_build` |
| `0x4003ED28` | `state_block_3f984_init` | `generator_state_init` |
| `0x4003ED80` | `state_block_180c8_init` | `bode_state_init` |
| `0x4003EDF4` | `state_block_68668_init` | `levels_state_init` |
| `0x4003FAB0` | `veneer_400636bc` | `fpga_cmd1b_write_veneer` |
| `0x4003FAB4` | `veneer_40063714` | `fpga_cmd1c_write_veneer` |
| `0x4003FAE4` | `veneer_40063798` | `fpga_position_update_veneer` |
| `0x40040030` | `timebase_apply_2b4` | `trace_pointers_2b4` |
| `0x4004003C` | `timebase_apply_2b0` | `trace_pointers_block_points` |
| `0x40040048` | `timebase_apply_2b0_x1_3` | `trace_pointers_block_points_x1_3` |
| `0x400424C4` | `sys_word_pair_578_set` | `view_position_set` |
| `0x40045688` | `settings_block_448b4_decode` | `settings_image_fields_decode` |
| `0x4004F59C` | `fpga_probe_and_copy_7fc` | `fpga_probe_and_record` |
| `0x400562F4` | `calibration_b4c_button_handler` | `bode_calibrate_button_handler` |
| `0x40063484` | `cursor_27e_clamp` | `trigger_level_clamp` |
| `0x40063798` | `fpga_position_290_update` | `fpga_position_update` |

## Step 6b

Window handlers and their updaters are named after the draw function of their window
(`window_buttons_dispatch` id -> `ui_window_NN_redraw` -> draw function). The factory
self-tests are named after the firmware's own step labels (`docs/DRIVERS.md`, "Factory test
flow"); the rest after the LEVELS and SYS_INFO fields they write. Every name DERIVED.

| Address | Old name | New name |
|---|---|---|
| `0x40023D04` | `window_frame_23d04_draw` | `trigger_level_drag_draw` |
| `0x40025D08` | `image_42468a58_show` | `screenshot_view_draw` |
| `0x4002F378` | `touch_group_005C` | `side_menu_buttons_update` |
| `0x4002F3C4` | `touch_group_00AC` | `status_bar_buttons_update` |
| `0x4002F490` | `touch_group_020A` | `display_menu_buttons_update` |
| `0x4002FAA4` | `touch_button_update_071E` | `trigger_level_drag_buttons_update` |
| `0x4002FAAC` | `touch_buttons_728_update` | `list_a_thumbs_buttons_update` |
| `0x4002FB48` | `touch_button_update_0804` | `screenshot_view_buttons_update` |
| `0x4002FB98` | `touch_buttons_840_update` | `list_b_window_buttons_update` |
| `0x4002FC38` | `touch_buttons_91c_update` | `list_c_window_buttons_update` |
| `0x4002FCD4` | `touch_group_09F8` | `colour_column_buttons_update` |
| `0x4002FD10` | `touch_group_0106` | `config_menu_buttons_update` |
| `0x4002FDCC` | `touch_buttons_a20_update` | `numpad_key_buttons_update` |
| `0x4002FEF0` | `touch_button_update_0B4C` | `bode_calibrate_button_update` |
| `0x40031710` | `colour_toggle_42e68d1a` | `row_shade_toggle` |
| `0x40035780` | `file_write_360404` | `waveform_file_write` |
| `0x400367EC` | `cursor_3da534_clamp` | `cursor_pair_clamp` |
| `0x40037160` | `number_text_88c0` | `path_buf_number_write` |
| `0x400374EC` | `array_3da548_decimate` | `capture_samples_decimate` |
| `0x4004F5E4` | `selftest_4f5e4` | `adc_lanes_selftest` |
| `0x4004F89C` | `gen_selftest_4f89c` | `channel_mux_selftest` |
| `0x400504B8` | `gen_selftest_504b8` | `coupling_dcac_selftest` |
| `0x40050688` | `gen_selftest_50688` | `channel_ranges_selftest` |
| `0x40050898` | `gen_selftest_50898` | `flatness_selftest` |
| `0x40050BAC` | `gen_selftest_50bac` | `bandwidth_150m_selftest` |
| `0x40050DDC` | `gen_selftest_50ddc` | `bandwidth_20m_selftest` |
| `0x40052280` | `buttons_05c_handler` | `side_menu_buttons` |
| `0x400524B4` | `buttons_ac_open_windows` | `status_bar_buttons` |
| `0x400527C0` | `buttons_20a_handler` | `display_menu_buttons` |
| `0x40054540` | `button_71e_drag_handler` | `trigger_level_drag_buttons` |
| `0x40054F08` | `touch_btn_0804_release` | `screenshot_view_buttons` |
| `0x40055634` | `buttons_9f8_handler` | `colour_column_buttons` |
| `0x40055720` | `buttons_106_handler` | `config_menu_buttons` |
| `0x400621EC` | `level_stats_5f010` | `levels_plot_stats` |
| `0x40062284` | `level_stats_60008` | `levels_panel_a_stats` |
| `0x4006233C` | `level_stats_61000` | `levels_panel_b_stats` |
| `0x40062E14` | `measure_setup_62e14` | `measure_setup` |
| `0x40064C28` | `gen_selftest_64c28` | `adc_phase_trim8_check` |
| `0x40064CF0` | `gen_selftest_64cf0` | `adc_phase_trim4x2_check` |
