/*
 * calibration_save_wrapper -- stock 0x40044680 (52 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Encodes the calibration fields and writes 0x5000 bytes of the image to the flash address SYS+0x7E0. (name: Hack)
 */
#include "hw/platform.h"

#include "hw/fpga.h"

void serializer_set_cursor(u32 c);
void serializer_set_encode_mode(void);
void serializer_set_decode_mode(void);
void serializer_process_field(volatile void *p, u32 size);
void persistence_read(u32 dev, u32 addr, volatile void *buf, u32 len);
void persistence_write_read_modify_write(u32 dev, u32 addr, volatile void *buf, u32 len);
#define SER_IMAGE ((volatile void *)0x408A526Cu)   /* SYS_INFO+0x360404 */
void calibration_process_serialized_fields(void);

void calibration_save_wrapper(void)
{
    serializer_set_encode_mode();
    calibration_process_serialized_fields();
    persistence_write_read_modify_write(0, SYS32(0x7E0), SER_IMAGE, 0x5000);
}
