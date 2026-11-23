/*
 * settings_d_save -- stock 0x40043D54 (52 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Saves settings record D (0x42E554B0, 300 entries): encode the field list, then write 0x2000 bytes of the image to the flash address SYS_INFO+0x7EC.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "storage/settings.h"
#include "sys_info.h"

#define SER_IMAGE ((volatile void *)0x408A526Cu)   /* SYS_INFO+0x360404 */

void settings_d_save(void)
{
    serializer_set_encode_mode();
    settings_d_fields();
    persistence_write_read_modify_write(0, SI(rec_d_flash_addr), SER_IMAGE, 0x2000);
}
