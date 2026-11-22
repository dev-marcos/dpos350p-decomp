/*
 * __USBC_Dev_Tx_ConfigEp -- stock 0x4006CDDC (72 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * CSR = 0x2048 (flush + clear toggle; written twice for a double FIFO), MAXP |= max packet (11 bits), then the type (2 = ISO, 3 = INT, else bulk).
 */
#include "hw/usb.h"
#include "usb/usbc.h"

void __USBC_Dev_Tx_ConfigEp(u32 ts_type, u32 is_double_fifo, u32 ep_MaxPkt)
{
    REG16(USBC_CSR0) = 0x2048;
    if (is_double_fifo)
        REG16(USBC_CSR0) = 0x2048;
    REG16(USBC_TXMAXP) = (unsigned short)(REG16(USBC_TXMAXP) | (ep_MaxPkt & 0x7FF));
    switch (ts_type) {
    case 2: __USBC_Dev_Tx_EnableIsoEp(); break;
    case 3: __USBC_Dev_Tx_EnableIntEp(); break;
    default: __USBC_Dev_Tx_EnableBulkEp(); break;
    }
}
