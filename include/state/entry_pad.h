/* struct entry_pad: the numeric entry pad state (window 0x21), at 0x42E68A88 (build 2025-06-19).
 *
 * Only the fields the C code reaches with a constant offset are listed; the gaps are padding.
 * A named field carries its evidence level and source; the others keep their offset as their
 * name. Access a field with NUMPAD(field), a volatile access at the block address + offsetof, like
 * SI(field) for SYS_INFO (see include/sys_info.h). */
#ifndef ENTRY_PAD_H
#define ENTRY_PAD_H

#include <stddef.h>

#define NUMPAD_BASE 0x42E68A88u

struct entry_pad {
    unsigned char _pad0[0x1];
    unsigned char mode;                  /* +0x001 DERIVED: entry mode set by numpad_open_mode* */
    unsigned char f_002;                 /* +0x002 */
    unsigned char f_003;                 /* +0x003 */
    unsigned char _pad1[0x13];
    unsigned char len;                   /* +0x017 DERIVED: length of the text at +3 */
    unsigned char has_dot;               /* +0x018 DERIVED: '.' entered */
    unsigned char has_minus;             /* +0x019 DERIVED: '-' entered */
    unsigned char _pad2[0x6];
    double value;                        /* +0x020 DERIVED: entered value */
    unsigned int target_0;               /* +0x028 DERIVED: first of three target pointers */
    unsigned int target_1;               /* +0x02C DERIVED: second target pointer */
    unsigned int target_2;               /* +0x030 DERIVED: third target pointer */
    unsigned char _pad3[0x18];
    unsigned char f_04c;                 /* +0x04C */
    unsigned char _pad4[0x3];
    unsigned int f_050;                  /* +0x050 */
};

_Static_assert(offsetof(struct entry_pad, mode) == 0x1, "mode");
_Static_assert(offsetof(struct entry_pad, f_002) == 0x2, "f_002");
_Static_assert(offsetof(struct entry_pad, f_003) == 0x3, "f_003");
_Static_assert(offsetof(struct entry_pad, len) == 0x17, "len");
_Static_assert(offsetof(struct entry_pad, has_dot) == 0x18, "has_dot");
_Static_assert(offsetof(struct entry_pad, has_minus) == 0x19, "has_minus");
_Static_assert(offsetof(struct entry_pad, value) == 0x20, "value");
_Static_assert(offsetof(struct entry_pad, target_0) == 0x28, "target_0");
_Static_assert(offsetof(struct entry_pad, target_1) == 0x2C, "target_1");
_Static_assert(offsetof(struct entry_pad, target_2) == 0x30, "target_2");
_Static_assert(offsetof(struct entry_pad, f_04c) == 0x4C, "f_04c");
_Static_assert(offsetof(struct entry_pad, f_050) == 0x50, "f_050");

#define NUMPAD(f) (*(volatile __typeof__(((struct entry_pad *)0)->f) *)(NUMPAD_BASE + offsetof(struct entry_pad, f)))

#endif
