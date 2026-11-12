/*
 * __USBC_Dev_Rx_ConfigEp -- stock 0x4006CE68 (72 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * CSR = 0x0090 (flush + clear toggle; written twice for a double FIFO), MAXP |= max packet (11 bits), then the type (2 = ISO, 3 = INT, else bulk).
 */
#include "hw/usb.h"

void __USBC_Dev_Rx_ConfigEp(u32 ts_type, u32 is_double_fifo, u32 ep_MaxPkt)
{
    REG16(USBC_RXCSR) = 0x0090;
    if (is_double_fifo)
        REG16(USBC_RXCSR) = 0x0090;
    REG16(USBC_RXMAXP) = (unsigned short)(REG16(USBC_RXMAXP) | (ep_MaxPkt & 0x7FF));
    switch (ts_type) {
    case 2: __USBC_Dev_Rx_EnableIsoEp(); break;
    case 3: __USBC_Dev_Rx_EnableIntEp(); break;
    default: __USBC_Dev_Rx_EnableBulkEp(); break;
    }
}
