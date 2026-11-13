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
void fpga_selector_command_write(u32 sel);   /* 0x4004927C */
u32  fpga_data_byte_read(void);              /* 0x400491B8 */
void fpga_data_byte_write(u32 v);            /* 0x40049214 */
void fpga_bus_init(void);                    /* 0x40049110 */
#define g_pe_mode  (*(volatile unsigned char *)0x42E68D19u)   /* shared PE pins: 0 = FPGA control bus, else CSI capture */
void csi_full_init(u32 arg);                 /* 0x400660C8 */
void csi_pins_init_0(void);                  /* 0x400660C0 */
void ms5351_pins_init(void);                 /* 0x4005F1BC */
void ms5351_set_clk0(u32 hz);                /* 0x4005F1F0 */
void ms5351_set_clk1(u32 hz);                /* 0x4005F38C */

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

void selector_0x69_16_bit_timing_retry(u32 v);   /* 0x40049FB8 */
void selector_0x6a_one_byte_job_index(u32 v);    /* 0x40049F60 */
void selector_0x6b_32_bit_job_value(u32 v);      /* 0x40049F7C */
void smec_start_job(void);                       /* 0x40049FDC */
u32  smec_job_valid(void);                       /* 0x4004A020 */
void smec_wait_ready(void);                      /* 0x4004A040 */
u32  smec_job_echo(void);                        /* 0x4004A064 */
u32  smec_job_result(void);                      /* 0x4004A080 */
u32  smec_run_job(u32 idx, u32 val);             /* 0x4004A0B0 */
void smec_reset_cmd(void);                       /* 0x4004A13C */
void smec_timing_reset(void);                    /* 0x40049F44 */
void smec_commit(u32 idx);                       /* 0x40049F1C */
void fpga_param_updater(u32 idx, u32 size, u32 v32, u32 vsmall);   /* 0x40049EC4 */
void fpga_params_push(void);                     /* 0x40049E30 */
void __aeabi_memclr4(void *d, unsigned int n);   /* 0x40068E5C */

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
void gt911_read16(u32 reg, unsigned char *buf, u32 len);         /* 0x400498D4 */
void gt911_write16(u32 reg, const unsigned char *buf, u32 len);  /* 0x400498C4 */
void gt911_i2c_pins_init(void);                                  /* 0x40049788 */
void gt911_ctrl_pins_out(void);                                  /* 0x400497D4 */
void gt911_bus_init(void);                                       /* 0x400498A4 */
void gt911_poll(void);                                           /* 0x400498E4 */
void touch_state_clear(void);                                    /* 0x40049A38 */
void gt911_pins_init(void);                                      /* 0x40049A5C */

/* ---- power */
void func_40068e94(void);                    /* LRADC (battery) init */
void func_40034134(int x, int y, u32 img, int flags);   /* draws an image (OPEN) */
void delay_ms(u32 ms);                       /* 0x40042CF0 */
void power_off(void);                        /* 0x4004A1B4 (never returns) */
void auto_off_arm(void);                     /* 0x4004A1D0 */
void battery_coeffs_default(void);           /* 0x4004A2A0 */

/* ---- disk glue (FatFs diskio for the SD card, drive 0) */
#define g_sd_state     (*(volatile u32 *)0x40095490u)   /* 1 = card ready */
#define g_sector_size  (*(volatile u32 *)0x4009548Cu)
#define g_sd_info      ((volatile u32 *)0x42E68A18u)    /* [1] = sector count */
#define SD_BOUNCE      ((unsigned char *)0x427EB634u)
#define g_fatfs0       ((void *)0x42E67E24u)
#define STR_DRIVE0     ((const char *)0x40049DE0u)     /* "0:" */
#define MKFS_WORK      ((void *)0x428DF874u)
int  mmc_card_init(int n);                                        /* 0x40068918 */
int  mmc_read_blocks(int n, u32 sector, u32 count, void *buf);    /* 0x4006866C */
int  mmc_write_blocks(int n, u32 sector, u32 count, const void *buf);   /* 0x40068794 */
void *memcpy(void *d, const void *s, unsigned int n);      /* memcpy veneer */
int  f_mount(void *fs, const char *path, int opt);                /* 0x40059AEC */
int  f_mkfs(const char *path, int opt, u32 au, void *work, u32 len);   /* 0x4005B1F8 */
int  f_getfree(const char *path, u32 *nclst, void **fatfs);       /* 0x4005ABC0 */
int  disk_mount(void);                                            /* 0x40049B80 */
#endif
