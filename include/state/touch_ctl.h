/* struct touch_ctl: the touch-control state (control code, drag start), at 0x42E68998 (build 2025-06-19).
 *
 * Only the fields the C code reaches with a constant offset are listed; the gaps are padding.
 * A named field carries its evidence level and source; the others keep their offset as their
 * name. Access a field with TCS(field), a volatile access at the block address + offsetof, like
 * SI(field) for SYS_INFO (see include/sys_info.h). */
#ifndef TOUCH_CTL_H
#define TOUCH_CTL_H

#include <stddef.h>

#define TCS_BASE 0x42E68998u

struct touch_ctl {
    unsigned char code;                  /* +0x000 DERIVED: control code of the current touch (0x12 when moved) */
    unsigned char _pad0[0x3];
    unsigned int cursor_limit;           /* +0x004 DERIVED: limit for the cursors */
    long long drag_start;                /* +0x008 DERIVED: s64 start kept on press by the drag buttons */
    long long f_010;                     /* +0x010 */
    long long f_018;                     /* +0x018 */
    unsigned char _pad1[0x10];
    unsigned int drag_dx;                /* +0x030 DERIVED: second point minus first, x */
    unsigned int drag_dy;                /* +0x034 DERIVED: second point minus first, y */
    unsigned int trace_dist_0;           /* +0x038 DERIVED: touch-to-trace distance, trace 0 */
    unsigned int trace_dist_1;           /* +0x03C DERIVED: touch-to-trace distance, trace 1 */
    unsigned int trace_dist_2;           /* +0x040 DERIVED: touch-to-trace distance, trace 2 (split view) */
    unsigned int trace_dist_3;           /* +0x044 DERIVED: touch-to-trace distance, trace 3 */
    unsigned int x_cursor_1;             /* +0x048 DERIVED: x cursor */
    unsigned int x_cursor_2;             /* +0x04C DERIVED: x cursor */
    unsigned int f_050;                  /* +0x050 */
    unsigned int f_054;                  /* +0x054 */
    unsigned int f_058;                  /* +0x058 */
    unsigned int f_05c;                  /* +0x05C */
    unsigned int f_060;                  /* +0x060 */
    unsigned int f_064;                  /* +0x064 */
    unsigned int f_068;                  /* +0x068 */
    unsigned int f_06c;                  /* +0x06C */
    unsigned int f_070;                  /* +0x070 */
    unsigned int f_074;                  /* +0x074 */
};

_Static_assert(offsetof(struct touch_ctl, code) == 0x0, "code");
_Static_assert(offsetof(struct touch_ctl, cursor_limit) == 0x4, "cursor_limit");
_Static_assert(offsetof(struct touch_ctl, drag_start) == 0x8, "drag_start");
_Static_assert(offsetof(struct touch_ctl, f_010) == 0x10, "f_010");
_Static_assert(offsetof(struct touch_ctl, f_018) == 0x18, "f_018");
_Static_assert(offsetof(struct touch_ctl, drag_dx) == 0x30, "drag_dx");
_Static_assert(offsetof(struct touch_ctl, drag_dy) == 0x34, "drag_dy");
_Static_assert(offsetof(struct touch_ctl, trace_dist_0) == 0x38, "trace_dist_0");
_Static_assert(offsetof(struct touch_ctl, trace_dist_1) == 0x3C, "trace_dist_1");
_Static_assert(offsetof(struct touch_ctl, trace_dist_2) == 0x40, "trace_dist_2");
_Static_assert(offsetof(struct touch_ctl, trace_dist_3) == 0x44, "trace_dist_3");
_Static_assert(offsetof(struct touch_ctl, x_cursor_1) == 0x48, "x_cursor_1");
_Static_assert(offsetof(struct touch_ctl, x_cursor_2) == 0x4C, "x_cursor_2");
_Static_assert(offsetof(struct touch_ctl, f_050) == 0x50, "f_050");
_Static_assert(offsetof(struct touch_ctl, f_054) == 0x54, "f_054");
_Static_assert(offsetof(struct touch_ctl, f_058) == 0x58, "f_058");
_Static_assert(offsetof(struct touch_ctl, f_05c) == 0x5C, "f_05c");
_Static_assert(offsetof(struct touch_ctl, f_060) == 0x60, "f_060");
_Static_assert(offsetof(struct touch_ctl, f_064) == 0x64, "f_064");
_Static_assert(offsetof(struct touch_ctl, f_068) == 0x68, "f_068");
_Static_assert(offsetof(struct touch_ctl, f_06c) == 0x6C, "f_06c");
_Static_assert(offsetof(struct touch_ctl, f_070) == 0x70, "f_070");
_Static_assert(offsetof(struct touch_ctl, f_074) == 0x74, "f_074");

#define TCS(f) (*(volatile __typeof__(((struct touch_ctl *)0)->f) *)(TCS_BASE + offsetof(struct touch_ctl, f)))

#endif
