/*
 * settings_f_save -- stock 0x40043EEC (52 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Saves settings record F (the slot byte SYS_INFO+0x7C0): encode the field list, then write 0x1000 bytes of the image to the flash address SYS_INFO+0x7C4.
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "storage/settings.h"

#define SER_IMAGE ((volatile void *)0x408A526Cu)   /* SYS_INFO+0x360404 */

void settings_f_save(void)
{
    serializer_set_encode_mode();
    settings_f_fields();
    persistence_write_read_modify_write(0, SYS32(0x7C4), SER_IMAGE, 0x1000);
}
