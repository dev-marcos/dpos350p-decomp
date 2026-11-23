/* struct factory_block: the factory / self-test state (settings record E; word +0 is the magic 0x5A5555A5), at 0x42E68AE0 (build 2025-06-19).
 *
 * Only the fields the C code reaches with a constant offset are listed; the gaps are padding.
 * A named field carries its evidence level and source; the others keep their offset as their
 * name. Access a field with FACT(field), a volatile access at the block address + offsetof, like
 * SI(field) for SYS_INFO (see include/sys_info.h). */
#ifndef FACTORY_BLOCK_H
#define FACTORY_BLOCK_H

#include <stddef.h>

#define FACT_BASE 0x42E68AE0u

struct factory_block {
    unsigned int magic;                  /* +0x000 DERIVED: 0x5A5555A5 when the factory flow is complete */
    unsigned int f_004;                  /* +0x004 */
    unsigned int f_008;                  /* +0x008 */
    unsigned int f_00c;                  /* +0x00C */
    unsigned char f_010;                 /* +0x010 */
    unsigned char fpga_probe_ok;         /* +0x011 DERIVED: copy of SYS_INFO fpga_probe_result */
    unsigned char security_chip_ok;      /* +0x012 DERIVED: result of security_chip_check */
    unsigned char f_013;                 /* +0x013 */
    unsigned char _pad0[0x4];
    unsigned char f_018;                 /* +0x018 */
    unsigned char _pad1[0x8];
    unsigned char f_021;                 /* +0x021 */
    unsigned char f_022;                 /* +0x022 */
    unsigned char f_023;                 /* +0x023 */
    unsigned char f_024;                 /* +0x024 */
    unsigned char ch1_checks_ok;         /* +0x025 DERIVED: all CH1 checks passed */
    unsigned char ch2_checks_ok;         /* +0x026 DERIVED: all CH2 checks passed */
    unsigned char f_027;                 /* +0x027 */
    unsigned char f_028;                 /* +0x028 */
    unsigned char f_029;                 /* +0x029 */
    unsigned char _pad2[0x4];
    unsigned char f_02e;                 /* +0x02E */
    unsigned char f_02f;                 /* +0x02F */
    unsigned char f_030;                 /* +0x030 */
    unsigned char _pad3[0x3];
    unsigned char f_034;                 /* +0x034 */
    unsigned char window_a_ok;           /* +0x035 DERIVED: AND of the five window results A */
    unsigned char window_b_ok;           /* +0x036 DERIVED: AND of the five window results B */
    unsigned char _pad4[0x8];
    unsigned char f_03f;                 /* +0x03F */
    unsigned char f_040;                 /* +0x040 */
    unsigned char f_041;                 /* +0x041 */
    unsigned char f_042;                 /* +0x042 */
    unsigned char f_043;                 /* +0x043 */
    unsigned char f_044;                 /* +0x044 */
    unsigned char f_045;                 /* +0x045 */
};

_Static_assert(offsetof(struct factory_block, magic) == 0x0, "magic");
_Static_assert(offsetof(struct factory_block, f_004) == 0x4, "f_004");
_Static_assert(offsetof(struct factory_block, f_008) == 0x8, "f_008");
_Static_assert(offsetof(struct factory_block, f_00c) == 0xC, "f_00c");
_Static_assert(offsetof(struct factory_block, f_010) == 0x10, "f_010");
_Static_assert(offsetof(struct factory_block, fpga_probe_ok) == 0x11, "fpga_probe_ok");
_Static_assert(offsetof(struct factory_block, security_chip_ok) == 0x12, "security_chip_ok");
_Static_assert(offsetof(struct factory_block, f_013) == 0x13, "f_013");
_Static_assert(offsetof(struct factory_block, f_018) == 0x18, "f_018");
_Static_assert(offsetof(struct factory_block, f_021) == 0x21, "f_021");
_Static_assert(offsetof(struct factory_block, f_022) == 0x22, "f_022");
_Static_assert(offsetof(struct factory_block, f_023) == 0x23, "f_023");
_Static_assert(offsetof(struct factory_block, f_024) == 0x24, "f_024");
_Static_assert(offsetof(struct factory_block, ch1_checks_ok) == 0x25, "ch1_checks_ok");
_Static_assert(offsetof(struct factory_block, ch2_checks_ok) == 0x26, "ch2_checks_ok");
_Static_assert(offsetof(struct factory_block, f_027) == 0x27, "f_027");
_Static_assert(offsetof(struct factory_block, f_028) == 0x28, "f_028");
_Static_assert(offsetof(struct factory_block, f_029) == 0x29, "f_029");
_Static_assert(offsetof(struct factory_block, f_02e) == 0x2E, "f_02e");
_Static_assert(offsetof(struct factory_block, f_02f) == 0x2F, "f_02f");
_Static_assert(offsetof(struct factory_block, f_030) == 0x30, "f_030");
_Static_assert(offsetof(struct factory_block, f_034) == 0x34, "f_034");
_Static_assert(offsetof(struct factory_block, window_a_ok) == 0x35, "window_a_ok");
_Static_assert(offsetof(struct factory_block, window_b_ok) == 0x36, "window_b_ok");
_Static_assert(offsetof(struct factory_block, f_03f) == 0x3F, "f_03f");
_Static_assert(offsetof(struct factory_block, f_040) == 0x40, "f_040");
_Static_assert(offsetof(struct factory_block, f_041) == 0x41, "f_041");
_Static_assert(offsetof(struct factory_block, f_042) == 0x42, "f_042");
_Static_assert(offsetof(struct factory_block, f_043) == 0x43, "f_043");
_Static_assert(offsetof(struct factory_block, f_044) == 0x44, "f_044");
_Static_assert(offsetof(struct factory_block, f_045) == 0x45, "f_045");

#define FACT(f) (*(volatile __typeof__(((struct factory_block *)0)->f) *)(FACT_BASE + offsetof(struct factory_block, f)))

#endif
