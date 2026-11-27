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

#define S(a) ((const char *)IMG(a))

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
    ut(  0, S(4008f04c), 0, 1, S(40090028), 0, 0, S(4008f05c), 0, 0, S(40090030), 0, 0);   /* System */
    ut(  1, S(4008f06c), 0, 1, S(4008fa5c), 0, 0, S(4008f07c), 0, 0, S(4008fa68), 0, 0);   /* Function */
    ut(  2, S(4008f08c), 0, 1, S(40090038), 0, 0, S(4008e938), 0, 0, S(4008fa74), 0, 0);   /* Measure */
    ut(  3, S(4008f09c), 0, 1, S(40090040), 0, 0, S(4008f0ac), 0, 0, S(4008fa80), 0, 0);   /* Browser */
    ut(  4, S(4008f0bc), 0, 1, S(4008fa8c), 0, 0, S(4008f0cc), 0, 0, S(4008fa8c), 0, 0);   /* Cursor X */
    ut(  5, S(4008f0dc), 0, 1, S(4008fa98), 0, 0, S(4008f0ec), 0, 0, S(4008fa98), 0, 0);   /* Cursor Y */
    ut(  6, S(4008f0fc), 0, 1, S(4008faa4), 0, 0, S(4008fab0), 0, 0, S(40090048), 0, 0);   /* Save wave */
    ut(  7, S(4008f10c), 0, 1, S(4008fabc), 0, 0, S(4008e94c), 0, 0, S(40090050), 0, 0);   /* Screenshot */
    ut(  8, S(4008f11c), 0, 1, S(40090058), 0, 0, S(4008f12c), 0, 0, S(40090058), 0, 0);   /* Zoom */
    ut(  9, S(4008f13c), 0, 0, S(4008e37c), 0, 0, S(4008e960), 0, 0, S(4008f14c), 0, 0);   /* Configuration settings */
    ut( 10, S(4008f15c), 0, 0, S(4008e974), 0, 0, S(4008d4b8), 0, 0, S(4008e394), 0, 0);   /* System calibration */
    ut( 11, S(4008fac8), 0, 0, S(4008fad4), 0, 0, S(4008e988), 0, 0, S(4008fae0), 0, 0);   /* USB connect */
    ut( 12, S(4008f16c), 0, 0, S(4008e99c), 0, 0, S(4008da40), 0, 0, S(4008dcb0), 0, 0);   /* Automatic shutdown */
    ut( 13, S(4008f17c), 0, 0, S(4008e9b0), 0, 0, S(4008da60), 0, 0, S(4008dccc), 0, 0);   /* Language settings */
    ut( 14, S(4008e9c4), 0, 0, S(4008e9d8), 0, 0, S(4008da80), 0, 0, S(4008dce8), 0, 0);   /* Factory settings */
    ut( 15, S(4008f18c), 0, 0, S(4008f19c), 0, 0, S(4008dd04), 0, 0, S(4008e9ec), 0, 0);   /* Disk formatting */
    ut( 16, S(4008f1ac), 0, 0, S(4008f1bc), 0, 0, S(4008d4e0), 0, 0, S(4008e3ac), 0, 0);   /* Waveform light */
    ut( 17, S(4008f1cc), 0, 0, S(4008ea00), 0, 0, S(4008d508), 0, 0, S(4008ea14), 0, 0);   /* Color temperature */
    ut( 18, S(4008faec), 0, 0, S(4008ea28), 0, 0, S(4008f1dc), 0, 0, S(4008faf8), 0, 0);   /* X-Y mode display */
    ut( 19, S(4008f1ec), 0, 0, S(4008f1fc), 0, 0, S(4008dd20), 0, 0, S(4008ea3c), 0, 0);   /* Background gird */
    ut( 20, S(4008f20c), 0, 0, S(4008f21c), 0, 0, S(4008daa0), 0, 0, S(4008f22c), 0, 0);   /* Capture output */
    ut( 21, S(4008e3c4), 0, 0, S(4008dd3c), 0, 0, S(4008cea0), 0, 0, S(4008d530), 0, 0);   /* Frequency response analyzer */
    ut( 22, S(4008f23c), 0, 0, S(4008ea50), 0, 0, S(4008d0bc), 0, 0, S(4008e3dc), 0, 0);   /* Spectrum analyzer */
    ut( 23, S(4008ea64), 0, 0, S(4008dd58), 0, 0, S(4008d558), 0, 0, S(4008e3f4), 0, 0);   /* CH1 parameter measurement */
    ut( 24, S(4008ea78), 0, 0, S(4008dd74), 0, 0, S(4008d580), 0, 0, S(4008e40c), 0, 0);   /* CH2 parameter measurement */
    ut( 25, S(4008f24c), 0, 0, S(4008f25c), 0, 0, S(4008dd90), 0, 0, S(4008f26c), 0, 0);   /* Channel enable */
    ut( 26, S(4008f27c), 0, 0, S(4008f25c), 0, 0, S(4008dd90), 0, 0, S(4008f26c), 0, 0);   /* Channel enable */
    ut( 27, S(4008f28c), 0, 0, S(4008ea8c), 0, 0, S(4008dac0), 0, 0, S(4008e424), 0, 0);   /* Proble attenuation */
    ut( 28, S(4008f29c), 0, 0, S(4008f2ac), 0, 0, S(4008ddac), 0, 0, S(4008eaa0), 0, 0);   /* Coupling mode */
    ut( 29, S(40090060), 0, 0, "DC", 0, 0, "DC", 0, 0, "DC", 0, 0);   /* DC */
    ut( 30, S(40090068), 0, 0, "AC", 0, 0, "AC", 0, 0, "AC", 0, 0);   /* AC */
    ut( 31, S(4008fb04), 0, 0, S(4008fb10), 0, 0, S(4008e43c), 0, 0, S(4008fb10), 0, 0);   /* FFT display */
    ut( 32, S(4008f2bc), 0, 0, S(4008f2cc), 0, 0, S(4008ddc8), 0, 0, S(4008f2dc), 0, 0);   /* Bandwidth limit */
    ut( 33, S(4008f2ec), 0, 0, S(4008fb1c), 0, 0, S(4008eab4), 0, 0, S(4008fb28), 0, 0);   /* Full 350M */
    ut( 34, S(4008fb34), 0, 0, S(4008f2fc), 0, 0, S(4008d358), 0, 0, S(4008e454), 0, 0);   /* Main time base */
    ut( 35, S(4008f11c), 0, 0, S(4008f30c), 0, 0, S(4008d7d4), 0, 0, S(4008f31c), 0, 0);   /* Zoom time base */
    ut( 36, S(4008eac8), 0, 0, S(4008e46c), 0, 0, S(4008cfb8), 0, 0, S(4008dde4), 0, 0);   /* High resolution mode */
    ut( 37, S(4008f32c), 0, 0, S(4008f33c), 0, 0, S(4008de00), 0, 0, S(4008f34c), 0, 0);   /* Trigger mode */
    ut( 38, S(4008f35c), 0, 0, S(4008f36c), 0, 0, S(4008de1c), 0, 0, S(4008f37c), 0, 0);   /* Trigger edge */
    ut( 39, S(4008f38c), 0, 0, S(4008f39c), 0, 0, S(4008de38), 0, 0, S(4008eadc), 0, 0);   /* Trigger channel */
    ut( 40, S(4008f3ac), 0, 0, S(4008f3bc), 0, 0, S(4008de54), 0, 0, S(4008f3cc), 0, 0);   /* HF suppresstion */
    ut( 41, S(40090070), 0, 0, S(40090078), 0, 0, S(4008fb40), 0, 0, S(40090078), 0, 0);   /* Auto */
    ut( 42, S(40090080), 0, 0, S(40090088), 0, 0, S(40090090), 0, 0, S(4008fb4c), 0, 0);   /* Single */
    ut( 43, S(40090098), 0, 0, S(400900a0), 0, 0, S(4008f3dc), 0, 0, S(400900a0), 0, 0);   /* Normal */
    ut( 44, S(4008fb58), 0, 0, S(400900a8), 0, 0, S(4008eaf0), 0, 0, S(400900b0), 0, 0);   /* Rising */
    ut( 45, S(4008fb64), 0, 0, S(400900b8), 0, 0, S(4008f3ec), 0, 0, S(400900c0), 0, 0);   /* Falling */
    ut( 46, S(400900c8), 0, 0, S(4008fb70), 0, 0, S(4008f3fc), 0, 0, S(4008fb7c), 0, 0);   /* Frequency */
    ut( 47, S(400900d0), 0, 0, S(4008fb88), 0, 0, S(4008eb04), 0, 0, S(4008fb88), 0, 0);   /* Amplitude */
    ut( 48, S(400900d8), 0, 0, S(400900e0), 0, 0, S(4008e484), 0, 0, S(4008f40c), 0, 0);   /* Offset */
    ut( 49, S(4008fb94), 0, 0, S(4008fba0), 0, 0, S(4008fbac), 0, 0, S(400900e8), 0, 0);   /* Duty pulse */
    ut( 50, S(4008fbb8), 0, 0, S(400900f0), 0, 0, S(4008fbc4), 0, 0, S(400900f8), 0, 0);   /* Sine */
    ut( 51, S(40090100), 0, 0, S(40090108), 0, 0, S(4008f41c), 0, 0, S(4008fbd0), 0, 0);   /* Square */
    ut( 52, S(4008fbdc), 0, 0, S(4008fbe8), 0, 0, S(4008e49c), 0, 0, S(4008fbe8), 0, 0);   /* Triangular */
    ut( 53, S(4008fbf4), 0, 0, S(4008fc00), 0, 0, S(4008e4b4), 0, 0, S(4008fc00), 0, 0);   /* Sawtooth1 */
    ut( 54, S(4008fc0c), 0, 0, S(4008fc18), 0, 0, S(4008e4cc), 0, 0, S(4008fc18), 0, 0);   /* Sawtooth2 */
    ut( 55, S(4008fc24), 0, 0, S(40090110), 0, 0, S(4008fc30), 0, 0, S(40090118), 0, 0);   /* Step */
    ut( 56, S(40090120), 0, 0, S(40090128), 0, 0, S(4008eb18), 0, 0, S(40090130), 0, 0);   /* Half */
    ut( 57, S(40090138), 0, 0, S(40090140), 0, 0, S(4008fc3c), 0, 0, S(40090148), 0, 0);   /* Full */
    ut( 58, S(4008fc48), 0, 0, S(4008fc54), 0, 0, S(4008f42c), 0, 0, S(4008fc60), 0, 0);   /* Exponent */
    ut( 59, S(4008f43c), 0, 0, S(40090150), 0, 0, S(4008e4e4), 0, 0, S(40090158), 0, 0);   /* Ringing */
    ut( 60, S(4008fc6c), 0, 0, S(40090160), 0, 0, S(4008f44c), 0, 0, S(40090168), 0, 0);   /* Sqrt */
    ut( 61, S(4008f45c), 0, 0, S(4008fc78), 0, 0, S(4008fc84), 0, 0, S(40090170), 0, 0);   /* Multi-sin */
    ut( 62, S(4008f46c), 0, 0, S(40090178), 0, 0, S(4008f47c), 0, 0, S(4008fc90), 0, 0);   /* SinC */
    ut( 63, S(40090060), 0, 0, "DC", 0, 0, "DC", 0, 0, "DC", 0, 0);   /* DC */
    ut( 64, S(40090180), 0, 0, S(40090188), 0, 0, S(4008f48c), 0, 0, S(4008fc9c), 0, 0);   /* Capture */
    ut( 65, S(40090190), 0, 0, S(4008fca8), 0, 0, S(4008f49c), 0, 0, S(40090198), 0, 0);   /* Fast move */
    ut( 66, S(400901a0), 0, 0, S(4008fcb4), 0, 0, S(4008eb2c), 0, 0, S(400901a8), 0, 0);   /* Slow move */
    ut( 67, S(4008f4ac), 0, 1, S(400901b0), 0, 0, S(4008e4fc), 0, 0, S(400901b8), 0, 0);   /* Return */
    ut( 68, S(4008f15c), 0, 1, S(4008fcc0), 0, 0, S(4008e514), 0, 0, S(4008f4bc), 0, 0);   /* Calibration */
    ut( 69, S(4008f4cc), 0, 1, S(400901c0), 0, 0, S(4008f4dc), 0, 0, S(400901c0), 0, 0);   /* Cursor */
    ut( 70, S(4008f4ec), 0, 1, S(40090088), 0, 0, S(40090090), 0, 0, S(4008fb4c), 0, 0);   /* Single */
    ut( 71, S(4008f4fc), 0, 1, S(400901c8), 0, 0, S(4008fccc), 0, 0, S(400901c8), 0, 0);   /* Loop */
    ut( 72, S(400900d0), 0, 0, S(4008fb88), 0, 0, S(4008eb04), 0, 0, S(4008fb88), 0, 0);   /* Amplitude */
    ut( 73, S(400900d8), 0, 0, S(400900e0), 0, 0, S(4008e484), 0, 0, S(4008f50c), 0, 0);   /* Offset */
    ut( 74, S(4008f51c), 0, 0, S(4008f52c), 0, 0, S(4008f53c), 0, 0, S(4008fcd8), 0, 0);   /* Start frequency */
    ut( 75, S(4008f54c), 0, 0, S(4008f55c), 0, 0, S(4008f56c), 0, 0, S(4008fce4), 0, 0);   /* Stop frequency */
    ut( 76, S(4008f57c), 0, 0, S(4008f58c), 0, 0, S(4008e52c), 0, 0, S(4008fcf0), 0, 0);   /* Frequency count */
    ut( 77, S(400901d0), 0, 0, S(4008fcfc), 0, 0, S(4008fd08), 0, 0, S(4008fd14), 0, 0);   /* Movement */
    ut( 78, S(4008f59c), 0, 0, S(400901d8), 0, 0, S(4008fb40), 0, 0, S(4008fd20), 0, 0);   /* Autoset */
    ut( 79, S(400901e0), 0, 0, S(4008fd2c), 0, 0, S(4008f5ac), 0, 0, S(4008fd38), 0, 0);   /* Reference */
    ut( 80, S(400901e8), 0, 0, S(4008fd44), 0, 0, S(4008e544), 0, 0, S(4008fd50), 0, 0);   /* Attenuation */
    ut( 81, S(4008f5bc), 0, 0, S(4008fd5c), 0, 0, S(4008f5cc), 0, 0, S(4008fd68), 0, 0);   /* Center freq */
    ut( 82, S(4008f5dc), 0, 0, S(4008fd74), 0, 0, S(4008f5ec), 0, 0, S(4008fd80), 0, 0);   /* Freq width */
    ut( 83, S(4008f5fc), 0, 0, S(4008eb40), 0, 0, S(4008d384), 0, 0, S(4008e55c), 0, 0);   /* Save configuration */
    ut( 84, S(4008f60c), 0, 0, S(4008eb54), 0, 0, S(4008d5a8), 0, 0, S(4008e574), 0, 0);   /* Read configuration */
    ut( 85, S(4008f61c), 0, 0, S(4008e58c), 0, 0, S(4008cfec), 0, 0, S(4008de70), 0, 0);   /* Power-on configuration */
    ut( 86, S(400901f0), 0, 0, S(4008f62c), 0, 0, S(4008de8c), 0, 0, S(4008eb68), 0, 0);   /* Configuration 1 */
    ut( 87, S(400901f8), 0, 0, S(4008f63c), 0, 0, S(4008dea8), 0, 0, S(4008eb7c), 0, 0);   /* Configuration 2 */
    ut( 88, S(40090200), 0, 0, S(4008f64c), 0, 0, S(4008dec4), 0, 0, S(4008eb90), 0, 0);   /* Configuration 3 */
    ut( 89, S(40090208), 0, 0, S(4008f65c), 0, 0, S(4008dee0), 0, 0, S(4008eba4), 0, 0);   /* Configuration 4 */
    ut( 90, S(40090210), 0, 0, S(4008f66c), 0, 0, S(4008defc), 0, 0, S(4008ebb8), 0, 0);   /* Configuration 5 */
    ut( 91, S(4008fd8c), 0, 0, S(4008fd98), 0, 0, S(4008df18), 0, 0, S(40090218), 0, 0);   /* Notice ! */
    ut( 92, S(4008d7f8), 0, 0, S(4008df34), 0, 0, S(4008d5d0), 5, 0, S(4008e5a4), 0, 0);   /* Please unplug all signal */
    ut( 93, "", 0, 0, S(4008e5bc), 0, 0, S(4008ebcc), 5, 0, S(4008ebe0), 0, 0);   /* proble and USB first ! */
    ut( 94, "", 0, 0, "", 0, 0, "", 0, 0, "", 0, 0);   /*  */
    ut( 95, S(40090220), 0, 0, S(40090228), 0, 0, S(4008e5d4), 0, 0, S(4008fda4), 0, 0);   /* Confirm */
    ut( 96, S(4008ebf4), 0, 0, S(4008ec08), 0, 0, S(4008df50), 0, 0, S(4008f67c), 0, 0);   /* Calibratting ... */
    ut( 97, S(4008fdb0), 0, 0, S(4008fdbc), 0, 0, S(4008f68c), 0, 0, S(4008fdc8), 0, 0);   /* Succeed ! */
    ut(101, S(4008fdd4), 0, 0, S(4008fde0), 0, 0, S(4008ec1c), 0, 0, S(4008fdec), 0, 0);   /* Failed ! */
    ut( 98, S(4008f69c), 0, 0, S(4008f6ac), 0, 0, S(4008ec30), 0, 0, S(4008f6bc), 0, 0);   /* CH1 failed ! */
    ut( 99, S(4008f6cc), 0, 0, S(4008f6dc), 0, 0, S(4008ec44), 0, 0, S(4008f6ec), 0, 0);   /* CH2 failed ! */
    ut(100, S(4008f6fc), 0, 0, S(4008fde0), 0, 0, S(4008e5ec), 0, 0, S(4008f70c), 0, 0);   /* Failed ! */
    ut(102, S(4008e604), 0, 0, S(4008ec58), 0, 0, S(4008d81c), 0, 0, S(4008ec6c), 10, 0);   /* Restore to factory */
    ut(103, "", 0, 0, S(4008fdf8), 0, 0, "", 0, 0, S(4008fe04), 10, 0);   /* settings ? */
    ut(104, "", 0, 0, "", 0, 0, "", 0, 0, "", 0, 0);   /*  */
    ut(105, S(4008fe10), 0, 0, S(40090230), 0, 0, S(4008f71c), 0, 0, S(4008fe1c), 0, 0);   /* Done ! */
    ut(106, S(4008fe28), 0, 0, S(4008fe34), 0, 0, S(4008ec80), 0, 0, S(40090218), 0, 0);   /* Warning ! */
    ut(107, S(4008dae0), 0, 0, S(4008ec94), 0, 0, S(4008db00), -13, -4, S(4008e61c), -8, -4);   /* All saved data will */
    ut(108, S(4008eca8), 0, 0, S(4008fe40), 0, 0, S(4008d5f8), -13, -4, S(4008ecbc), -8, -4);   /* be clear ! */
    ut(109, "", 0, 0, "", 0, 0, S(4008df6c), -13, -4, S(4008ecd0), -8, -4);   /*  */
    ut(110, S(4008f72c), 0, 0, S(4008f73c), 0, 0, S(4008db20), 0, 0, S(4008e634), 0, 0);   /* Picture browser */
    ut(111, S(4008f74c), 0, 0, S(4008ece4), 0, 0, S(4008d3b0), 0, 0, S(4008e64c), 0, 0);   /* Waveform browser */
    ut(112, S(4008f75c), 0, 0, S(4008f76c), 0, 0, S(4008d840), 0, 0, S(4008df88), 0, 0);   /* Capture browser */
    ut(113, S(40090238), 0, 0, S(40090240), 0, 0, S(4008fe4c), 0, 0, S(4008fe58), 0, 0);   /* Select */
    ut(114, S(40090248), 0, 0, S(4008fe64), 0, 0, S(4008ecf8), 0, 0, S(4008f77c), 0, 0);   /* Select all */
    ut(115, S(40090250), 0, 0, S(40090258), 0, 0, S(4008ed0c), 0, 0, S(40090260), 0, 0);   /* Delete */
    ut(116, S(4008fe70), 0, 0, S(4008fe7c), 0, 0, S(4008ed20), 0, 0, S(4008fe88), 0, 0);   /* Last page */
    ut(117, S(4008fe94), 0, 0, S(4008fea0), 0, 0, S(4008ed34), 0, 0, S(4008feac), 0, 0);   /* Next page */
    ut(118, S(4008f78c), 0, 0, S(4008f79c), 0, 0, S(4008ed0c), 0, 0, S(40090260), 0, 0);   /* Delete selected */
    ut(119, S(40090268), 0, 0, S(40090270), 0, 0, S(4008feb8), -10, 0, S(40090270), 0, 0);   /* Total */
    ut(120, S(4008fec4), 0, 0, S(40090278), 0, 0, S(4008f7ac), 0, 0, S(40090280), 0, 0);   /* items */
    ut(121, "\351\241\265", 0, 0, S(40090288), 0, 0, S(4008f7bc), 0, 0, S(4008fed0), 0, 0);   /* pages */
    ut(122, "\351\241\265", 0, 0, S(40090290), 8, 0, S(4008f7bc), 0, 0, S(40090298), 8, 0);   /* page */
    ut(123, S(400902a0), 20, 0, S(400902a8), 0, 0, S(4008f7cc), -15, 0, S(400902b0), 0, 0);   /* Current */
    ut(124, S(4008e664), 0, 0, S(4008ed48), 0, 0, S(4008d0ec), 0, 0, S(4008dfa4), 0, 0);   /* Waveform saving ... */
    ut(125, S(4008e67c), 0, 0, S(4008e694), 0, 0, S(4008d11c), 0, 0, S(4008ed5c), 0, 0);   /* Screenshut saving ... */
    ut(126, S(4008d620), 0, 0, S(4008dfc0), 0, 0, S(4008d3dc), 0, 0, S(4008dfdc), 9, 0);   /* Please connect all channels */
    ut(127, S(4008fedc), 0, 0, S(4008dff8), 0, 0, S(4008d648), 0, 0, S(4008ed70), 9, 0);   /* to the signal generator ! */
    ut(128, "", 0, 0, "", 0, 0, "", 0, 0, "", 0, 0);   /*  */
    ut(129, S(4008d864), 0, 0, S(4008d670), 0, 0, S(4008c9d4), 0, 0, S(4008ced8), 0, 0);   /* No signal detected in all channels ! */
    ut(130, S(4008e014), 0, 0, S(4008d698), 0, 0, S(4008cdb0), 0, 0, S(4008d14c), 0, 0);   /* No signal detected in all channel 1 ! */
    ut(131, S(4008e030), 0, 0, S(4008d6c0), 0, 0, S(4008cdec), 0, 0, S(4008d17c), 0, 0);   /* No signal detected in all channel 2 ! */
    ut(132, S(4008e04c), 0, 0, S(4008e6ac), 0, 0, S(4008e068), 0, 0, S(4008d6e8), 0, 0);   /* No signal detected ! */
    ut(133, S(4008e6c4), 0, 0, S(4008e6dc), 0, 0, S(4008cd30), 0, 0, S(4008d888), 0, 0);   /* Disk space is full ! */
    ut(134, S(4008ed84), 0, 0, S(4008fee8), 0, 0, S(4008fef4), 0, 0, S(4008ff00), 0, 0);   /* Autoset ... */
    ut(135, S(4008f7dc), 0, 0, S(4008f7ec), 0, 0, S(4008db40), 0, 0, S(4008ed98), 0, 0);   /* Auto shutdown ! */
    ut(136, S(4008e6f4), 0, 0, S(4008e70c), 0, 0, S(4008d1ac), 0, 0, S(4008edac), 0, 0);   /* Low battery shutdown ! */
    ut(137, S(4008ff0c), 0, 0, S(4008ff18), 0, 0, S(4008f7fc), 0, 0, S(4008f80c), 0, 0);   /* FFT length */
    ut(138, S(4008f81c), 0, 0, S(4008edc0), 0, 0, S(4008e084), 0, 0, S(4008edd4), 0, 0);   /* Waterfall_display */
    ut(139, S(4008ff24), 0, 0, S(4008ff30), 0, 0, S(4008d408), 0, 0, S(4008ede8), 0, 0);   /* 3D display */
    ut(140, S(400902b8), 0, 0, S(40048ef4), 0, 0, S(4008ff3c), 0, 0, S(4008ff48), 0, 0);   /* OFF */
}
