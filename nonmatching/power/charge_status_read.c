/*
 * charge_status_read -- stock 0x4004A278 (40 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Charger status (selector 0x02 bit 0) into SYS_INFO+0x779.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "drivers/fpga_bus.h"
#include "power.h"

void charge_status_read(void)
{
    fpga_selector_command_write(2);
    SYS8(0x779) = fpga_data_byte_read() & 1;
}
