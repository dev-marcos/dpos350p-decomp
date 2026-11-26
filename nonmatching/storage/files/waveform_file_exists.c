/*
 * waveform_file_exists -- stock 0x400364A0 (64 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * 0x40035BC4(a) (builds the path 0x4054D728), then 1 when f_open(read) of it succeeds (the file is closed again), else 0.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "fatfs_app.h"
#include "storage/files.h"

u32 waveform_file_exists(u32 a)
{
    unsigned char fil[0x250];
    u32 r = 0;
    waveform_path_make(a);
    if (f_open(fil, (const char *)0x4054D728u, 1) == 0) {
        r = 1;
        f_close(fil);
    }
    return r;
}
