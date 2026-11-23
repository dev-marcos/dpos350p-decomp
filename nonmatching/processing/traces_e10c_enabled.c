/*
 * traces_e10c_enabled -- stock 0x4003C550 (112 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Runs 0x4005E10C for every enabled trace: 0 (CH1 on, SYS+0x3E), then 2 when SYS+0x2AA is set; 1 (CH2 on, SYS+0x102), then 3. When SYS+0x652 is set it only runs 0x4005E10C(0).
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "processing.h"
#include "ui/draw.h"
#include "sys_info.h"

void traces_e10c_enabled(void)
{
    if (SI(f_652)) {
        layer_clear(0);
        return;
    }
    if (SI(ch1_enabled)) {
        layer_clear(0);
        if (SI(zoom_on))
            layer_clear(2);
    }
    if (SI(ch2_enabled)) {
        layer_clear(1);
        if (SI(zoom_on))
            layer_clear(3);
    }
}
