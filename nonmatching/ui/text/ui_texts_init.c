/*
 * ui_texts_init -- stock 0x40045974 (13584 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Fills the UI text table: for each of the 141 entries (in stock order, 101 before 98..100) ui_text_set_lang1..4(k, s, a, b) with the Chinese, English, Russian and Portuguese strings and their x / y offset bytes. Called once by the boot code before ui_text_language_apply.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "ui/text.h"

/* Same functions, declared the way the stock code calls them (see the definitions for the
 * full interface): the C must pass exactly these registers. */
void ui_text_set_lang1_raw(u32 k, const char *s, u32 a, u32 b) __asm__("ui_text_set_lang1");
void ui_text_set_lang2_raw(u32 k, const char *s, u32 a, u32 b) __asm__("ui_text_set_lang2");
void ui_text_set_lang3_raw(u32 k, const char *s, u32 a, u32 b) __asm__("ui_text_set_lang3");
void ui_text_set_lang4_raw(u32 k, const char *s, u32 a, u32 b) __asm__("ui_text_set_lang4");

#define S(a) ((const char *)(a##u))

static void ut(u32 k, const char *zh, int zx, int zy, const char *en, int ex, int ey,
               const char *ru, int rx, int ry, const char *pt, int px, int py)
{
    ui_text_set_lang1_raw(k, zh, zx, zy);
    ui_text_set_lang2_raw(k, en, ex, ey);
    ui_text_set_lang3_raw(k, ru, rx, ry);
    ui_text_set_lang4_raw(k, pt, px, py);
}

/* comments: the English column */
void ui_texts_init(void)
{
    ut(  0, S(0x4008F04C), 0, 1, S(0x40090028), 0, 0, S(0x4008F05C), 0, 0, S(0x40090030), 0, 0);   /* System */
    ut(  1, S(0x4008F06C), 0, 1, S(0x4008FA5C), 0, 0, S(0x4008F07C), 0, 0, S(0x4008FA68), 0, 0);   /* Function */
    ut(  2, S(0x4008F08C), 0, 1, S(0x40090038), 0, 0, S(0x4008E938), 0, 0, S(0x4008FA74), 0, 0);   /* Measure */
    ut(  3, S(0x4008F09C), 0, 1, S(0x40090040), 0, 0, S(0x4008F0AC), 0, 0, S(0x4008FA80), 0, 0);   /* Browser */
    ut(  4, S(0x4008F0BC), 0, 1, S(0x4008FA8C), 0, 0, S(0x4008F0CC), 0, 0, S(0x4008FA8C), 0, 0);   /* Cursor X */
    ut(  5, S(0x4008F0DC), 0, 1, S(0x4008FA98), 0, 0, S(0x4008F0EC), 0, 0, S(0x4008FA98), 0, 0);   /* Cursor Y */
    ut(  6, S(0x4008F0FC), 0, 1, S(0x4008FAA4), 0, 0, S(0x4008FAB0), 0, 0, S(0x40090048), 0, 0);   /* Save wave */
    ut(  7, S(0x4008F10C), 0, 1, S(0x4008FABC), 0, 0, S(0x4008E94C), 0, 0, S(0x40090050), 0, 0);   /* Screenshot */
    ut(  8, S(0x4008F11C), 0, 1, S(0x40090058), 0, 0, S(0x4008F12C), 0, 0, S(0x40090058), 0, 0);   /* Zoom */
    ut(  9, S(0x4008F13C), 0, 0, S(0x4008E37C), 0, 0, S(0x4008E960), 0, 0, S(0x4008F14C), 0, 0);   /* Configuration settings */
    ut( 10, S(0x4008F15C), 0, 0, S(0x4008E974), 0, 0, S(0x4008D4B8), 0, 0, S(0x4008E394), 0, 0);   /* System calibration */
    ut( 11, S(0x4008FAC8), 0, 0, S(0x4008FAD4), 0, 0, S(0x4008E988), 0, 0, S(0x4008FAE0), 0, 0);   /* USB connect */
    ut( 12, S(0x4008F16C), 0, 0, S(0x4008E99C), 0, 0, S(0x4008DA40), 0, 0, S(0x4008DCB0), 0, 0);   /* Automatic shutdown */
    ut( 13, S(0x4008F17C), 0, 0, S(0x4008E9B0), 0, 0, S(0x4008DA60), 0, 0, S(0x4008DCCC), 0, 0);   /* Language settings */
    ut( 14, S(0x4008E9C4), 0, 0, S(0x4008E9D8), 0, 0, S(0x4008DA80), 0, 0, S(0x4008DCE8), 0, 0);   /* Factory settings */
    ut( 15, S(0x4008F18C), 0, 0, S(0x4008F19C), 0, 0, S(0x4008DD04), 0, 0, S(0x4008E9EC), 0, 0);   /* Disk formatting */
    ut( 16, S(0x4008F1AC), 0, 0, S(0x4008F1BC), 0, 0, S(0x4008D4E0), 0, 0, S(0x4008E3AC), 0, 0);   /* Waveform light */
    ut( 17, S(0x4008F1CC), 0, 0, S(0x4008EA00), 0, 0, S(0x4008D508), 0, 0, S(0x4008EA14), 0, 0);   /* Color temperature */
    ut( 18, S(0x4008FAEC), 0, 0, S(0x4008EA28), 0, 0, S(0x4008F1DC), 0, 0, S(0x4008FAF8), 0, 0);   /* X-Y mode display */
    ut( 19, S(0x4008F1EC), 0, 0, S(0x4008F1FC), 0, 0, S(0x4008DD20), 0, 0, S(0x4008EA3C), 0, 0);   /* Background gird */
    ut( 20, S(0x4008F20C), 0, 0, S(0x4008F21C), 0, 0, S(0x4008DAA0), 0, 0, S(0x4008F22C), 0, 0);   /* Capture output */
    ut( 21, S(0x4008E3C4), 0, 0, S(0x4008DD3C), 0, 0, S(0x4008CEA0), 0, 0, S(0x4008D530), 0, 0);   /* Frequency response analyzer */
    ut( 22, S(0x4008F23C), 0, 0, S(0x4008EA50), 0, 0, S(0x4008D0BC), 0, 0, S(0x4008E3DC), 0, 0);   /* Spectrum analyzer */
    ut( 23, S(0x4008EA64), 0, 0, S(0x4008DD58), 0, 0, S(0x4008D558), 0, 0, S(0x4008E3F4), 0, 0);   /* CH1 parameter measurement */
    ut( 24, S(0x4008EA78), 0, 0, S(0x4008DD74), 0, 0, S(0x4008D580), 0, 0, S(0x4008E40C), 0, 0);   /* CH2 parameter measurement */
    ut( 25, S(0x4008F24C), 0, 0, S(0x4008F25C), 0, 0, S(0x4008DD90), 0, 0, S(0x4008F26C), 0, 0);   /* Channel enable */
    ut( 26, S(0x4008F27C), 0, 0, S(0x4008F25C), 0, 0, S(0x4008DD90), 0, 0, S(0x4008F26C), 0, 0);   /* Channel enable */
    ut( 27, S(0x4008F28C), 0, 0, S(0x4008EA8C), 0, 0, S(0x4008DAC0), 0, 0, S(0x4008E424), 0, 0);   /* Proble attenuation */
    ut( 28, S(0x4008F29C), 0, 0, S(0x4008F2AC), 0, 0, S(0x4008DDAC), 0, 0, S(0x4008EAA0), 0, 0);   /* Coupling mode */
    ut( 29, S(0x40090060), 0, 0, "DC", 0, 0, "DC", 0, 0, "DC", 0, 0);   /* DC */
    ut( 30, S(0x40090068), 0, 0, "AC", 0, 0, "AC", 0, 0, "AC", 0, 0);   /* AC */
    ut( 31, S(0x4008FB04), 0, 0, S(0x4008FB10), 0, 0, S(0x4008E43C), 0, 0, S(0x4008FB10), 0, 0);   /* FFT display */
    ut( 32, S(0x4008F2BC), 0, 0, S(0x4008F2CC), 0, 0, S(0x4008DDC8), 0, 0, S(0x4008F2DC), 0, 0);   /* Bandwidth limit */
    ut( 33, S(0x4008F2EC), 0, 0, S(0x4008FB1C), 0, 0, S(0x4008EAB4), 0, 0, S(0x4008FB28), 0, 0);   /* Full 350M */
    ut( 34, S(0x4008FB34), 0, 0, S(0x4008F2FC), 0, 0, S(0x4008D358), 0, 0, S(0x4008E454), 0, 0);   /* Main time base */
    ut( 35, S(0x4008F11C), 0, 0, S(0x4008F30C), 0, 0, S(0x4008D7D4), 0, 0, S(0x4008F31C), 0, 0);   /* Zoom time base */
    ut( 36, S(0x4008EAC8), 0, 0, S(0x4008E46C), 0, 0, S(0x4008CFB8), 0, 0, S(0x4008DDE4), 0, 0);   /* High resolution mode */
    ut( 37, S(0x4008F32C), 0, 0, S(0x4008F33C), 0, 0, S(0x4008DE00), 0, 0, S(0x4008F34C), 0, 0);   /* Trigger mode */
    ut( 38, S(0x4008F35C), 0, 0, S(0x4008F36C), 0, 0, S(0x4008DE1C), 0, 0, S(0x4008F37C), 0, 0);   /* Trigger edge */
    ut( 39, S(0x4008F38C), 0, 0, S(0x4008F39C), 0, 0, S(0x4008DE38), 0, 0, S(0x4008EADC), 0, 0);   /* Trigger channel */
    ut( 40, S(0x4008F3AC), 0, 0, S(0x4008F3BC), 0, 0, S(0x4008DE54), 0, 0, S(0x4008F3CC), 0, 0);   /* HF suppresstion */
    ut( 41, S(0x40090070), 0, 0, S(0x40090078), 0, 0, S(0x4008FB40), 0, 0, S(0x40090078), 0, 0);   /* Auto */
    ut( 42, S(0x40090080), 0, 0, S(0x40090088), 0, 0, S(0x40090090), 0, 0, S(0x4008FB4C), 0, 0);   /* Single */
    ut( 43, S(0x40090098), 0, 0, S(0x400900A0), 0, 0, S(0x4008F3DC), 0, 0, S(0x400900A0), 0, 0);   /* Normal */
    ut( 44, S(0x4008FB58), 0, 0, S(0x400900A8), 0, 0, S(0x4008EAF0), 0, 0, S(0x400900B0), 0, 0);   /* Rising */
    ut( 45, S(0x4008FB64), 0, 0, S(0x400900B8), 0, 0, S(0x4008F3EC), 0, 0, S(0x400900C0), 0, 0);   /* Falling */
    ut( 46, S(0x400900C8), 0, 0, S(0x4008FB70), 0, 0, S(0x4008F3FC), 0, 0, S(0x4008FB7C), 0, 0);   /* Frequency */
    ut( 47, S(0x400900D0), 0, 0, S(0x4008FB88), 0, 0, S(0x4008EB04), 0, 0, S(0x4008FB88), 0, 0);   /* Amplitude */
    ut( 48, S(0x400900D8), 0, 0, S(0x400900E0), 0, 0, S(0x4008E484), 0, 0, S(0x4008F40C), 0, 0);   /* Offset */
    ut( 49, S(0x4008FB94), 0, 0, S(0x4008FBA0), 0, 0, S(0x4008FBAC), 0, 0, S(0x400900E8), 0, 0);   /* Duty pulse */
    ut( 50, S(0x4008FBB8), 0, 0, S(0x400900F0), 0, 0, S(0x4008FBC4), 0, 0, S(0x400900F8), 0, 0);   /* Sine */
    ut( 51, S(0x40090100), 0, 0, S(0x40090108), 0, 0, S(0x4008F41C), 0, 0, S(0x4008FBD0), 0, 0);   /* Square */
    ut( 52, S(0x4008FBDC), 0, 0, S(0x4008FBE8), 0, 0, S(0x4008E49C), 0, 0, S(0x4008FBE8), 0, 0);   /* Triangular */
    ut( 53, S(0x4008FBF4), 0, 0, S(0x4008FC00), 0, 0, S(0x4008E4B4), 0, 0, S(0x4008FC00), 0, 0);   /* Sawtooth1 */
    ut( 54, S(0x4008FC0C), 0, 0, S(0x4008FC18), 0, 0, S(0x4008E4CC), 0, 0, S(0x4008FC18), 0, 0);   /* Sawtooth2 */
    ut( 55, S(0x4008FC24), 0, 0, S(0x40090110), 0, 0, S(0x4008FC30), 0, 0, S(0x40090118), 0, 0);   /* Step */
    ut( 56, S(0x40090120), 0, 0, S(0x40090128), 0, 0, S(0x4008EB18), 0, 0, S(0x40090130), 0, 0);   /* Half */
    ut( 57, S(0x40090138), 0, 0, S(0x40090140), 0, 0, S(0x4008FC3C), 0, 0, S(0x40090148), 0, 0);   /* Full */
    ut( 58, S(0x4008FC48), 0, 0, S(0x4008FC54), 0, 0, S(0x4008F42C), 0, 0, S(0x4008FC60), 0, 0);   /* Exponent */
    ut( 59, S(0x4008F43C), 0, 0, S(0x40090150), 0, 0, S(0x4008E4E4), 0, 0, S(0x40090158), 0, 0);   /* Ringing */
    ut( 60, S(0x4008FC6C), 0, 0, S(0x40090160), 0, 0, S(0x4008F44C), 0, 0, S(0x40090168), 0, 0);   /* Sqrt */
    ut( 61, S(0x4008F45C), 0, 0, S(0x4008FC78), 0, 0, S(0x4008FC84), 0, 0, S(0x40090170), 0, 0);   /* Multi-sin */
    ut( 62, S(0x4008F46C), 0, 0, S(0x40090178), 0, 0, S(0x4008F47C), 0, 0, S(0x4008FC90), 0, 0);   /* SinC */
    ut( 63, S(0x40090060), 0, 0, "DC", 0, 0, "DC", 0, 0, "DC", 0, 0);   /* DC */
    ut( 64, S(0x40090180), 0, 0, S(0x40090188), 0, 0, S(0x4008F48C), 0, 0, S(0x4008FC9C), 0, 0);   /* Capture */
    ut( 65, S(0x40090190), 0, 0, S(0x4008FCA8), 0, 0, S(0x4008F49C), 0, 0, S(0x40090198), 0, 0);   /* Fast move */
    ut( 66, S(0x400901A0), 0, 0, S(0x4008FCB4), 0, 0, S(0x4008EB2C), 0, 0, S(0x400901A8), 0, 0);   /* Slow move */
    ut( 67, S(0x4008F4AC), 0, 1, S(0x400901B0), 0, 0, S(0x4008E4FC), 0, 0, S(0x400901B8), 0, 0);   /* Return */
    ut( 68, S(0x4008F15C), 0, 1, S(0x4008FCC0), 0, 0, S(0x4008E514), 0, 0, S(0x4008F4BC), 0, 0);   /* Calibration */
    ut( 69, S(0x4008F4CC), 0, 1, S(0x400901C0), 0, 0, S(0x4008F4DC), 0, 0, S(0x400901C0), 0, 0);   /* Cursor */
    ut( 70, S(0x4008F4EC), 0, 1, S(0x40090088), 0, 0, S(0x40090090), 0, 0, S(0x4008FB4C), 0, 0);   /* Single */
    ut( 71, S(0x4008F4FC), 0, 1, S(0x400901C8), 0, 0, S(0x4008FCCC), 0, 0, S(0x400901C8), 0, 0);   /* Loop */
    ut( 72, S(0x400900D0), 0, 0, S(0x4008FB88), 0, 0, S(0x4008EB04), 0, 0, S(0x4008FB88), 0, 0);   /* Amplitude */
    ut( 73, S(0x400900D8), 0, 0, S(0x400900E0), 0, 0, S(0x4008E484), 0, 0, S(0x4008F50C), 0, 0);   /* Offset */
    ut( 74, S(0x4008F51C), 0, 0, S(0x4008F52C), 0, 0, S(0x4008F53C), 0, 0, S(0x4008FCD8), 0, 0);   /* Start frequency */
    ut( 75, S(0x4008F54C), 0, 0, S(0x4008F55C), 0, 0, S(0x4008F56C), 0, 0, S(0x4008FCE4), 0, 0);   /* Stop frequency */
    ut( 76, S(0x4008F57C), 0, 0, S(0x4008F58C), 0, 0, S(0x4008E52C), 0, 0, S(0x4008FCF0), 0, 0);   /* Frequency count */
    ut( 77, S(0x400901D0), 0, 0, S(0x4008FCFC), 0, 0, S(0x4008FD08), 0, 0, S(0x4008FD14), 0, 0);   /* Movement */
    ut( 78, S(0x4008F59C), 0, 0, S(0x400901D8), 0, 0, S(0x4008FB40), 0, 0, S(0x4008FD20), 0, 0);   /* Autoset */
    ut( 79, S(0x400901E0), 0, 0, S(0x4008FD2C), 0, 0, S(0x4008F5AC), 0, 0, S(0x4008FD38), 0, 0);   /* Reference */
    ut( 80, S(0x400901E8), 0, 0, S(0x4008FD44), 0, 0, S(0x4008E544), 0, 0, S(0x4008FD50), 0, 0);   /* Attenuation */
    ut( 81, S(0x4008F5BC), 0, 0, S(0x4008FD5C), 0, 0, S(0x4008F5CC), 0, 0, S(0x4008FD68), 0, 0);   /* Center freq */
    ut( 82, S(0x4008F5DC), 0, 0, S(0x4008FD74), 0, 0, S(0x4008F5EC), 0, 0, S(0x4008FD80), 0, 0);   /* Freq width */
    ut( 83, S(0x4008F5FC), 0, 0, S(0x4008EB40), 0, 0, S(0x4008D384), 0, 0, S(0x4008E55C), 0, 0);   /* Save configuration */
    ut( 84, S(0x4008F60C), 0, 0, S(0x4008EB54), 0, 0, S(0x4008D5A8), 0, 0, S(0x4008E574), 0, 0);   /* Read configuration */
    ut( 85, S(0x4008F61C), 0, 0, S(0x4008E58C), 0, 0, S(0x4008CFEC), 0, 0, S(0x4008DE70), 0, 0);   /* Power-on configuration */
    ut( 86, S(0x400901F0), 0, 0, S(0x4008F62C), 0, 0, S(0x4008DE8C), 0, 0, S(0x4008EB68), 0, 0);   /* Configuration 1 */
    ut( 87, S(0x400901F8), 0, 0, S(0x4008F63C), 0, 0, S(0x4008DEA8), 0, 0, S(0x4008EB7C), 0, 0);   /* Configuration 2 */
    ut( 88, S(0x40090200), 0, 0, S(0x4008F64C), 0, 0, S(0x4008DEC4), 0, 0, S(0x4008EB90), 0, 0);   /* Configuration 3 */
    ut( 89, S(0x40090208), 0, 0, S(0x4008F65C), 0, 0, S(0x4008DEE0), 0, 0, S(0x4008EBA4), 0, 0);   /* Configuration 4 */
    ut( 90, S(0x40090210), 0, 0, S(0x4008F66C), 0, 0, S(0x4008DEFC), 0, 0, S(0x4008EBB8), 0, 0);   /* Configuration 5 */
    ut( 91, S(0x4008FD8C), 0, 0, S(0x4008FD98), 0, 0, S(0x4008DF18), 0, 0, S(0x40090218), 0, 0);   /* Notice ! */
    ut( 92, S(0x4008D7F8), 0, 0, S(0x4008DF34), 0, 0, S(0x4008D5D0), 5, 0, S(0x4008E5A4), 0, 0);   /* Please unplug all signal */
    ut( 93, "", 0, 0, S(0x4008E5BC), 0, 0, S(0x4008EBCC), 5, 0, S(0x4008EBE0), 0, 0);   /* proble and USB first ! */
    ut( 94, "", 0, 0, "", 0, 0, "", 0, 0, "", 0, 0);   /*  */
    ut( 95, S(0x40090220), 0, 0, S(0x40090228), 0, 0, S(0x4008E5D4), 0, 0, S(0x4008FDA4), 0, 0);   /* Confirm */
    ut( 96, S(0x4008EBF4), 0, 0, S(0x4008EC08), 0, 0, S(0x4008DF50), 0, 0, S(0x4008F67C), 0, 0);   /* Calibratting ... */
    ut( 97, S(0x4008FDB0), 0, 0, S(0x4008FDBC), 0, 0, S(0x4008F68C), 0, 0, S(0x4008FDC8), 0, 0);   /* Succeed ! */
    ut(101, S(0x4008FDD4), 0, 0, S(0x4008FDE0), 0, 0, S(0x4008EC1C), 0, 0, S(0x4008FDEC), 0, 0);   /* Failed ! */
    ut( 98, S(0x4008F69C), 0, 0, S(0x4008F6AC), 0, 0, S(0x4008EC30), 0, 0, S(0x4008F6BC), 0, 0);   /* CH1 failed ! */
    ut( 99, S(0x4008F6CC), 0, 0, S(0x4008F6DC), 0, 0, S(0x4008EC44), 0, 0, S(0x4008F6EC), 0, 0);   /* CH2 failed ! */
    ut(100, S(0x4008F6FC), 0, 0, S(0x4008FDE0), 0, 0, S(0x4008E5EC), 0, 0, S(0x4008F70C), 0, 0);   /* Failed ! */
    ut(102, S(0x4008E604), 0, 0, S(0x4008EC58), 0, 0, S(0x4008D81C), 0, 0, S(0x4008EC6C), 10, 0);   /* Restore to factory */
    ut(103, "", 0, 0, S(0x4008FDF8), 0, 0, "", 0, 0, S(0x4008FE04), 10, 0);   /* settings ? */
    ut(104, "", 0, 0, "", 0, 0, "", 0, 0, "", 0, 0);   /*  */
    ut(105, S(0x4008FE10), 0, 0, S(0x40090230), 0, 0, S(0x4008F71C), 0, 0, S(0x4008FE1C), 0, 0);   /* Done ! */
    ut(106, S(0x4008FE28), 0, 0, S(0x4008FE34), 0, 0, S(0x4008EC80), 0, 0, S(0x40090218), 0, 0);   /* Warning ! */
    ut(107, S(0x4008DAE0), 0, 0, S(0x4008EC94), 0, 0, S(0x4008DB00), -13, -4, S(0x4008E61C), -8, -4);   /* All saved data will */
    ut(108, S(0x4008ECA8), 0, 0, S(0x4008FE40), 0, 0, S(0x4008D5F8), -13, -4, S(0x4008ECBC), -8, -4);   /* be clear ! */
    ut(109, "", 0, 0, "", 0, 0, S(0x4008DF6C), -13, -4, S(0x4008ECD0), -8, -4);   /*  */
    ut(110, S(0x4008F72C), 0, 0, S(0x4008F73C), 0, 0, S(0x4008DB20), 0, 0, S(0x4008E634), 0, 0);   /* Picture browser */
    ut(111, S(0x4008F74C), 0, 0, S(0x4008ECE4), 0, 0, S(0x4008D3B0), 0, 0, S(0x4008E64C), 0, 0);   /* Waveform browser */
    ut(112, S(0x4008F75C), 0, 0, S(0x4008F76C), 0, 0, S(0x4008D840), 0, 0, S(0x4008DF88), 0, 0);   /* Capture browser */
    ut(113, S(0x40090238), 0, 0, S(0x40090240), 0, 0, S(0x4008FE4C), 0, 0, S(0x4008FE58), 0, 0);   /* Select */
    ut(114, S(0x40090248), 0, 0, S(0x4008FE64), 0, 0, S(0x4008ECF8), 0, 0, S(0x4008F77C), 0, 0);   /* Select all */
    ut(115, S(0x40090250), 0, 0, S(0x40090258), 0, 0, S(0x4008ED0C), 0, 0, S(0x40090260), 0, 0);   /* Delete */
    ut(116, S(0x4008FE70), 0, 0, S(0x4008FE7C), 0, 0, S(0x4008ED20), 0, 0, S(0x4008FE88), 0, 0);   /* Last page */
    ut(117, S(0x4008FE94), 0, 0, S(0x4008FEA0), 0, 0, S(0x4008ED34), 0, 0, S(0x4008FEAC), 0, 0);   /* Next page */
    ut(118, S(0x4008F78C), 0, 0, S(0x4008F79C), 0, 0, S(0x4008ED0C), 0, 0, S(0x40090260), 0, 0);   /* Delete selected */
    ut(119, S(0x40090268), 0, 0, S(0x40090270), 0, 0, S(0x4008FEB8), -10, 0, S(0x40090270), 0, 0);   /* Total */
    ut(120, S(0x4008FEC4), 0, 0, S(0x40090278), 0, 0, S(0x4008F7AC), 0, 0, S(0x40090280), 0, 0);   /* items */
    ut(121, "\351\241\265", 0, 0, S(0x40090288), 0, 0, S(0x4008F7BC), 0, 0, S(0x4008FED0), 0, 0);   /* pages */
    ut(122, "\351\241\265", 0, 0, S(0x40090290), 8, 0, S(0x4008F7BC), 0, 0, S(0x40090298), 8, 0);   /* page */
    ut(123, S(0x400902A0), 20, 0, S(0x400902A8), 0, 0, S(0x4008F7CC), -15, 0, S(0x400902B0), 0, 0);   /* Current */
    ut(124, S(0x4008E664), 0, 0, S(0x4008ED48), 0, 0, S(0x4008D0EC), 0, 0, S(0x4008DFA4), 0, 0);   /* Waveform saving ... */
    ut(125, S(0x4008E67C), 0, 0, S(0x4008E694), 0, 0, S(0x4008D11C), 0, 0, S(0x4008ED5C), 0, 0);   /* Screenshut saving ... */
    ut(126, S(0x4008D620), 0, 0, S(0x4008DFC0), 0, 0, S(0x4008D3DC), 0, 0, S(0x4008DFDC), 9, 0);   /* Please connect all channels */
    ut(127, S(0x4008FEDC), 0, 0, S(0x4008DFF8), 0, 0, S(0x4008D648), 0, 0, S(0x4008ED70), 9, 0);   /* to the signal generator ! */
    ut(128, "", 0, 0, "", 0, 0, "", 0, 0, "", 0, 0);   /*  */
    ut(129, S(0x4008D864), 0, 0, S(0x4008D670), 0, 0, S(0x4008C9D4), 0, 0, S(0x4008CED8), 0, 0);   /* No signal detected in all channels ! */
    ut(130, S(0x4008E014), 0, 0, S(0x4008D698), 0, 0, S(0x4008CDB0), 0, 0, S(0x4008D14C), 0, 0);   /* No signal detected in all channel 1 ! */
    ut(131, S(0x4008E030), 0, 0, S(0x4008D6C0), 0, 0, S(0x4008CDEC), 0, 0, S(0x4008D17C), 0, 0);   /* No signal detected in all channel 2 ! */
    ut(132, S(0x4008E04C), 0, 0, S(0x4008E6AC), 0, 0, S(0x4008E068), 0, 0, S(0x4008D6E8), 0, 0);   /* No signal detected ! */
    ut(133, S(0x4008E6C4), 0, 0, S(0x4008E6DC), 0, 0, S(0x4008CD30), 0, 0, S(0x4008D888), 0, 0);   /* Disk space is full ! */
    ut(134, S(0x4008ED84), 0, 0, S(0x4008FEE8), 0, 0, S(0x4008FEF4), 0, 0, S(0x4008FF00), 0, 0);   /* Autoset ... */
    ut(135, S(0x4008F7DC), 0, 0, S(0x4008F7EC), 0, 0, S(0x4008DB40), 0, 0, S(0x4008ED98), 0, 0);   /* Auto shutdown ! */
    ut(136, S(0x4008E6F4), 0, 0, S(0x4008E70C), 0, 0, S(0x4008D1AC), 0, 0, S(0x4008EDAC), 0, 0);   /* Low battery shutdown ! */
    ut(137, S(0x4008FF0C), 0, 0, S(0x4008FF18), 0, 0, S(0x4008F7FC), 0, 0, S(0x4008F80C), 0, 0);   /* FFT length */
    ut(138, S(0x4008F81C), 0, 0, S(0x4008EDC0), 0, 0, S(0x4008E084), 0, 0, S(0x4008EDD4), 0, 0);   /* Waterfall_display */
    ut(139, S(0x4008FF24), 0, 0, S(0x4008FF30), 0, 0, S(0x4008D408), 0, 0, S(0x4008EDE8), 0, 0);   /* 3D display */
    ut(140, S(0x400902B8), 0, 0, S(0x40048EF4), 0, 0, S(0x4008FF3C), 0, 0, S(0x4008FF48), 0, 0);   /* OFF */
}
