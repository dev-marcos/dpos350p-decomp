/*
 * settings_image_decode -- stock 0x400456F4 (32 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * settings_h_decode, settings_image_fields_decode, settings_i_decode, settings_j_decode.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "storage/settings.h"

void settings_image_decode(void)
{
    settings_h_decode();
    settings_image_fields_decode();
    settings_i_decode();
    settings_j_decode();
}
