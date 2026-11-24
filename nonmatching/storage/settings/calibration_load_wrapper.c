/*
 * calibration_load_wrapper -- stock 0x400446B4 (52 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Reads 0x5000 bytes from SYS+0x7E0 into the image and decodes the calibration fields. (name: Hack)
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "storage/settings.h"
#include "sys_info.h"
#include "shared/storage.h"

void calibration_load_wrapper(void)
{
    persistence_read(0, SI(calibration_flash_addr), SER_IMAGE, 0x5000);
    serializer_set_decode_mode();
    calibration_process_serialized_fields();
}
