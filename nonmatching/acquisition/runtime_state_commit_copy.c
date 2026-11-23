/*
 * runtime_state_commit_copy -- stock 0x4003BEB4 (160 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * When SYS+0x38 is set: copies the live acquisition parameters to their committed copies (0x28->0x29, 0x550->0x5D0 (16 bit), 0x558/0x55C->0x5D4/0x5D8, 0x588->0x5B0 (byte), 0x590->0x5B8, 0x530->0x5A8, 0x570->0x5E0 (64 bit), 0x568/0x56C->0x5E8/0x5EC, 0x50->0x58, 0x60->0x64, 0x110->0x118, 0x120->0x124).
 */
#include "hw/platform.h"

#include "hw/fpga.h"
#include "acquisition.h"
#include "sys_info.h"

void runtime_state_commit_copy(void)
{
    if (SI(acq_done) == 0)
        return;
    SI(acq_path_mode_shadow) = SI(acq_path_mode);
    SI(f_5d0) = SI(capture_count);
    SI(f_5b0) = SI(timebase_index);
    SI(f_5d4) = SI(f_558);
    SI(f_5d8) = SI(f_55c);
    SI(f_5b8) = SI(f_590);
    SI(f_5a8) = SI(f_530);
    SYS64(0x5E0) = SYS64(0x570);
    SI(f_5e8) = SI(f_568);
    SI(f_5ec) = SI(f_56c);
    SYS64(0x58) = SI(ch1_scale);
    SI(f_064) = SI(ch1_offset);
    SYS64(0x118) = SI(ch2_scale);
    SI(f_124) = SI(ch2_offset);
}
