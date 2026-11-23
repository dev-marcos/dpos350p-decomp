/*
 * auto_off_arm -- stock 0x4004A1D0 (36 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Auto power-off deadline = tick + minutes (SYS_INFO+0x658) x 60000 ms, kept in SYS_INFO+0x65C.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "power.h"
#include "sys_info.h"

void auto_off_arm(void)
{
    SI(auto_off_deadline) = g_tick_count + 60000u * SI(auto_off_minutes);
}
