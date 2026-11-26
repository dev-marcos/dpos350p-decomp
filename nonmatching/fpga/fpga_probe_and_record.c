/*
 * fpga_probe_and_record -- stock 0x4004F59C (36 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * perform_fpga_identification_probe, then byte 0x42E68AE0+0x11 = SYS+0x7FC.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "fpga.h"
#include "sys_info.h"

void fpga_probe_and_record(void)
{
    perform_fpga_identification_probe();
    *(volatile unsigned char *)0x42E68AF1u = SI(fpga_probe_result);
}
