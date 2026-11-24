/*
 * ms5351_write_reg -- stock 0x4005F164 (88 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * MS5351M: write one register (START, 0xC0, reg, value, STOP).
 */
#include "hw/drivers.h"
#include "drivers/ms5351.h"
#include "drivers/timer.h"
#include "shared/drivers.h"

void ms5351_write_reg(u32 reg, u32 val)
{
    ms5351_i2c_start();
    delay_us(1);
    ms5351_write_byte(0xC0);
    delay_us(1);
    ms5351_write_byte(reg);
    delay_us(1);
    ms5351_write_byte(val);
    delay_us(1);
    ms5351_i2c_stop();
}
