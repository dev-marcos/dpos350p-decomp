/*
 * fpga_channel_mode_apply -- stock 0x4004AA24 (76 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * When SYS_INFO+0x100 changed (copy in +0xBCA): FPGA selector 0x2E <- value, then 50 ms.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "drivers/fpga_bus.h"
#include "drivers/timer.h"
#include "fpga.h"

void fpga_channel_mode_apply(void)
{
    if (SYS8(0xBCA) == SYS8(0x100))
        return;
    SYS8(0xBCA) = SYS8(0x100);
    fpga_selector_command_write(0x2E);
    fpga_data_byte_write(SYS8(0xBCA));
    delay_ms(50);
}
