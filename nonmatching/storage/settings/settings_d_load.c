/*
 * settings_d_load -- stock 0x40043D88 (52 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Loads settings record D: read 0x2000 bytes from the flash address SYS_INFO+0x7EC into the image, then decode the field list.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "storage/settings.h"
#include "sys_info.h"
#include "shared/storage.h"

void settings_d_load(void)
{
    persistence_read(0, SI(rec_d_flash_addr), SER_IMAGE, 0x2000);
    serializer_set_decode_mode();
    settings_d_fields();
}
