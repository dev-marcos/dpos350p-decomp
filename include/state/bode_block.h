/* struct bode_block: the Bode (frequency response) view block (backed up, 0xAD20 bytes), at 0x42E180C8 (build 2025-06-19).
 *
 * Only the fields the C code reaches with a constant offset are listed; the gaps are padding.
 * A named field carries its evidence level and source; the others keep their offset as their
 * name. Access a field with BODES(field), a volatile access at the block address + offsetof, like
 * SI(field) for SYS_INFO (see include/sys_info.h). */
#ifndef BODE_BLOCK_H
#define BODE_BLOCK_H

#include <stddef.h>

#define BODES_BASE 0x42E180C8u

struct bode_block {
    unsigned char _pad0[0x4];
    unsigned short f_004;                /* +0x004 */
    unsigned short f_006;                /* +0x006 */
    unsigned char _pad1[0x8];
    double f_010;                        /* +0x010 */
    double f_018;                        /* +0x018 */
    double f_020;                        /* +0x020 */
    double f_028;                        /* +0x028 */
    unsigned char _pad2[0x10];
    unsigned int f_040;                  /* +0x040 */
    unsigned char loop;                  /* +0x044 DERIVED: 'Single' / 'Loop' */
    unsigned char running;               /* +0x045 DERIVED: run indicator of the Bode toolbar */
    unsigned char move_slow;             /* +0x046 DERIVED: 'Movement': Fast move / Slow move */
    unsigned char f_047;                 /* +0x047 */
    int f_048;                           /* +0x048 */
    int amplitude;                       /* +0x04C DERIVED: 'Amplitude' (x 1000 uV) */
    int offset;                          /* +0x050 DERIVED: 'Offset' (x 1000 uV) */
    int start_freq;                      /* +0x054 DERIVED: 'Start frequency', Hz */
    int stop_freq;                       /* +0x058 DERIVED: 'Stop frequency', Hz */
    unsigned int freq_count;             /* +0x05C DERIVED: 'Frequency count' (log steps) */
    unsigned char _pad3[0x8CA8];
    double f_8d08;                       /* +0x8D08 */
    unsigned char _pad4[0x8];
    double f_8d18;                       /* +0x8D18 */
    unsigned char _pad5[0x1F40];
    unsigned char cursors_on;            /* +0xAC60 DERIVED: Bode cursors drawn */
    unsigned char _pad6[0x3];
    int cursor1;                         /* +0xAC64 DERIVED: Bode cursor mark 1 */
    int cursor2;                         /* +0xAC68 DERIVED: Bode cursor mark 2 */
    unsigned char _pad7[0x4];
    double f_ac70;                       /* +0xAC70 */
    double f_ac78;                       /* +0xAC78 */
    double f_ac80;                       /* +0xAC80 */
    double f_ac88;                       /* +0xAC88 */
    double f_ac90;                       /* +0xAC90 */
    double f_ac98;                       /* +0xAC98 */
    double f_aca0;                       /* +0xACA0 */
    double f_aca8;                       /* +0xACA8 */
    double f_acb0;                       /* +0xACB0 */
    unsigned char _pad8[0x60];
    unsigned char sweep_busy;            /* +0xAD18 DERIVED: set during the points (no event service) */
    unsigned char f_ad19;                /* +0xAD19 */
    unsigned char f_ad1a;                /* +0xAD1A */
};

_Static_assert(offsetof(struct bode_block, f_004) == 0x4, "f_004");
_Static_assert(offsetof(struct bode_block, f_006) == 0x6, "f_006");
_Static_assert(offsetof(struct bode_block, f_010) == 0x10, "f_010");
_Static_assert(offsetof(struct bode_block, f_018) == 0x18, "f_018");
_Static_assert(offsetof(struct bode_block, f_020) == 0x20, "f_020");
_Static_assert(offsetof(struct bode_block, f_028) == 0x28, "f_028");
_Static_assert(offsetof(struct bode_block, f_040) == 0x40, "f_040");
_Static_assert(offsetof(struct bode_block, loop) == 0x44, "loop");
_Static_assert(offsetof(struct bode_block, running) == 0x45, "running");
_Static_assert(offsetof(struct bode_block, move_slow) == 0x46, "move_slow");
_Static_assert(offsetof(struct bode_block, f_047) == 0x47, "f_047");
_Static_assert(offsetof(struct bode_block, f_048) == 0x48, "f_048");
_Static_assert(offsetof(struct bode_block, amplitude) == 0x4C, "amplitude");
_Static_assert(offsetof(struct bode_block, offset) == 0x50, "offset");
_Static_assert(offsetof(struct bode_block, start_freq) == 0x54, "start_freq");
_Static_assert(offsetof(struct bode_block, stop_freq) == 0x58, "stop_freq");
_Static_assert(offsetof(struct bode_block, freq_count) == 0x5C, "freq_count");
_Static_assert(offsetof(struct bode_block, f_8d08) == 0x8D08, "f_8d08");
_Static_assert(offsetof(struct bode_block, f_8d18) == 0x8D18, "f_8d18");
_Static_assert(offsetof(struct bode_block, cursors_on) == 0xAC60, "cursors_on");
_Static_assert(offsetof(struct bode_block, cursor1) == 0xAC64, "cursor1");
_Static_assert(offsetof(struct bode_block, cursor2) == 0xAC68, "cursor2");
_Static_assert(offsetof(struct bode_block, f_ac70) == 0xAC70, "f_ac70");
_Static_assert(offsetof(struct bode_block, f_ac78) == 0xAC78, "f_ac78");
_Static_assert(offsetof(struct bode_block, f_ac80) == 0xAC80, "f_ac80");
_Static_assert(offsetof(struct bode_block, f_ac88) == 0xAC88, "f_ac88");
_Static_assert(offsetof(struct bode_block, f_ac90) == 0xAC90, "f_ac90");
_Static_assert(offsetof(struct bode_block, f_ac98) == 0xAC98, "f_ac98");
_Static_assert(offsetof(struct bode_block, f_aca0) == 0xACA0, "f_aca0");
_Static_assert(offsetof(struct bode_block, f_aca8) == 0xACA8, "f_aca8");
_Static_assert(offsetof(struct bode_block, f_acb0) == 0xACB0, "f_acb0");
_Static_assert(offsetof(struct bode_block, sweep_busy) == 0xAD18, "sweep_busy");
_Static_assert(offsetof(struct bode_block, f_ad19) == 0xAD19, "f_ad19");
_Static_assert(offsetof(struct bode_block, f_ad1a) == 0xAD1A, "f_ad1a");

#define BODES(f) (*(volatile __typeof__(((struct bode_block *)0)->f) *)(BODES_BASE + offsetof(struct bode_block, f)))

#endif
