/*
 * dbg_printf -- stock 0x4006709C (120 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * printf to the debug UART when its bus clock gate is on (CCU 0x01C2006C bit 16 + port).
 */
#include "hw/platform.h"

#include <stdarg.h>
#include "drivers/mmio.h"
#include "drivers/uart.h"

void dbg_printf(const char *fmt, ...)
{
    va_list ap;
    const char *p = fmt;
    if (!((readl_g(0x01C2006Cu) >> (g_uart_port + 16)) & 1))
        return;
    va_start(ap, fmt);
    while (*p) {
        if (*p == '%')
            p = console_format_conv(p + 1, va_arg(ap, int));
        else
            console_putc((signed char)*p++);
    }
    va_end(ap);
}
