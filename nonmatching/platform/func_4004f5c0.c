/*
 * security_check_and_copy -- stock 0x4004F5C0 (36 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * security_chip_check, then byte 0x42E68AE0+0x12 = byte 0x42E62C2C+0x8F8.
 */
#include "hw/platform.h"

#include "hw/fpga.h"

void security_chip_check(void);

void security_check_and_copy(void)
{
    security_chip_check();
    *(volatile unsigned char *)0x42E68AF2u = *(volatile unsigned char *)(0x42E62C2Cu + 0x8F8u);
}
