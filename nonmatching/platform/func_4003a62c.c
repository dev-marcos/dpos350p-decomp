/*
 * acquisition_reconfigure -- stock 0x4003A62C (40 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * sample_divisor_setup, 0x4003A680, 0x4005DB9C, 0x4005D27C, acquisition_geometry_setup, then runtime_state_commit_copy.
 */
#include "hw/platform.h"

#include "hw/fpga.h"

void sample_divisor_setup(void);
void func_4003a680(void);
void func_4005db9c(void);
void func_4005d27c(void);
void acquisition_geometry_setup(void);
void runtime_state_commit_copy(void);

void acquisition_reconfigure(void)
{
    sample_divisor_setup();
    func_4003a680();
    func_4005db9c();
    func_4005d27c();
    acquisition_geometry_setup();
    runtime_state_commit_copy();
}
