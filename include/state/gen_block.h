/* struct gen_block: the generator block G (initialised by the block initialiser, saved in part as record G), at 0x42E3F984 (build 2025-06-19).
 *
 * Only the fields the C code reaches with a constant offset are listed; the gaps are padding.
 * A named field carries its evidence level and source; the others keep their offset as their
 * name. Access a field with GENS(field), a volatile access at the block address + offsetof, like
 * SI(field) for SYS_INFO (see include/sys_info.h). */
#ifndef GEN_BLOCK_H
#define GEN_BLOCK_H

#include <stddef.h>

#define GENS_BASE 0x42E3F984u

struct gen_block {
    unsigned char _pad0[0x1];
    unsigned char f_001;                 /* +0x001 */
    unsigned char _pad1[0x2];
    unsigned int f_004;                  /* +0x004 */
    unsigned int f_008;                  /* +0x008 */
    unsigned char f_00c;                 /* +0x00C */
    unsigned char _pad2[0xFA1];
    unsigned short f_fae;                /* +0xFAE */
    unsigned short f_fb0;                /* +0xFB0 */
    unsigned char _pad3[0x2];
    unsigned short f_fb4;                /* +0xFB4 */
    unsigned char _pad4[0x1F40];
    unsigned short period_len;           /* +0x2EF6 DERIVED: samples in one period of the output table (G+0xFB6) */
    unsigned char _pad5[0x2];
    unsigned char waveform;              /* +0x2EFA DERIVED: waveform byte of the GEN menu */
    unsigned char _pad6[0x1];
    unsigned int frequency;              /* +0x2EFC DERIVED: start of the settings block the GEN menu reads (frequency) */
    unsigned int amplitude;              /* +0x2F00 DERIVED: amplitude word (the Bode sweep copies its amplitude here) */
    unsigned int offset;                 /* +0x2F04 DERIVED: offset word (the Bode sweep copies its offset here) */
    unsigned int f_2f08;                 /* +0x2F08 */
    unsigned char f_2f0c;                /* +0x2F0C */
    unsigned char _pad7[0x3];
    unsigned int f_2f10;                 /* +0x2F10 */
    unsigned int f_2f14;                 /* +0x2F14 */
    unsigned int f_2f18;                 /* +0x2F18 */
    unsigned int f_2f1c;                 /* +0x2F1C */
    unsigned int f_2f20;                 /* +0x2F20 */
    unsigned int f_2f24;                 /* +0x2F24 */
    unsigned int f_2f28;                 /* +0x2F28 */
    unsigned char _pad8[0x8];
    unsigned int f_2f34;                 /* +0x2F34 */
    unsigned int f_2f38;                 /* +0x2F38 */
    unsigned int f_2f3c;                 /* +0x2F3C */
    unsigned int f_2f40;                 /* +0x2F40 */
    unsigned char _pad9[0x2];
    unsigned short f_2f46;               /* +0x2F46 */
    unsigned int f_2f48;                 /* +0x2F48 */
    unsigned int f_2f4c;                 /* +0x2F4C */
    unsigned int f_2f50;                 /* +0x2F50 */
    unsigned int f_2f54;                 /* +0x2F54 */
    unsigned int f_2f58;                 /* +0x2F58 */
    unsigned int f_2f5c;                 /* +0x2F5C */
    unsigned char f_2f60;                /* +0x2F60 */
    unsigned char _pad10[0x3];
    float rec_g_0;                       /* +0x2F64 DERIVED: first of the three words saved as settings record G */
    float rec_g_1;                       /* +0x2F68 DERIVED: second word of record G */
    float rec_g_2;                       /* +0x2F6C DERIVED: third word of record G */
};

_Static_assert(offsetof(struct gen_block, f_001) == 0x1, "f_001");
_Static_assert(offsetof(struct gen_block, f_004) == 0x4, "f_004");
_Static_assert(offsetof(struct gen_block, f_008) == 0x8, "f_008");
_Static_assert(offsetof(struct gen_block, f_00c) == 0xC, "f_00c");
_Static_assert(offsetof(struct gen_block, f_fae) == 0xFAE, "f_fae");
_Static_assert(offsetof(struct gen_block, f_fb0) == 0xFB0, "f_fb0");
_Static_assert(offsetof(struct gen_block, f_fb4) == 0xFB4, "f_fb4");
_Static_assert(offsetof(struct gen_block, period_len) == 0x2EF6, "period_len");
_Static_assert(offsetof(struct gen_block, waveform) == 0x2EFA, "waveform");
_Static_assert(offsetof(struct gen_block, frequency) == 0x2EFC, "frequency");
_Static_assert(offsetof(struct gen_block, amplitude) == 0x2F00, "amplitude");
_Static_assert(offsetof(struct gen_block, offset) == 0x2F04, "offset");
_Static_assert(offsetof(struct gen_block, f_2f08) == 0x2F08, "f_2f08");
_Static_assert(offsetof(struct gen_block, f_2f0c) == 0x2F0C, "f_2f0c");
_Static_assert(offsetof(struct gen_block, f_2f10) == 0x2F10, "f_2f10");
_Static_assert(offsetof(struct gen_block, f_2f14) == 0x2F14, "f_2f14");
_Static_assert(offsetof(struct gen_block, f_2f18) == 0x2F18, "f_2f18");
_Static_assert(offsetof(struct gen_block, f_2f1c) == 0x2F1C, "f_2f1c");
_Static_assert(offsetof(struct gen_block, f_2f20) == 0x2F20, "f_2f20");
_Static_assert(offsetof(struct gen_block, f_2f24) == 0x2F24, "f_2f24");
_Static_assert(offsetof(struct gen_block, f_2f28) == 0x2F28, "f_2f28");
_Static_assert(offsetof(struct gen_block, f_2f34) == 0x2F34, "f_2f34");
_Static_assert(offsetof(struct gen_block, f_2f38) == 0x2F38, "f_2f38");
_Static_assert(offsetof(struct gen_block, f_2f3c) == 0x2F3C, "f_2f3c");
_Static_assert(offsetof(struct gen_block, f_2f40) == 0x2F40, "f_2f40");
_Static_assert(offsetof(struct gen_block, f_2f46) == 0x2F46, "f_2f46");
_Static_assert(offsetof(struct gen_block, f_2f48) == 0x2F48, "f_2f48");
_Static_assert(offsetof(struct gen_block, f_2f4c) == 0x2F4C, "f_2f4c");
_Static_assert(offsetof(struct gen_block, f_2f50) == 0x2F50, "f_2f50");
_Static_assert(offsetof(struct gen_block, f_2f54) == 0x2F54, "f_2f54");
_Static_assert(offsetof(struct gen_block, f_2f58) == 0x2F58, "f_2f58");
_Static_assert(offsetof(struct gen_block, f_2f5c) == 0x2F5C, "f_2f5c");
_Static_assert(offsetof(struct gen_block, f_2f60) == 0x2F60, "f_2f60");
_Static_assert(offsetof(struct gen_block, rec_g_0) == 0x2F64, "rec_g_0");
_Static_assert(offsetof(struct gen_block, rec_g_1) == 0x2F68, "rec_g_1");
_Static_assert(offsetof(struct gen_block, rec_g_2) == 0x2F6C, "rec_g_2");

#define GENS(f) (*(volatile __typeof__(((struct gen_block *)0)->f) *)(GENS_BASE + offsetof(struct gen_block, f)))

#endif
