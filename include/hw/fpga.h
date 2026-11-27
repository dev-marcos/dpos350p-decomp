/* FPGA control protocol, SMEC jobs, touch service, power and disk glue (Phase 5 batch 11).
 * Addresses are STATIC-CONFIRMED from the stock code; names are DERIVED unless noted.
 * FPGA bus primitives and selectors: Hack docs/FPGA_PROTOCOL.md; SMEC: Hack docs/SMEC.md. */
#ifndef HW_FPGA_H
#define HW_FPGA_H
#include "hw/v3s.h"

#define SYS8(o)   (*(volatile unsigned char *)(0x40544E68u + (o)))
#define SYS16(o)  (*(volatile unsigned short *)(0x40544E68u + (o)))
#define SYS32(o)  (*(volatile u32 *)(0x40544E68u + (o)))
#define g_tick_count  (*(volatile u32 *)0x42E68CD8u)   /* 1 ms tick (timer0_isr) */

/* ---- FPGA parallel bus (already decompiled, batch 8/drivers) */
#define g_pe_mode  (*(volatile unsigned char *)0x42E68D19u)   /* shared PE pins: 0 = FPGA control bus, else CSI capture */

/* ---- SMEC job interface: parameters go to the FPGA as (index, 32-bit value) jobs.
 * Selectors: 0x69 timing (16 bit), 0x6A index, 0x6B value (32 bit, MSB first), 0x68 start/echo,
 * 0x64 busy->done, 0x65 ready, 0x67 result valid (bit 0) / reset (write 8), 0x66 result (32 bit). */
typedef struct {
    unsigned char _pad0[0x3FC];
    u32 cache[255];                 /* +0x3FC  last value sent per job index */
    unsigned char valid[255];       /* +0x7F8  cache[] holds a sent value */
    unsigned char pending;          /* +0x8F7  a job was sent since the last commit */
    unsigned char secure_ok;        /* +0x8F8  security chip status (selector 0x60 bit 0) */
    unsigned char _pad1[3];
} smec_state_t;                     /* 0x8FC bytes, cleared by SMEC_Init */
_Static_assert(__builtin_offsetof(smec_state_t, cache) == 0x3FC, "cache");
_Static_assert(__builtin_offsetof(smec_state_t, valid) == 0x7F8, "valid");
_Static_assert(__builtin_offsetof(smec_state_t, pending) == 0x8F7, "pending");
_Static_assert(__builtin_offsetof(smec_state_t, secure_ok) == 0x8F8, "secure_ok");
_Static_assert(sizeof(smec_state_t) == 0x8FC, "size");
#define g_smec        ((smec_state_t *)0x42E62C2Cu)
#define g_smec_timing ((volatile u32 *)0x42E67558u)   /* per-index timing, 2000..3000 */


/* ---- touch panel state (GT911, polled) */
typedef struct {
    short x, y;            /* +0x00 current point */
    short down_x, down_y;  /* +0x04 point of the last press */
    short up_x, up_y;      /* +0x08 point of the last release */
    unsigned char event;   /* +0x0C 1 press, 2 release */
    unsigned char touched; /* +0x0D */
    unsigned char prev;    /* +0x0E touched at the previous poll */
    unsigned char _pad;
    short prev_x, prev_y;  /* +0x10 */
    unsigned char busy;    /* +0x14 event waiting for the UI */
    unsigned char busy2;   /* +0x15 */
    unsigned char enabled; /* +0x16 */
    unsigned char serviced;/* +0x17 */
} touch_t;
_Static_assert(__builtin_offsetof(touch_t, event) == 0x0C, "event");
_Static_assert(__builtin_offsetof(touch_t, prev_x) == 0x10, "prev_x");
_Static_assert(__builtin_offsetof(touch_t, busy) == 0x14, "busy");
#define g_touch          ((volatile touch_t *)0x42E68BB0u)
#define g_touch_last     (*(volatile u32 *)0x42E68C70u)   /* tick of the last poll */
#define g_touch_hold     (*(volatile unsigned char *)0x42E61FF8u)    /* repeat while held */

/* ---- power */

/* ---- battery monitor (batch 12) */
#define g_batt_sum     (*(volatile u32 *)0x42E68CA0u)   /* LRADC readings accumulated */
#define g_batt_avg     (*(volatile int *)0x42E68CA4u)   /* averaged LRADC code */
#define g_batt_count   (*(volatile u32 *)0x42E68CA8u)
#define g_batt_last    (*(volatile u32 *)0x42E68CACu)   /* tick of the last average */
#define g_batt_slope   (*(volatile float *)0x42E68C94u)
#define g_batt_base    (*(volatile float *)0x42E68C98u)
#define g_batt_offset  (*(volatile float *)0x42E68C9Cu)

/* ---- front end set-up (batch 12) */
#define SYS64(o)  (*(volatile unsigned long long *)(0x40544E68u + (o)))
#define CAL_OBJ   ((volatile u32 *)0x42E53A0Cu)          /* calibration object: aux values at +0..+0xC */
long long __aeabi_ldivmod(long long n, long long d);       /* 0x4003453C */

/* ---- signal generator (batch 13). State block G = 0x42E3F984; the settings block at G+0x2EFC
 * (0x42E42880: frequency, amplitude, offset, duty, ...) is the one the GEN menu edits. */
#define GEN_G        0x42E3F984u
#define G16(o)       (*(volatile unsigned short *)(GEN_G + (o)))
#define G32(o)       (*(volatile u32 *)(GEN_G + (o)))
#define GS32(o)      (*(volatile int *)(GEN_G + (o)))
#define GF32(o)      (*(volatile float *)(GEN_G + (o)))
#define GEN_WAVE     (*(volatile unsigned char *)(GEN_G + 0x2EFA))
#define GEN_BUF      ((unsigned short *)(GEN_G + 0x0E))      /* sample table */
#define GEN_BUF2     ((unsigned short *)(GEN_G + 0xFB6))     /* second table */
/* __aeabi_d2ulz is Thumb code and uses the base AAPCS (double in r0:r1): called through a pointer
 * to its Thumb address so the BLX switches state, as the stock BLX does */
typedef unsigned long long (*d2ulz_fn)(double) __attribute__((pcs("aapcs")));
#define D2ULZ(d) (((d2ulz_fn)CODE_ADDR(__aeabi_d2ulz))(d))
unsigned long long __aeabi_uldivmod(unsigned long long n, unsigned long long d);   /* 0x40034590 */

/* ---- acquisition control (batch 14) */
#define SYS_BACKUP   ((void *)0x409223F0u)     /* copy of SYS_INFO (0x3DD588 bytes) */
#define SYS_SIZE     0x3DD588u

/* ---- disk glue (FatFs diskio for the SD card, drive 0) */
#define g_sd_state     (*(volatile u32 *)IMG(40095490))   /* 1 = card ready */
#define g_sector_size  (*(volatile u32 *)IMG(4009548c))
#define g_sd_info      ((volatile u32 *)0x42E68A18u)    /* [1] = sector count */
#define SD_BOUNCE      ((unsigned char *)0x427EB634u)
#define g_fatfs0       ((void *)0x42E67E24u)
#define STR_DRIVE0     ((const char *)IMG(40049de0))     /* "0:" */
#define MKFS_WORK      ((void *)0x428DF874u)
#endif
