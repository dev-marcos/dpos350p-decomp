/*
 * outputs_apply -- stock 0x4003E708 (48 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Unless the byte 0x42E68BC7 is set: 0x4003A654, frontend_apply_all, 0x40063328.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "acquisition.h"
#include "fpga.h"
#include "frontend.h"
#include "system.h"

void outputs_apply(void)
{
    if ((g_touch->serviced))
        return;
    acquisition_apply();
    frontend_apply_all();
    fpga_config_push();
}
