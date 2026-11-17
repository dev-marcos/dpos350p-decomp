/*
 * window_buttons_dispatch -- stock 0x40051E88 (520 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Touch dispatch of the windows: id 1..42 (no handler for 4) calls that window's button handler (main menu, settings, lists, numpads, calibration ...), then ui_window_touch_handle(id) for every id.
 */
#include "hw/platform.h"

#include "hw/fpga.h"

void main_menu_buttons(void);
void buttons_05c_handler(void);
void buttons_ac_open_windows(void);
void touch_00f2_then_400545f8(void);
void system_settings_buttons(void);
void buttons_106_handler(void);
void settings_slot_buttons(void);
void language_select_buttons(void);
void buttons_20a_handler(void);
void func_400529f0(void);
void file_lists_open_buttons(void);
void channel1_settings_buttons(void);
void channel2_settings_buttons(void);
void func_400533a0(void);
void output_select_buttons(void);
void trigger_option_buttons(void);
void func_40053b78(void);
void button_71e_drag_handler(void);
void list_a_thumbs_buttons(void);
void list_a_delete_button_handler(void);
void touch_btn_0804_release(void);
void list_a_window_buttons(void);
void list_b_window_buttons(void);
void list_b_delete_button_handler(void);
void list_c_window_buttons(void);
void list_c_delete_button_handler(void);
void buttons_9f8_handler(void);
void system_calibration_button_handler(void);
void touch_btn_01ec_release(void);
void format_button_handler(void);
void touch_btn_019c_release(void);
void numpad_key_buttons(void);
void buttons_ac0_handler(void);
void numpad_buttons_b06(void);
void cursor_drag_buttons_b42(void);
void calibration_b4c_button_handler(void);
void buttons_b56_handler(void);
void measure_numpad_buttons(void);
void marker_drag_buttons_bd8(void);
void buttons_be2_handler(void);
void calibration_button_handler(void);
void ui_window_touch_handle(u32 id);

void window_buttons_dispatch(u32 id)
{
    switch (id) {
    case 1:
        main_menu_buttons();
        break;
    case 2:
        buttons_05c_handler();
        break;
    case 3:
        buttons_ac_open_windows();
        break;
    case 5:
        touch_00f2_then_400545f8();
        break;
    case 6:
        system_settings_buttons();
        break;
    case 7:
        buttons_106_handler();
        break;
    case 8:
        settings_slot_buttons();
        break;
    case 9:
        language_select_buttons();
        break;
    case 10:
        buttons_20a_handler();
        break;
    case 11:
        func_400529f0();
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
        func_400533a0();
        break;
    case 16:
        output_select_buttons();
        break;
    case 17:
        trigger_option_buttons();
        break;
    case 18:
        func_40053b78();
        break;
    case 19:
        button_71e_drag_handler();
        break;
    case 20:
        list_a_thumbs_buttons();
        break;
    case 21:
        list_a_delete_button_handler();
        break;
    case 22:
        touch_btn_0804_release();
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
        buttons_9f8_handler();
        break;
    case 29:
        system_calibration_button_handler();
        break;
    case 30:
        touch_btn_01ec_release();
        break;
    case 31:
        format_button_handler();
        break;
    case 32:
        touch_btn_019c_release();
        break;
    case 33:
        numpad_key_buttons();
        break;
    case 34:
        buttons_ac0_handler();
        break;
    case 35:
        numpad_buttons_b06();
        break;
    case 36:
        cursor_drag_buttons_b42();
        break;
    case 37:
        calibration_b4c_button_handler();
        break;
    case 38:
        buttons_b56_handler();
        break;
    case 39:
        measure_numpad_buttons();
        break;
    case 40:
        marker_drag_buttons_bd8();
        break;
    case 41:
        buttons_be2_handler();
        break;
    case 42:
        calibration_button_handler();
        break;
    default:
        break;
    }
    ui_window_touch_handle(id);
}
