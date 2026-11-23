/*
 * settings_c_load -- stock 0x40043C9C (12 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Loads settings record C: read 0x4000 bytes from the flash address SYS_INFO+0x7F0 into the image, then decode the field list. The stock entry is 12 bytes and falls through into 0x40043CA8.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "storage/files.h"
#include "storage/settings.h"
#include "sys_info.h"

#define SER_IMAGE ((volatile void *)0x408A526Cu)   /* SYS_INFO+0x360404 */

void settings_c_load(void)
{
    persistence_read(0, SI(rec_c_flash_addr), SER_IMAGE, 0x4000);
    serializer_set_decode_mode();
    record_c_fields();
}
