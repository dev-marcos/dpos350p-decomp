/*
 * calibration_save_wrapper -- stock 0x40044680 (52 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Encodes the calibration fields and writes 0x5000 bytes of the image to the flash address SYS+0x7E0. (name: Hack)
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "storage/settings.h"
#include "sys_info.h"
#include "shared/storage.h"

void calibration_save_wrapper(void)
{
    serializer_set_encode_mode();
    calibration_process_serialized_fields();
    persistence_write_read_modify_write(0, SI(calibration_flash_addr), SER_IMAGE, 0x5000);
}
