/*
 * settings_h_fields -- stock 0x4004480C (124 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Field list of record H: the words SYS_INFO+0x75C, 0x768, 0x760, 0x76C, 0x764, 0x770, 0x774, in that order.
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

void settings_h_fields(void)
{
    serializer_set_cursor(0);
    serializer_process_field(&SYS32(0x75C), 4);
    serializer_process_field(&SYS32(0x768), 4);
    serializer_process_field(&SYS32(0x760), 4);
    serializer_process_field(&SYS32(0x76C), 4);
    serializer_process_field(&SYS32(0x764), 4);
    serializer_process_field(&SYS32(0x770), 4);
    serializer_process_field(&SYS32(0x774), 4);
}
