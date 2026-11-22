/* Functions of nonmatching/drivers/uart/ (one file per function, named after it).
 * Addresses are of build 2025-06-19. Names are DERIVED unless the C file says otherwise. */
#ifndef DRIVERS_UART_H
#define DRIVERS_UART_H

#include "rt/bits.h"

void uart_putc(int port, int c);                                                                 /* 0x40066C90 */
void console_putc(int c);                                                                        /* 0x40066CF0 */
void console_puts(const char *s);                                                                /* 0x40066D04 */
void console_putn(int c, int n);                                                                 /* 0x40066D28 */
void console_put_reversed(const char *buf, int n);                                               /* 0x40066D54 */
void console_print_num(int val, int base, int width, int pad);                                   /* 0x40066D88 */
const char *console_format_conv(const char *p, int arg);                                         /* 0x40066ED0 */
void dbg_printf(const char *fmt, ...);                                                           /* 0x4006709C */
void debug_printf_stub(const char *fmt, ...);                                                    /* 0x4006711C */

#endif
