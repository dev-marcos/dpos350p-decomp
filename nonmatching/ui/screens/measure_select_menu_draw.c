/*
 * measure_select_menu_draw -- stock 0x4001D608 (4804 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Draws the measurement selection menu (layout +0xA0..+0xA6): for CH1 (label 'CH1 parameter measurement', text entry 23, at y + 0x14) and CH2 (entry 24, at y + 0xFA) an image 0x40089EC8 and twelve 0x50 x 0x28 buttons in two rows of six, 0x55 apart (rows at y + 0x50 / 0xA5 and y + 0x136 / 0x18B): 'Vmax', 'Vmin', 'Vavg', 'Vrms', 'Vp', 'Vpp', 'F', 'T', 'T+', 'T-', 'Du+', 'Du-' (the last two are centred by the width of 'Dt+' / 'Dt-'). Measurement k (1 .. 12) of channel c is shown as selected (theme colour +0x10, black text) when value_in_table_405456e8(c, k) is set, i.e. when k (CH1) or -k (CH2) is in the list of 24 shown measurements at 0x405456E8. Touch records 0x42E61FF8 + 0x250 + 0xA x (12 c + k - 1); a pressed button is shaded with theme +8.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "ui/draw.h"
#include "ui/input.h"
#include "ui/screens.h"
#include "util.h"
#include "state/ui_windows.h"
#include "shared/access.h"

#define SYS 0x40544E68u
#define TH (SYS + 0x3DD550)          /* theme colours: fill, text, pressed, ... */
#define TXT 0x42C2FE74u              /* text table, 0x1A0 bytes per entry */
#define BTN 0x42E61FF8u              /* touch button records */
#define ENT(n) (TXT + 0x1A0 * (n))

/* the stock code measures "Dt+" / "Dt-" but draws "Du+" / "Du-" (the centring uses the other text) */
static const char ms_text[12][5] = { "Vmax", "Vmin", "Vavg", "Vrms", "Vp", "Vpp", "F", "T", "T+", "T-", "Dt+", "Dt-" };
static const char ms_draw[2][4] = { "Du+", "Du-" };

static void measure_rows(u32 x0, u32 r, u32 c)
{
    u32 txt, tw, xo, rec;
    int k;

    for (k = 0; k < 12; k++) {
        if (k == 6)
            r += 0x55;
        xo = (x0 + 0x55 * (k % 6)) & 0xFFFF;
        rec = 0x250 + 0xA * (12 * c + k);
        if (value_in_table_405456e8(c, k + 1)) {
            draw_colour_set(W32(TH + 0x10));
            txt = 0;
        } else {
            draw_colour_set(W32(TH + 0xC));
            txt = W32(TH + 0x14);
        }
        rect_fill(xo, r & 0xFFFF, 0x50, 0x28);
        draw_colour_set(txt);
        tw = text_width(ms_text[k]);
        text_draw((xo + ((0x50 - tw) >> 1)) & 0xFFFF, (r + 0xC) & 0xFFFF, k < 10 ? ms_text[k] : ms_draw[k - 10]);
        touch_button_set_rect(xo, (r - 0xF) & 0xFFFF, 0x50, 0x46, (volatile unsigned char *)(BTN + rec));
        if (B8(BTN + rec + 1)) {
            draw_colour_set(W32(TH + 8));
            rect_fill(xo, r & 0xFFFF, 0x50, 0x28);
        }
    }
}

static void header(u32 x, u32 y, u32 e)
{
    draw_colour_set(W32(TH + 4));
    draw_text_shadowed((x + 0x14) & 0xFFFF, y & 0xFFFF, 0x40089EC8u, 2, 2, 0);
    draw_colour_set(W32(TH + 4));
    text_shadow_draw((x + SB8(e + 0x194) + 0x41) & 0xFFFF, (y + SB8(e + 0x195) + 3) & 0xFFFF, (const char *)W32(e), 1, 2, 0);
}

void measure_select_menu_draw(void)
{
    u32 x = WREC(11, x), y = WREC(11, y), w = WREC(11, w), h = WREC(11, h);

    word_42e68c74_set(0x42E658D8u);
    half_42e68be2_set(1);
    draw_colour_set(W32(TH));
    rect_fill(x & 0xFFFF, y, w, h);
    header(x, y + 0x14, ENT(23));
    half_42e68be2_set(2);
    measure_rows(x + 0x14, y + 0x50, 0);
    half_42e68be2_set(1);
    header(x, y + 0xFA, ENT(24));
    measure_rows(x + 0x14, y + 0x136, 1);
}
