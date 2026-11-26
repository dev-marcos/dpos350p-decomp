/*
 * window_buttons_dispatch -- stock 0x40051E88 (520 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Touch dispatch of the windows: id 1..42 (no handler for 4) calls that window's button handler (main menu, settings, lists, numpads, calibration ...), then ui_window_touch_handle(id) for every id.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "ui/input.h"

void window_buttons_dispatch(u32 id)
{
    switch (id) {
    case 1:
        main_menu_buttons();
        break;
    case 2:
        side_menu_buttons();
        break;
    case 3:
        status_bar_buttons();
        break;
    case 5:
        plot_touch_handle();
        break;
    case 6:
        system_settings_buttons();
        break;
    case 7:
        config_menu_buttons();
        break;
    case 8:
        settings_slot_buttons();
        break;
    case 9:
        language_select_buttons();
        break;
    case 10:
        display_menu_buttons();
        break;
    case 11:
        measure_select_buttons();
        break;
    case 12:
        file_lists_open_buttons();
        break;
    case 13:
        channel1_settings_buttons();
        break;
    case 14:
        channel2_settings_buttons();
        break;
    case 15:
        timebase_menu_buttons();
        break;
    case 16:
        output_select_buttons();
        break;
    case 17:
        trigger_option_buttons();
        break;
    case 18:
        gen_panel_buttons();
        break;
    case 19:
        trigger_level_drag_buttons();
        break;
    case 20:
        list_a_thumbs_buttons();
        break;
    case 21:
        list_a_delete_button_handler();
        break;
    case 22:
        screenshot_view_buttons();
        break;
    case 23:
        list_a_window_buttons();
        break;
    case 24:
        list_b_window_buttons();
        break;
    case 25:
        list_b_delete_button_handler();
        break;
    case 26:
        list_c_window_buttons();
        break;
    case 27:
        list_c_delete_button_handler();
        break;
    case 28:
        colour_column_buttons();
        break;
    case 29:
        system_calibration_button_handler();
        break;
    case 30:
        factory_reset_button();
        break;
    case 31:
        format_button_handler();
        break;
    case 32:
        usb_mode_exit_button();
        break;
    case 33:
        numpad_key_buttons();
        break;
    case 34:
        bode_toolbar_buttons();
        break;
    case 35:
        bode_settings_numpad_buttons();
        break;
    case 36:
        bode_cursor_drag_buttons();
        break;
    case 37:
        bode_calibrate_button_handler();
        break;
    case 38:
        levels_toolbar_buttons();
        break;
    case 39:
        measure_numpad_buttons();
        break;
    case 40:
        levels_marker_drag_buttons();
        break;
    case 41:
        levels_options_buttons();
        break;
    case 42:
        calibration_button_handler();
        break;
    default:
        break;
    }
    ui_window_touch_handle(id);
}
