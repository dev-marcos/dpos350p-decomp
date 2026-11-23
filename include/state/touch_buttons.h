/* struct touch_button: the touch button records: record k is at 0x42E61FF8 + 2 + 10k (build 2025-06-19).
 *
 * Field meanings are DERIVED from the code that reads and writes them (docs/DRIVERS.md).
 * Access a field of record i with BREC(i, field): a volatile access at
 * 0x42E61FF8 + 2 + 10 i + offsetof(field), constant when i is. */
#ifndef TOUCH_BUTTON_H
#define TOUCH_BUTTON_H

#include <stddef.h>

#define BREC_BASE 0x42E61FFAu

struct touch_button {
    unsigned char state;                 /* +0x0 1 on a press, 2 on a release */
    unsigned char touch_code;            /* +0x1 copy of touch byte +0xD */
    unsigned short x0;                   /* +0x2 */
    unsigned short y0;                   /* +0x4 */
    unsigned short x1;                   /* +0x6 */
    unsigned short y1;                   /* +0x8 */
};

_Static_assert(sizeof(struct touch_button) == 10, "record size");

#define BREC(i, f) (*(volatile __typeof__(((struct touch_button *)0)->f) *)(BREC_BASE + 10u * (i) + offsetof(struct touch_button, f)))

/* The byte before record 0. It is cleared together with a released button; its meaning is
 * OPEN. */
#define BREC_BYTE0 (*(volatile unsigned char *)0x42E61FF8u)

#endif
