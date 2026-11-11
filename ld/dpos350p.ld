/* DPOS350P application image, build 2025-06-19.
 * Loaded and executed at 0x40000000 (DDR2, 64 MiB) by the FNIRSI boot chain. */
OUTPUT_FORMAT("elf32-littlearm")
OUTPUT_ARCH(arm)
ENTRY(app_start)

MEMORY
{
    DRAM (rwx) : ORIGIN = 0x40000000, LENGTH = 64M
}

SECTIONS
{
    .text 0x40000000 : {
        KEEP(*(.text.app))
    } > DRAM
    __image_end = .;
    ASSERT(__image_end - 0x40000000 == 0x957E0, "app image must be exactly 612320 bytes")

    /DISCARD/ : { *(.ARM.attributes) *(.comment) *(.note*) }
}
