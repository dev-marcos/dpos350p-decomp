/*
 * console_putc -- stock 0x40066CF0 (20 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * uart_putc on the debug port (g_uart_port, low byte sign-extended).
 */
#include "hw/platform.h"
#include "drivers/uart.h"

void console_putc(int c) { uart_putc((signed char)g_uart_port, c); }
