/* struct ui_window: the UI window / layout records: record id is at 0x42E64520 + 14 id (build 2025-06-19).
 *
 * Field meanings are DERIVED from the code that reads and writes them (docs/DRIVERS.md).
 * Access a field of record i with WREC(i, field): a volatile access at
 * 0x42E64520 + 0 + 14 i + offsetof(field), constant when i is. */
#ifndef UI_WINDOW_H
#define UI_WINDOW_H

#include <stddef.h>

#define WREC_BASE 0x42E64520u

struct ui_window {
    unsigned char open;                  /* +0x0 set by ui_window_open */
    unsigned char dirty;                 /* +0x1 */
    unsigned char self_contained;        /* +0x2 when 0 the window is redrawn when one below sets +4 */
    unsigned char animate;               /* +0x3 animate on the next redraw */
    unsigned char f_4;                   /* +0x4 */
    unsigned char f_5;                   /* +0x5 */
    unsigned short x;                    /* +0x6 */
    unsigned short y;                    /* +0x8 */
    unsigned short w;                    /* +0xA */
    unsigned short h;                    /* +0xC */
};

_Static_assert(sizeof(struct ui_window) == 14, "record size");

#define WREC(i, f) (*(volatile __typeof__(((struct ui_window *)0)->f) *)(WREC_BASE + 14u * (i) + offsetof(struct ui_window, f)))

#endif
