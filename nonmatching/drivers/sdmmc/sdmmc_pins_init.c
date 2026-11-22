/*
 * sdmmc_pins_init -- stock 0x400675B0 (140 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * SD/MMC pin mux: controller 0 -> PF0..PF5, 1 -> PC0..PC5, 2 -> PC0..PC10; function 2, pull-up. Name DERIVED from the pins.
 */
#include "hw/drivers.h"

void sdmmc_pins_init(int ctrl)
{
    int i;
    if (ctrl == 0) {
        for (i = 0; i < 6; i++) pio_cfg_pin((pio_port_t *)PIO_PF, i, 2, 1);
    } else if (ctrl == 1) {
        for (i = 0; i < 6; i++) pio_cfg_pin((pio_port_t *)PIO_PC, i, 2, 1);
    } else if (ctrl == 2) {
        for (i = 0; i < 11; i++) pio_cfg_pin((pio_port_t *)PIO_PC, i, 2, 1);
    }
}
