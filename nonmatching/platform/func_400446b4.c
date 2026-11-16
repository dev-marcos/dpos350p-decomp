/*
 * calibration_load_wrapper -- stock 0x400446B4 (52 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Reads 0x5000 bytes from SYS+0x7E0 into the image and decodes the calibration fields. (name: Hack)
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

void calibration_load_wrapper(void)
{
    persistence_read(0, SYS32(0x7E0), SER_IMAGE, 0x5000);
    serializer_set_decode_mode();
    calibration_process_serialized_fields();
}
