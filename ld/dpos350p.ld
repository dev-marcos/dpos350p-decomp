/* DPOS350P application image, build 2025-06-19.
 * Loaded and executed at 0x40000000 (DDR2, 64 MiB) by the FNIRSI boot chain.
 * Single load region, load address == execution address (armlink original);
 * ZI (.bss) 0x400954A4..0x42E68D24 is not part of the image (see docs/MEMORY_MAP.md). */
OUTPUT_FORMAT("elf32-littlearm")
OUTPUT_ARCH(arm)
ENTRY(vectors)

MEMORY
{
    DRAM (rwx) : ORIGIN = 0x40000000, LENGTH = 64M
}

SECTIONS
{
    .text 0x40000000 : {
        INCLUDE ld/sections.ld
    } > DRAM
    __image_end = .;
    ASSERT(__image_end - 0x40000000 == 0x957E0, "app image must be exactly 612320 bytes")

    /DISCARD/ : { *(.ARM.attributes) *(.comment) *(.note*) *(.ARM.exidx*) }
}
