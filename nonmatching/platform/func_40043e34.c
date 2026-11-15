/*
 * settings_e_load -- stock 0x40043E34 (148 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Loads record E. Without the magic 0x5A5555A5 (first start or erased flash) it rebuilds the defaults: 0x4003EFAC, then 0x4004435C for slots 1..5 (SYS_INFO+0x7C0), slot 1 saved in record F, SYS+0x7AC = 2 and record B saved.
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

void settings_e_fields(void);
void settings_f_save(void);
void settings_b_save(void);
void func_4003efac(void);
void func_4004435c(void);

void settings_e_load(void)
{
    u32 i;
    persistence_read(0, SYS32(0x7DC), SER_IMAGE, 0x1000);
    serializer_set_decode_mode();
    settings_e_fields();
    if (*(volatile u32 *)0x42E68AE0u == 0x5A5555A5u)
        return;
    func_4003efac();
    for (i = 1; i <= 5; i++) {
        SYS8(0x7C0) = i;
        func_4004435c();
    }
    SYS8(0x7C0) = 1;
    settings_f_save();
    SYS8(0x7AC) = 2;
    settings_b_save();
}
