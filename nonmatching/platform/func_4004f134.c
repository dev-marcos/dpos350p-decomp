/*
 * sd_update_check -- stock 0x4004F134 (1116 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Firmware update check at start-up (called by main). Scans the SD root for the first name whose text after its first '.' is exactly 'iso' (lower case, at least one character before the dot), closes the directory and opens the file. The file must be at least 1000 bytes (low word of the size) and start with the string 'DPOS350P' (0x40093EF0). The data after the first 4 KB is read into 0x41E7B9C8; its last 36 bytes are nine big-endian checksums: the byte sum, then for bit 7 down to bit 0 the sum of the offsets of the bytes that have that bit set. When all nine match, the 4 KB flash sector at 0xA8000 is cleared to zero and the system is reset by the watchdog. Otherwise the file is deleted. The size and name used after the scan are those of the last f_readdir result.
 */
#include "hw/platform.h"

#include "hw/fpga.h"

int f_opendir(void *dp, const char *path);
int f_closedir(void *dp);
int f_readdir(void *dp, void *fno);
int f_open(void *fp, const char *path, unsigned char mode);
int f_close(void *fp);
int f_read(void *fp, void *buff, u32 btr, u32 *br);
int f_lseek(void *fp, unsigned long long ofs);
int f_unlink(const char *path);
void __aeabi_memclr4(void *p, u32 n);
void persistence_write_read_modify_write(u32 dev, u32 addr, const volatile unsigned char *buf, u32 len);
void wdt_system_reset(void);

#define IMAGE ((volatile unsigned char *)0x41E7B9C8u)
#define MAGIC ((const volatile unsigned char *)0x40093EF0u)
#define SECTOR_ZERO ((void *)0x42E59B10u)

struct finfo {                    /* FatFs FILINFO (R0.13b, long file names) */
    unsigned long long fsize;
    unsigned short fdate, ftime;
    unsigned char fattrib;
    char altname[13];
    char fname[256];
};

static u32 be32(const volatile unsigned char *p)
{
    return ((u32)p[0] << 24) | ((u32)p[1] << 16) | ((u32)p[2] << 8) | p[3];
}

void sd_update_check(void)
{
    unsigned long long fil[0x258 / 8];
    unsigned long long dir[0x50 / 8];
    struct finfo info;
    int dir_open = 1, found = 0, k;
    u32 size, n, m, i, sum[9], want[9];
    const volatile unsigned char *s, *e;
    volatile unsigned char *b;

    f_opendir(dir, "0:");
    while (f_readdir(dir, &info) == 0 && info.fname[0] != 0) {
        const char *p = info.fname;
        u32 len = 0;
        unsigned char l;
        while (*p != '.') {
            if (*p++ == 0)
                goto next;
            len++;
        }
        l = (unsigned char)len;
        if (l == 0)
            continue;
        if (info.fname[l + 1] == 'i' && info.fname[l + 2] == 's' && info.fname[l + 3] == 'o' &&
            info.fname[l + 4] == 0) {
            dir_open = 0;
            f_closedir(dir);
            if (f_open(fil, info.fname, 1) == 0)
                found = 1;
        }
next:;
    }
    if (dir_open)
        f_closedir(dir);
    if (!found)
        return;

    size = (u32)info.fsize;
    if (size < 1000) {
        f_close(fil);
        goto bad;
    }
    f_lseek(fil, 0);
    f_read(fil, (void *)IMAGE, 0x1000, 0);
    for (s = MAGIC, b = IMAGE; *s != 0; s++, b++)
        if (*s != *b) {
            f_close(fil);
            goto bad;
        }
    n = size - 0x1000;
    f_lseek(fil, 0x1000);
    f_read(fil, (void *)IMAGE, n, 0);
    f_close(fil);

    e = IMAGE + n - 36;
    for (k = 0; k < 9; k++) {
        want[k] = be32(e + 4 * k);
        sum[k] = 0;
    }
    m = n - 36;
    for (i = 0; i < m; i++) {
        u32 c = IMAGE[i];
        sum[0] += c;
        for (k = 0; k < 8; k++)
            if (c & (0x80u >> k))
                sum[1 + k] += i;
    }
    for (k = 0; k < 9; k++)
        if (sum[k] != want[k])
            goto bad;
    __aeabi_memclr4(SECTOR_ZERO, 0x1000);
    persistence_write_read_modify_write(0, 0xA8000, (const volatile unsigned char *)SECTOR_ZERO, 0x1000);
    wdt_system_reset();
    return;
bad:
    f_unlink(info.fname);
}
