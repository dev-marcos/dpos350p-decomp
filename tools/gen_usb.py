#!/usr/bin/env python3
"""Write nonmatching/usb/func_*.c from a compact spec, and the matching symbols/manual.csv rows.

The C bodies are transcriptions of the stock disassembly; names and structure follow EasyUSB
(see docs/USB.md). Usage: python3 tools/gen_usb.py [--csv]   (--csv prints manual.csv rows)
"""
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
OUT = ROOT / "nonmatching/usb"

HDR = """/*
 * {name} -- stock 0x{addr:08X} ({size} bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * {desc}
 */
#include "hw/usb.h"

"""

# (address, size, name, EasyUSB origin, description, body)
SPEC = []


def f(addr, size, name, src, desc, body):
    SPEC.append((addr, size, name, src, desc, body.strip("\n") + "\n"))


P = "usb_phy.c"
# ------------------------------------------------------------------ PHY, bus, FIFO base
f(0x4006C264, 84, "USBC_SelectBus", P, "VEND0: select DMA (DRQ for a TX/RX endpoint) or PIO.", """
void USBC_SelectBus(u32 io_type, u32 ep_type, u32 ep_index)
{
    u32 v = REG8(USBC_VEND0);
    if (io_type == 1) {
        if (ep_type == USBC_EP_TYPE_TX)
            v |= ((ep_index - 1) << 1) << 1 | 1;
        else
            v |= ((ep_index << 1) - 1) << 1 | 1;
    } else {
        v = 0;
    }
    REG8(USBC_VEND0) = (unsigned char)v;
}
""")
f(0x4006C2B8, 112, "usb_phy_write", P,
  "Bit-bang len bits of data into the PHY through the CSR at +0x410. Every macro does a 32-bit read and an 8-bit write (FNIRSI variant), so the bit address (bits 15:8) never reaches the PHY.", """
void usb_phy_write(int addr, int data, int len)
{
    int j;
    for (j = 0; j < len; j++) {
        REG8(USBC_PHY_CSR) = (unsigned char)(REG32(USBC_PHY_CSR) & ~(0xFFu << 8));
        REG8(USBC_PHY_CSR) = (unsigned char)(REG32(USBC_PHY_CSR) | ((u32)(addr + j) << 8));
        REG8(USBC_PHY_CSR) = (unsigned char)(REG32(USBC_PHY_CSR) & ~1u);
        if (data & 1)
            REG8(USBC_PHY_CSR) = (unsigned char)(REG32(USBC_PHY_CSR) | 0x80);
        else
            REG8(USBC_PHY_CSR) = (unsigned char)(REG32(USBC_PHY_CSR) & ~0x80u);
        REG8(USBC_PHY_CSR) = (unsigned char)(REG32(USBC_PHY_CSR) | 1);
        REG8(USBC_PHY_CSR) = (unsigned char)(REG32(USBC_PHY_CSR) & ~1u);
        data >>= 1;
    }
}
""")
f(0x4006C328, 64, "USBC_PhyConfig", P, "PHY tuning: 45 ohm (0x0C), magnitude/rate (0x20), disconnect threshold (0x2A).", """
void USBC_PhyConfig(void)
{
    usb_phy_write(0x0C, 0x01, 1);
    usb_phy_write(0x20, 0x14, 5);
    usb_phy_write(0x2A, 3, 2);
}
""")
f(0x4006C368, 28, "USBC_ConfigFIFO_Base", P, "SRAM controller +0x04: USB FIFO 8 KiB mode.", """
void USBC_ConfigFIFO_Base(void)
{
    u32 v = REG32(0x01C00004u);
    v = (v & ~3u) | 1;
    REG32(0x01C00004u) = v;
}
""")
f(0x4006C384, 80, "usb_phy_open_clock_Device", P,
  "USB PHY clock gate+reset (CCU 0x01C200CC bits 8, 0), bus gate (0x01C20060 bit 24) and bus reset (0x01C202C0 bit 24). FNIRSI version of usb_phy_open_clock.", """
u32 usb_phy_open_clock_Device(void)
{
    debug_printf_stub(STR(0x4008E27C));
    REG32(0x01C200CCu) |= 0x100;
    REG32(0x01C200CCu) |= 1;
    REG32(0x01C20060u) |= 0x1000000;
    REG32(0x01C202C0u) |= 0x1000000;
    return 0;
}
""")
f(0x4006C3D4, 80, "usb_phy_close_clock_Device", P, "Reverse of usb_phy_open_clock_Device.", """
u32 usb_phy_close_clock_Device(void)
{
    debug_printf_stub(STR(0x4008E298));
    REG32(0x01C200CCu) &= ~0x100u;
    REG32(0x01C200CCu) &= ~1u;
    REG32(0x01C20060u) &= ~0x1000000u;
    REG32(0x01C202C0u) &= ~0x1000000u;
    return 0;
}
""")
# ------------------------------------------------------------------ interrupts
f(0x4006C424, 16, "USBC_INT_DisableUsbMiscAll", P, "INTUSBE = 0.", """
void USBC_INT_DisableUsbMiscAll(void) { REG8(USBC_INTUSBE) = 0; }
""")
f(0x4006C434, 12, "USBC_INT_TxPending", P, "INTTx.", """
u32 USBC_INT_TxPending(void) { return REG16(USBC_INTTx); }
""")
f(0x4006C440, 20, "USBC_INT_ClearTxPending", P, "INTTx = 1 << ep.", """
void USBC_INT_ClearTxPending(u32 ep_index) { REG16(USBC_INTTx) = (unsigned short)(1u << ep_index); }
""")
f(0x4006C454, 20, "USBC_INT_ClearTxPendingAll", P, "INTTx = 0xFFFF.", """
void USBC_INT_ClearTxPendingAll(void) { REG16(USBC_INTTx) = 0xFFFF; }
""")
f(0x4006C468, 12, "USBC_INT_RxPending", P, "INTRx.", """
u32 USBC_INT_RxPending(void) { return REG16(USBC_INTRx); }
""")
f(0x4006C474, 20, "USBC_INT_ClearRxPending", P, "INTRx = 1 << ep.", """
void USBC_INT_ClearRxPending(u32 ep_index) { REG16(USBC_INTRx) = (unsigned short)(1u << ep_index); }
""")
f(0x4006C488, 20, "USBC_INT_ClearRxPendingAll", P, "INTRx = 0xFFFF.", """
void USBC_INT_ClearRxPendingAll(void) { REG16(USBC_INTRx) = 0xFFFF; }
""")
f(0x4006C49C, 24, "USBC_INT_EnableTxEp", P, "INTTxE |= 1 << ep.", """
void USBC_INT_EnableTxEp(u32 ep_index) { REG16(USBC_INTTxE) = (unsigned short)(REG16(USBC_INTTxE) | (1u << ep_index)); }
""")
f(0x4006C4B4, 24, "USBC_INT_EnableRxEp", P, "INTRxE |= 1 << ep.", """
void USBC_INT_EnableRxEp(u32 ep_index) { REG16(USBC_INTRxE) = (unsigned short)(REG16(USBC_INTRxE) | (1u << ep_index)); }
""")
f(0x4006C4CC, 4, "USBC_INT_DisableTxAll", P, "INTTxE = 0 (mov r0,#0, then falls into the store at 0x4006C4D0).", """
void USBC_INT_DisableTxAll(void) { REG16(USBC_INTTxE) = 0; }
""")
f(0x4006C4D0, 12, "USBC_INT_WriteTxE", P, "Store r0 to INTTxE: tail shared with USBC_INT_DisableTxAll, no direct callers (name OPEN).", """
void USBC_INT_WriteTxE(u32 v) { REG16(USBC_INTTxE) = (unsigned short)v; }
""")
f(0x4006C4DC, 4, "USBC_INT_DisableRxAll", P, "INTRxE = 0 (falls into 0x4006C4E0).", """
void USBC_INT_DisableRxAll(void) { REG16(USBC_INTRxE) = 0; }
""")
f(0x4006C4E0, 12, "USBC_INT_WriteRxE", P, "Store r0 to INTRxE: tail shared with USBC_INT_DisableRxAll (name OPEN).", """
void USBC_INT_WriteRxE(u32 v) { REG16(USBC_INTRxE) = (unsigned short)v; }
""")
f(0x4006C4EC, 36, "USBC_INT_EpPending", P, "EP0/TX -> INTTx, RX -> INTRx, else 0.", """
u32 USBC_INT_EpPending(u32 ep_type)
{
    switch (ep_type) {
    case USBC_EP_TYPE_EP0:
    case USBC_EP_TYPE_TX: return USBC_INT_TxPending();
    case USBC_EP_TYPE_RX: return USBC_INT_RxPending();
    default: return 0;
    }
}
""")
f(0x4006C510, 40, "USBC_INT_ClearEpPending", P, "Dispatch on the endpoint type.", """
void USBC_INT_ClearEpPending(u32 ep_type, u32 ep_index)
{
    switch (ep_type) {
    case USBC_EP_TYPE_EP0:
    case USBC_EP_TYPE_TX: USBC_INT_ClearTxPending(ep_index); break;
    case USBC_EP_TYPE_RX: USBC_INT_ClearRxPending(ep_index); break;
    default: break;
    }
}
""")
f(0x4006C538, 32, "USBC_INT_ClearEpPendingAll", P, "Dispatch on the endpoint type.", """
void USBC_INT_ClearEpPendingAll(u32 ep_type)
{
    switch (ep_type) {
    case USBC_EP_TYPE_EP0:
    case USBC_EP_TYPE_TX: USBC_INT_ClearTxPendingAll(); break;
    case USBC_EP_TYPE_RX: USBC_INT_ClearRxPendingAll(); break;
    default: break;
    }
}
""")
f(0x4006C558, 12, "USBC_INT_MiscPending", P, "INTUSB.", """
u32 USBC_INT_MiscPending(void) { return REG8(USBC_INTUSB); }
""")
f(0x4006C564, 12, "USBC_INT_ClearMiscPending", P, "INTUSB = mask.", """
void USBC_INT_ClearMiscPending(u32 mask) { REG8(USBC_INTUSB) = (unsigned char)mask; }
""")
f(0x4006C570, 16, "USBC_INT_ClearMiscPendingAll", P, "INTUSB = 0xFF.", """
void USBC_INT_ClearMiscPendingAll(void) { REG8(USBC_INTUSB) = 0xFF; }
""")
f(0x4006C580, 44, "USBC_INT_EnableEp", P, "TX/RX endpoint interrupt enable.", """
void USBC_INT_EnableEp(u32 ep_type, u32 ep_index)
{
    switch (ep_type) {
    case USBC_EP_TYPE_TX: USBC_INT_EnableTxEp(ep_index); break;
    case USBC_EP_TYPE_RX: USBC_INT_EnableRxEp(ep_index); break;
    default: break;
    }
}
""")
f(0x4006C5AC, 20, "USBC_INT_EnableUsbMiscUint", P, "INTUSBE |= mask.", """
void USBC_INT_EnableUsbMiscUint(u32 mask) { REG8(USBC_INTUSBE) = (unsigned char)(mask | REG8(USBC_INTUSBE)); }
""")
f(0x4006C5C0, 12, "USBC_GetActiveEp", P, "EPIND.", """
u32 USBC_GetActiveEp(void) { return REG8(USBC_EPIND); }
""")
f(0x4006C5CC, 12, "USBC_SelectActiveEp", P, "EPIND = ep.", """
void USBC_SelectActiveEp(u32 ep_index) { REG8(USBC_EPIND) = (unsigned char)ep_index; }
""")
# ------------------------------------------------------------------ power / transfer mode
f(0x4006C5D8, 44, "USBC_Dev_ConectSwitch", P, "PCTL bit 6 (soft connect): set for 1, cleared otherwise.", """
void USBC_Dev_ConectSwitch(u32 is_on)
{
    if (is_on == 1)
        REG8(USBC_PCTL) = (unsigned char)(REG8(USBC_PCTL) | 0x40);
    else
        REG8(USBC_PCTL) = (unsigned char)(REG8(USBC_PCTL) & 0xBF);
}
""")
for a, n, op in ((0x4006C604, "__USBC_Dev_TsType_default", "& 0x7F"), (0x4006C618, "__USBC_Dev_TsType_Ctrl", "& 0x7F"),
                 (0x4006C62C, "__USBC_Dev_TsType_Iso", "| 0x80"), (0x4006C640, "__USBC_Dev_TsType_Int", "& 0x7F"),
                 (0x4006C654, "__USBC_Dev_TsType_Bulk", "& 0x7F"), (0x4006C668, "__USBC_Dev_TsMode_default", "& 0xDF"),
                 (0x4006C67C, "__USBC_Dev_TsMode_Hs", "| 0x20"), (0x4006C690, "__USBC_Dev_TsMode_Fs", "& 0xDF")):
    f(a, 20, n, P, f"PCTL = PCTL {op} (ISO-update bit 7 / HS-enable bit 5).",
      f"void {n}(void) {{ REG8(USBC_PCTL) = (unsigned char)(REG8(USBC_PCTL) {op}); }}")
f(0x4006C6A4, 4, "__USBC_Dev_TsMode_Ls", P, "Same as full speed (b __USBC_Dev_TsMode_Fs).", """
void __USBC_Dev_TsMode_Ls(void) { __USBC_Dev_TsMode_Fs(); }
""")
f(0x4006C6A8, 136, "USBC_Dev_ConfigTransferMode", P, "ts_type 1..4 = CTRL/ISO/INT/BULK; speed 1 = LS, 2 = FS, 3 = HS.", """
void USBC_Dev_ConfigTransferMode(u32 ts_type, u32 speed_mode)
{
    switch (ts_type) {
    case 1: __USBC_Dev_TsType_Ctrl(); break;
    case 2: __USBC_Dev_TsType_Iso(); break;
    case 3: __USBC_Dev_TsType_Int(); break;
    case 4: __USBC_Dev_TsType_Bulk(); break;
    default: __USBC_Dev_TsType_default(); break;
    }
    switch (speed_mode) {
    case 3: __USBC_Dev_TsMode_Hs(); break;
    case 2: __USBC_Dev_TsMode_Fs(); break;
    case 1: __USBC_Dev_TsMode_Ls(); break;
    default: __USBC_Dev_TsMode_default(); break;
    }
}
""")
f(0x4006C730, 24, "USBC_Dev_QueryTransferMode", P, "PCTL bit 4 (HS flag): 3 = HS, else 2 = FS.", """
u32 USBC_Dev_QueryTransferMode(void) { return (REG8(USBC_PCTL) & 0x10) ? 3 : 2; }
""")
f(0x4006C748, 36, "USBC_INT_DisableEpAll", P, "TX/RX: all endpoint interrupts off.", """
void USBC_INT_DisableEpAll(u32 ep_type)
{
    switch (ep_type) {
    case USBC_EP_TYPE_TX: USBC_INT_DisableTxAll(); break;
    case USBC_EP_TYPE_RX: USBC_INT_DisableRxAll(); break;
    default: break;
    }
}
""")
f(0x4006C76C, 4, "USBC_Dev_SetAddress_default", P, "FADDR = 0 (falls into 0x4006C770).", """
void USBC_Dev_SetAddress_default(void) { REG8(USBC_FADDR) = 0; }
""")
f(0x4006C770, 12, "USBC_Dev_WriteFAddr", P, "FADDR = r0: tail shared with USBC_Dev_SetAddress_default (name OPEN).", """
void USBC_Dev_WriteFAddr(u32 v) { REG8(USBC_FADDR) = (unsigned char)v; }
""")
f(0x4006C77C, 12, "USBC_Dev_SetAddress", P, "FADDR = address.", """
void USBC_Dev_SetAddress(u32 address) { REG8(USBC_FADDR) = (unsigned char)address; }
""")
# ------------------------------------------------------------------ stall / setup end
R16 = lambda reg, op: f"REG16({reg}) = (unsigned short)(REG16({reg}) {op});"
f(0x4006C788, 16, "__USBC_Dev_ep0_IsEpStall", P, "CSR0 bit 2 (sent stall).", "u32 __USBC_Dev_ep0_IsEpStall(void) { return REG16(USBC_CSR0) & 4; }")
f(0x4006C798, 20, "__USBC_Dev_ep0_SendStall", P, "CSR0 |= 0x20.", f"void __USBC_Dev_ep0_SendStall(void) {{ {R16('USBC_CSR0', '| 0x20')} }}")
f(0x4006C7AC, 40, "__USBC_Dev_ep0_ClearStall", P, "CSR0 &= ~0x20, then &= ~0x04.",
  f"void __USBC_Dev_ep0_ClearStall(void) {{ {R16('USBC_CSR0', '& 0xFFDF')} {R16('USBC_CSR0', '& 0xFFFB')} }}")
f(0x4006C7D4, 20, "__USBC_Dev_Tx_SendStall", P, "TXCSR |= 0x10.", f"void __USBC_Dev_Tx_SendStall(void) {{ {R16('USBC_CSR0', '| 0x10')} }}")
f(0x4006C7E8, 16, "__USBC_Dev_Tx_IsEpStall", P, "TXCSR bit 5.", "u32 __USBC_Dev_Tx_IsEpStall(void) { return REG16(USBC_CSR0) & 0x20; }")
f(0x4006C7F8, 24, "__USBC_Dev_Tx_ClearStall", P, "TXCSR &= ~0x30.", f"void __USBC_Dev_Tx_ClearStall(void) {{ {R16('USBC_CSR0', '& 0xFFCF')} }}")
f(0x4006C810, 20, "__USBC_Dev_Rx_SendStall", P, "RXCSR |= 0x20.", f"void __USBC_Dev_Rx_SendStall(void) {{ {R16('USBC_RXCSR', '| 0x20')} }}")
f(0x4006C824, 16, "__USBC_Dev_Rx_IsEpStall", P, "RXCSR bit 6.", "u32 __USBC_Dev_Rx_IsEpStall(void) { return REG16(USBC_RXCSR) & 0x40; }")
f(0x4006C834, 40, "__USBC_Dev_Rx_ClearStall", P, "RXCSR &= ~0x20, then &= ~0x40.",
  f"void __USBC_Dev_Rx_ClearStall(void) {{ {R16('USBC_RXCSR', '& 0xFFDF')} {R16('USBC_RXCSR', '& 0xFFBF')} }}")
for a, n, fns in ((0x4006C85C, "USBC_Dev_IsEpStall", ("__USBC_Dev_ep0_IsEpStall", "__USBC_Dev_Tx_IsEpStall", "__USBC_Dev_Rx_IsEpStall")),
                  (0x4006C8B4, "USBC_Dev_EpSendStall", ("__USBC_Dev_ep0_SendStall", "__USBC_Dev_Tx_SendStall", "__USBC_Dev_Rx_SendStall")),
                  (0x4006C90C, "USBC_Dev_EpClearStall", ("__USBC_Dev_ep0_ClearStall", "__USBC_Dev_Tx_ClearStall", "__USBC_Dev_Rx_ClearStall"))):
    extra = " The stall bit read is discarded: always 0 for a valid type (EasyUSB bug kept)." if "Is" in n else ""
    f(a, 88, n, P, f"EP0/TX/RX dispatch; -1 for another type.{extra}", f"""
int {n}(u32 ep_type)
{{
    switch (ep_type) {{
    case USBC_EP_TYPE_EP0: {fns[0]}(); break;
    case USBC_EP_TYPE_TX: {fns[1]}(); break;
    case USBC_EP_TYPE_RX: {fns[2]}(); break;
    default: return -1;
    }}
    return 0;
}}
""")
f(0x4006C964, 16, "__USBC_Dev_ep0_IsSetupEnd", P, "CSR0 bit 4.", "u32 __USBC_Dev_ep0_IsSetupEnd(void) { return REG16(USBC_CSR0) & 0x10; }")
f(0x4006C974, 4, "USBC_Dev_Ctrl_IsSetupEnd", P, "b __USBC_Dev_ep0_IsSetupEnd.", "u32 USBC_Dev_Ctrl_IsSetupEnd(void) { return __USBC_Dev_ep0_IsSetupEnd(); }")
f(0x4006C978, 20, "USBC_Dev_Ctrl_ClearSetupEnd", P, "CSR0 |= 0x80 (serviced setup end).", f"void USBC_Dev_Ctrl_ClearSetupEnd(void) {{ {R16('USBC_CSR0', '| 0x80')} }}")
f(0x4006C98C, 12, "USBC_Dev_Read_EP0_CSR", P, "CSR0.", "u32 USBC_Dev_Read_EP0_CSR(void) { return REG16(USBC_CSR0); }")
f(0x4006C998, 16, "USBC_SelectFIFO", P, "Endpoint FIFO address 0x01C19000 + 4*ep.", "void *USBC_SelectFIFO(u32 ep_index) { return (void *)USBC_EPFIFO(ep_index); }")
for a, n, sz, ad in ((0x4006C9A8, "__USBC_ConfigFifo_TxEp", "USBC_TXFIFOSZ", "USBC_TXFIFOAD"),
                     (0x4006C9FC, "__USBC_ConfigFifo_RxEp", "USBC_RXFIFOSZ", "USBC_RXFIFOAD")):
    f(a, 84, n, P, "FIFO size code = log2 of the 512-aligned size in 16-byte units; address in 8-byte units; bit 4 = double buffer.", f"""
void {n}(u32 is_double_fifo, u32 fifo_size, u32 fifo_addr)
{{
    u32 temp = ((fifo_size + 511) & ~511u) >> 4, size = 0;
    while (temp) {{
        size++;
        temp >>= 1;
    }}
    REG16({ad}) = (unsigned short)(fifo_addr >> 3);
    REG8({sz}) = (unsigned char)(size & 0x0F);
    if (is_double_fifo)
        REG8({sz}) = (unsigned char)(REG8({sz}) | 0x10);
}}
""")
f(0x4006CA50, 52, "USBC_ConfigFifo", P, "TX/RX dispatch (EP0 not supported).", """
void USBC_ConfigFifo(u32 ep_type, u32 is_double_fifo, u32 fifo_size, u32 fifo_addr)
{
    switch (ep_type) {
    case USBC_EP_TYPE_TX: __USBC_ConfigFifo_TxEp(is_double_fifo, fifo_size, fifo_addr); break;
    case USBC_EP_TYPE_RX: __USBC_ConfigFifo_RxEp(is_double_fifo, fifo_size, fifo_addr); break;
    default: break;
    }
}
""")
# ------------------------------------------------------------------ data status
f(0x4006CA84, 16, "__USBC_Dev_ep0_WriteDataHalf", P, "CSR0 = 0x02 (TxPktRdy).", "void __USBC_Dev_ep0_WriteDataHalf(void) { REG16(USBC_CSR0) = 0x02; }")
f(0x4006CA94, 16, "__USBC_Dev_ep0_WriteDataComplete", P, "CSR0 = 0x0A (TxPktRdy | DataEnd).", "void __USBC_Dev_ep0_WriteDataComplete(void) { REG16(USBC_CSR0) = 0x0A; }")
f(0x4006CAA4, 28, "__USBC_Dev_Tx_WriteDataHalf", P, "TXCSR = (TXCSR & ~4) | 1.", "void __USBC_Dev_Tx_WriteDataHalf(void) { REG16(USBC_CSR0) = (unsigned short)((REG16(USBC_CSR0) & 0xFFFB) | 1); }")
f(0x4006CAC0, 28, "__USBC_Dev_Tx_WriteDataComplete", P, "Same as the half version.", "void __USBC_Dev_Tx_WriteDataComplete(void) { REG16(USBC_CSR0) = (unsigned short)((REG16(USBC_CSR0) & 0xFFFB) | 1); }")
for a, n, e0, tx in ((0x4006CADC, "__USBC_Dev_WriteDataHalf", "__USBC_Dev_ep0_WriteDataHalf", "__USBC_Dev_Tx_WriteDataHalf"),
                     (0x4006CB28, "__USBC_Dev_WriteDataComplete", "__USBC_Dev_ep0_WriteDataComplete", "__USBC_Dev_Tx_WriteDataComplete")):
    f(a, 76, n, P, "EP0/TX dispatch; -1 otherwise.", f"""
int {n}(u32 ep_type)
{{
    switch (ep_type) {{
    case USBC_EP_TYPE_EP0: {e0}(); break;
    case USBC_EP_TYPE_TX: {tx}(); break;
    default: return -1;
    }}
    return 0;
}}
""")
f(0x4006CB74, 16, "__USBC_Dev_ep0_ReadDataHalf", P, "CSR0 = 0x40 (ServicedRxPktRdy).", "void __USBC_Dev_ep0_ReadDataHalf(void) { REG16(USBC_CSR0) = 0x40; }")
f(0x4006CB84, 16, "__USBC_Dev_ep0_ReadDataComplete", P, "CSR0 = 0x48 (ServicedRxPktRdy | DataEnd).", "void __USBC_Dev_ep0_ReadDataComplete(void) { REG16(USBC_CSR0) = 0x48; }")
f(0x4006CB94, 24, "__USBC_Dev_Rx_ReadDataHalf", P, "RXCSR &= 0xFFF2.", f"void __USBC_Dev_Rx_ReadDataHalf(void) {{ {R16('USBC_RXCSR', '& 0xFFF2')} }}")
f(0x4006CBAC, 24, "__USBC_Dev_Rx_ReadDataComplete", P, "RXCSR &= 0xFFF2.", f"void __USBC_Dev_Rx_ReadDataComplete(void) {{ {R16('USBC_RXCSR', '& 0xFFF2')} }}")
for a, n, e0, rx in ((0x4006CBC4, "__USBC_Dev_ReadDataHalf", "__USBC_Dev_ep0_ReadDataHalf", "__USBC_Dev_Rx_ReadDataHalf"),
                     (0x4006CC00, "__USBC_Dev_ReadDataComplete", "__USBC_Dev_ep0_ReadDataComplete", "__USBC_Dev_Rx_ReadDataComplete")):
    f(a, 60, n, P, "EP0/RX dispatch; -1 otherwise.", f"""
int {n}(u32 ep_type)
{{
    switch (ep_type) {{
    case USBC_EP_TYPE_EP0: {e0}(); break;
    case USBC_EP_TYPE_RX: {rx}(); break;
    default: return -1;
    }}
    return 0;
}}
""")
f(0x4006CC3C, 12, "USBC_Dev_WriteDataStatus", P, "complete ? __USBC_Dev_WriteDataComplete : __USBC_Dev_WriteDataHalf.", """
int USBC_Dev_WriteDataStatus(u32 ep_type, u32 complete)
{
    if (complete) return __USBC_Dev_WriteDataComplete(ep_type);
    return __USBC_Dev_WriteDataHalf(ep_type);
}
""")
f(0x4006CC48, 12, "USBC_Dev_ReadDataStatus", P, "complete ? __USBC_Dev_ReadDataComplete : __USBC_Dev_ReadDataHalf.", """
int USBC_Dev_ReadDataStatus(u32 ep_type, u32 complete)
{
    if (complete) return __USBC_Dev_ReadDataComplete(ep_type);
    return __USBC_Dev_ReadDataHalf(ep_type);
}
""")
f(0x4006CC54, 16, "__USBC_Dev_Tx_IsWriteDataReady", P, "TXCSR bit 0.", "u32 __USBC_Dev_Tx_IsWriteDataReady(void) { return REG16(USBC_CSR0) & 1; }")
f(0x4006CC64, 16, "__USBC_Dev_Rx_IsReadDataReady", P, "RXCSR bit 0.", "u32 __USBC_Dev_Rx_IsReadDataReady(void) { return REG16(USBC_RXCSR) & 1; }")
f(0x4006CC74, 16, "__USBC_Dev_ep0_IsReadDataReady", P, "CSR0 bit 0.", "u32 __USBC_Dev_ep0_IsReadDataReady(void) { return REG16(USBC_CSR0) & 1; }")
f(0x4006CC84, 16, "__USBC_Dev_ep0_IsWriteDataReady", P, "CSR0 bit 1.", "u32 __USBC_Dev_ep0_IsWriteDataReady(void) { return REG16(USBC_CSR0) & 2; }")
f(0x4006CC94, 32, "USBC_Dev_IsReadDataReady", P, "EP0/RX dispatch; 0 otherwise.", """
u32 USBC_Dev_IsReadDataReady(u32 ep_type)
{
    switch (ep_type) {
    case USBC_EP_TYPE_EP0: return __USBC_Dev_ep0_IsReadDataReady();
    case USBC_EP_TYPE_RX: return __USBC_Dev_Rx_IsReadDataReady();
    default: return 0;
    }
}
""")
f(0x4006CCB4, 44, "USBC_Dev_IsWriteDataReady", P, "EP0/TX dispatch; 0 otherwise.", """
u32 USBC_Dev_IsWriteDataReady(u32 ep_type)
{
    switch (ep_type) {
    case USBC_EP_TYPE_EP0: return __USBC_Dev_ep0_IsWriteDataReady();
    case USBC_EP_TYPE_TX: return __USBC_Dev_Tx_IsWriteDataReady();
    default: return 0;
    }
}
""")
f(0x4006CCE0, 48, "USBC_ReadLenFromFifo", P, "COUNT0 (EP0) or RXCOUNT (RX), same register; 0 otherwise.", """
u32 USBC_ReadLenFromFifo(u32 ep_type)
{
    switch (ep_type) {
    case USBC_EP_TYPE_EP0: return REG16(USBC_COUNT0);
    case USBC_EP_TYPE_RX: return REG16(USBC_COUNT0);
    default: return 0;
    }
}
""")
f(0x4006CD10, 68, "USBC_WritePacket", P, "Copy cnt bytes to a FIFO: 32-bit words, then the tail bytes. Returns cnt.", """
u32 USBC_WritePacket(void *fifo, u32 cnt, void *buff)
{
    u32 i, *b32 = buff;
    unsigned char *b8;
    for (i = cnt >> 2; i; i--)
        REG32(fifo) = *b32++;
    b8 = (unsigned char *)b32;
    for (i = cnt & 3; i; i--)
        REG8(fifo) = *b8++;
    return cnt;
}
""")
f(0x4006CD54, 68, "USBC_ReadPacket", P, "Copy cnt bytes from a FIFO: 32-bit words, then the tail bytes. Returns cnt.", """
u32 USBC_ReadPacket(void *fifo, u32 cnt, void *buff)
{
    u32 i, *b32 = buff;
    unsigned char *b8;
    for (i = cnt >> 2; i; i--)
        *b32++ = REG32(fifo);
    b8 = (unsigned char *)b32;
    for (i = cnt & 3; i; i--)
        *b8++ = REG8(fifo);
    return cnt;
}
""")
# ------------------------------------------------------------------ endpoint config
f(0x4006CD98, 20, "__USBC_Dev_Tx_EnableIsoEp", P, "TXCSR |= 0x4000.", f"void __USBC_Dev_Tx_EnableIsoEp(void) {{ {R16('USBC_CSR0', '| 0x4000')} }}")
f(0x4006CDAC, 24, "__USBC_Dev_Tx_EnableIntEp", P, "TXCSR &= ~0x4000.", f"void __USBC_Dev_Tx_EnableIntEp(void) {{ {R16('USBC_CSR0', '& 0xBFFF')} }}")
f(0x4006CDC4, 24, "__USBC_Dev_Tx_EnableBulkEp", P, "TXCSR &= ~0x4000.", f"void __USBC_Dev_Tx_EnableBulkEp(void) {{ {R16('USBC_CSR0', '& 0xBFFF')} }}")
for a, n, csr, maxp, init, iso, intr, bulk in (
        (0x4006CDDC, "__USBC_Dev_Tx_ConfigEp", "USBC_CSR0", "USBC_TXMAXP", "0x2048", "__USBC_Dev_Tx_EnableIsoEp", "__USBC_Dev_Tx_EnableIntEp", "__USBC_Dev_Tx_EnableBulkEp"),
        (0x4006CE68, "__USBC_Dev_Rx_ConfigEp", "USBC_RXCSR", "USBC_RXMAXP", "0x0090", "__USBC_Dev_Rx_EnableIsoEp", "__USBC_Dev_Rx_EnableIntEp", "__USBC_Dev_Rx_EnableBulkEp")):
    f(a, 72, n, P, f"CSR = {init} (flush + clear toggle; written twice for a double FIFO), MAXP |= max packet (11 bits), then the type (2 = ISO, 3 = INT, else bulk).", f"""
void {n}(u32 ts_type, u32 is_double_fifo, u32 ep_MaxPkt)
{{
    REG16({csr}) = {init};
    if (is_double_fifo)
        REG16({csr}) = {init};
    REG16({maxp}) = (unsigned short)(REG16({maxp}) | (ep_MaxPkt & 0x7FF));
    switch (ts_type) {{
    case 2: {iso}(); break;
    case 3: {intr}(); break;
    default: {bulk}(); break;
    }}
}}
""")
f(0x4006CE24, 20, "__USBC_Dev_Rx_EnableIsoEp", P, "RXCSR |= 0x4000.", f"void __USBC_Dev_Rx_EnableIsoEp(void) {{ {R16('USBC_RXCSR', '| 0x4000')} }}")
f(0x4006CE38, 24, "__USBC_Dev_Rx_EnableIntEp", P, "RXCSR &= ~0x4000.", f"void __USBC_Dev_Rx_EnableIntEp(void) {{ {R16('USBC_RXCSR', '& 0xBFFF')} }}")
f(0x4006CE50, 24, "__USBC_Dev_Rx_EnableBulkEp", P, "RXCSR &= ~0x4000.", f"void __USBC_Dev_Rx_EnableBulkEp(void) {{ {R16('USBC_RXCSR', '& 0xBFFF')} }}")
f(0x4006CEB0, 16, "__USBC_Dev_ep0_ConfigEp0", P, "CSR0 = 0x100 (flush FIFO).", "void __USBC_Dev_ep0_ConfigEp0(void) { REG16(USBC_CSR0) = 0x100; }")
f(0x4006CEC0, 100, "USBC_Dev_ConfigEp", P, "EP0/TX/RX dispatch; -1 otherwise.", """
int USBC_Dev_ConfigEp(u32 ts_type, u32 ep_type, u32 is_double_fifo, u32 ep_MaxPkt)
{
    switch (ep_type) {
    case USBC_EP_TYPE_EP0: __USBC_Dev_ep0_ConfigEp0(); break;
    case USBC_EP_TYPE_TX: __USBC_Dev_Tx_ConfigEp(ts_type, is_double_fifo, ep_MaxPkt); break;
    case USBC_EP_TYPE_RX: __USBC_Dev_Rx_ConfigEp(ts_type, is_double_fifo, ep_MaxPkt); break;
    default: return -1;
    }
    return 0;
}
""")
f(0x4006CF24, 56, "USBC_Clear_Feature_HALT", P, "TX: TXCSR = 0x2048; RX: RXCSR = 0x90.", """
void USBC_Clear_Feature_HALT(u32 ep_type)
{
    switch (ep_type) {
    case USBC_EP_TYPE_TX: REG16(USBC_CSR0) = 0x2048; break;
    case USBC_EP_TYPE_RX: REG16(USBC_RXCSR) = 0x90; break;
    default: break;
    }
}
""")


for _part in sorted((ROOT / "tools/usb_spec").glob("*.py")):   # usb_dev.c, class drivers
    exec(compile(_part.read_text(), str(_part), "exec"))


def main():
    if "--csv" in sys.argv:
        for addr, size, name, src, desc, body in SPEC:
            origin = "this-project:phase5 (FNIRSI code; EasyUSB-style API)" if src.startswith("FNIRSI") else f"EasyUSB:{src} (this-project:phase5)"
            print(f"0x{addr:08X},{name},function,DERIVED,{origin},")
        return
    OUT.mkdir(parents=True, exist_ok=True)
    for addr, size, name, src, desc, body in SPEC:
        (OUT / f"func_{addr:08x}.c").write_text(HDR.format(name=name, addr=addr, size=size, desc=desc) + body)
    print(f"{len(SPEC)} files in {OUT.relative_to(ROOT)}")


if __name__ == "__main__":
    main()
