/*
 * settings_h_decode -- stock 0x40045670 (24 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Decode mode, then settings_h_fields (the header at cursor 0).
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "storage/settings.h"

void settings_h_decode(void)
{
    serializer_set_decode_mode();
    settings_h_fields();
}
