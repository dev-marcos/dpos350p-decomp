/* V3s USB OTG controller (Mentor MUSB with the Allwinner "USBC" register layout) and the
 * EasyUSB-derived device stack of the stock firmware. See docs/USB.md.
 * Function names follow EasyUSB (VeiLiang, usb_phy/f1c100s/usb_phy.c, usb_dev.c): DERIVED.
 * Register offsets are those the stock code uses (base 0x01C19000): STATIC-CONFIRMED. */
#ifndef HW_USB_H
#define HW_USB_H
#include "hw/v3s.h"

#define REG16(a) (*(volatile unsigned short *)(a))

#define USBC0_BASE      0x01C19000u
#define USBC_EPFIFO(n)  (USBC0_BASE + ((u32)(n) << 2))
#define USBC_PCTL       (USBC0_BASE + 0x40)   /* POWER, 8 bit */
#define USBC_EPIND      (USBC0_BASE + 0x42)
#define USBC_VEND0      (USBC0_BASE + 0x43)
#define USBC_INTTx      (USBC0_BASE + 0x44)
#define USBC_INTRx      (USBC0_BASE + 0x46)
#define USBC_INTTxE     (USBC0_BASE + 0x48)
#define USBC_INTRxE     (USBC0_BASE + 0x4A)
#define USBC_INTUSB     (USBC0_BASE + 0x4C)
#define USBC_INTUSBE    (USBC0_BASE + 0x50)
#define USBC_TXMAXP     (USBC0_BASE + 0x80)
#define USBC_CSR0       (USBC0_BASE + 0x82)   /* = TXCSR for endpoints > 0 */
#define USBC_RXMAXP     (USBC0_BASE + 0x84)
#define USBC_RXCSR      (USBC0_BASE + 0x86)
#define USBC_COUNT0     (USBC0_BASE + 0x88)   /* = RXCOUNT */
#define USBC_TXFIFOSZ   (USBC0_BASE + 0x90)
#define USBC_TXFIFOAD   (USBC0_BASE + 0x92)
#define USBC_RXFIFOSZ   (USBC0_BASE + 0x94)
#define USBC_RXFIFOAD   (USBC0_BASE + 0x96)
#define USBC_FADDR      (USBC0_BASE + 0x98)
#define USBC_PHY_CSR    (USBC0_BASE + 0x410)

/* EasyUSB enums as the stock code uses them */
#define USBC_EP_TYPE_IDLE 0
#define USBC_EP_TYPE_EP0  1
#define USBC_EP_TYPE_TX   2
#define USBC_EP_TYPE_RX   3

/* debug output: the stock build links usbprint to a variadic no-op */
void debug_printf_stub(const char *fmt, ...);
#define STR(a) ((const char *)(a))    /* stock string literal (keeps call arguments identical) */

void usb_phy_write(int addr, int data, int len);
u32 USBC_INT_TxPending(void);
void USBC_INT_ClearTxPending(u32 ep_index);
void USBC_INT_ClearTxPendingAll(void);
u32 USBC_INT_RxPending(void);
void USBC_INT_ClearRxPending(u32 ep_index);
void USBC_INT_ClearRxPendingAll(void);
void USBC_INT_EnableTxEp(u32 ep_index);
void USBC_INT_EnableRxEp(u32 ep_index);
void USBC_INT_DisableTxAll(void);
void USBC_INT_DisableRxAll(void);
void __USBC_Dev_TsType_default(void);
void __USBC_Dev_TsType_Ctrl(void);
void __USBC_Dev_TsType_Iso(void);
void __USBC_Dev_TsType_Int(void);
void __USBC_Dev_TsType_Bulk(void);
void __USBC_Dev_TsMode_default(void);
void __USBC_Dev_TsMode_Hs(void);
void __USBC_Dev_TsMode_Fs(void);
void __USBC_Dev_TsMode_Ls(void);
u32 __USBC_Dev_ep0_IsEpStall(void);
void __USBC_Dev_ep0_SendStall(void);
void __USBC_Dev_ep0_ClearStall(void);
void __USBC_Dev_Tx_SendStall(void);
u32 __USBC_Dev_Tx_IsEpStall(void);
void __USBC_Dev_Tx_ClearStall(void);
void __USBC_Dev_Rx_SendStall(void);
u32 __USBC_Dev_Rx_IsEpStall(void);
void __USBC_Dev_Rx_ClearStall(void);
u32 __USBC_Dev_ep0_IsSetupEnd(void);
void __USBC_ConfigFifo_TxEp(u32 is_double_fifo, u32 fifo_size, u32 fifo_addr);
void __USBC_ConfigFifo_RxEp(u32 is_double_fifo, u32 fifo_size, u32 fifo_addr);
void __USBC_Dev_ep0_WriteDataHalf(void);
void __USBC_Dev_ep0_WriteDataComplete(void);
void __USBC_Dev_Tx_WriteDataHalf(void);
void __USBC_Dev_Tx_WriteDataComplete(void);
int __USBC_Dev_WriteDataHalf(u32 ep_type);
int __USBC_Dev_WriteDataComplete(u32 ep_type);
void __USBC_Dev_ep0_ReadDataHalf(void);
void __USBC_Dev_ep0_ReadDataComplete(void);
void __USBC_Dev_Rx_ReadDataHalf(void);
void __USBC_Dev_Rx_ReadDataComplete(void);
int __USBC_Dev_ReadDataHalf(u32 ep_type);
int __USBC_Dev_ReadDataComplete(u32 ep_type);
u32 __USBC_Dev_Tx_IsWriteDataReady(void);
u32 __USBC_Dev_Rx_IsReadDataReady(void);
u32 __USBC_Dev_ep0_IsReadDataReady(void);
u32 __USBC_Dev_ep0_IsWriteDataReady(void);
void __USBC_Dev_Tx_EnableIsoEp(void);
void __USBC_Dev_Tx_EnableIntEp(void);
void __USBC_Dev_Tx_EnableBulkEp(void);
void __USBC_Dev_Rx_EnableIsoEp(void);
void __USBC_Dev_Rx_EnableIntEp(void);
void __USBC_Dev_Rx_EnableBulkEp(void);
void __USBC_Dev_Tx_ConfigEp(u32 ts_type, u32 is_double_fifo, u32 ep_MaxPkt);
void __USBC_Dev_Rx_ConfigEp(u32 ts_type, u32 is_double_fifo, u32 ep_MaxPkt);
void __USBC_Dev_ep0_ConfigEp0(void);


/* ---- usb_dev.c state (EasyUSB names; addresses STATIC-CONFIRMED, names DERIVED) */
#define usb_tx_buf        ((unsigned char *)0x42E6825Cu)   /* [512] */
#define usb_rx_buf        ((unsigned char *)0x42E6845Cu)   /* [512] */
#define fifo_offset       (*(u32 *)0x42E68CF0u)
#define usb_connect       (*(u32 *)0x42E68CF4u)
#define usb_ep0_state     (*(int *)0x42E68CF8u)
#define usb_tx_buf_len    (*(int *)0x42E68CFCu)
#define usb_tx_pos        (*(int *)0x42E68D00u)
#define usb_rx_buf_len    (*(int *)0x42E68D04u)
#define usb_tx_data       (*(unsigned char **)0x42E68D08u)  /* FNIRSI: caller's buffer, not usb_tx_buf */
#define current_usb_type  (*(unsigned char *)0x42E68D1Fu)
#define current_speed     (*(unsigned char *)0x42E68D20u)
#define usb_addr          (*(unsigned char *)0x42E68D21u)
#define usb_xfer_type     (*(unsigned char *)0x42E68D22u)
#define ep_max_len        ((int *)0x40095470u)              /* {64, 512, 512} in .data */

void *memcpy(void *d, const void *s, unsigned int n);
void dbg_printf(const char *fmt, ...);
int irq_register(int irq, void (*fn)(void), int a, int b);
void irq_enable(void);
void USBC_SelectActiveEp(u32 ep_index);
u32 USBC_GetActiveEp(void);
void *USBC_SelectFIFO(u32 ep_index);
u32 USBC_WritePacket(void *fifo, u32 cnt, void *buff);
u32 USBC_ReadPacket(void *fifo, u32 cnt, void *buff);
int USBC_Dev_WriteDataStatus(u32 ep_type, u32 complete);
int USBC_Dev_ReadDataStatus(u32 ep_type, u32 complete);
u32 USBC_Dev_IsWriteDataReady(u32 ep_type);
u32 USBC_Dev_IsReadDataReady(u32 ep_type);
u32 USBC_ReadLenFromFifo(u32 ep_type);
int USBC_Dev_IsEpStall(u32 ep_type);
int USBC_Dev_EpSendStall(u32 ep_type);
int USBC_Dev_EpClearStall(u32 ep_type);
u32 USBC_Dev_Ctrl_IsSetupEnd(void);
void USBC_Dev_Ctrl_ClearSetupEnd(void);
u32 USBC_Dev_Read_EP0_CSR(void);
void USBC_Dev_SetAddress(u32 address);
void USBC_Dev_SetAddress_default(void);
u32 usb_phy_open_clock_Device(void);
u32 usb_phy_close_clock_Device(void);
void USBC_PhyConfig(void);
void USBC_ConfigFIFO_Base(void);
void USBC_SelectBus(u32 io_type, u32 ep_type, u32 ep_index);
void USBC_INT_ClearEpPendingAll(u32 ep_type);
void USBC_INT_ClearEpPending(u32 ep_type, u32 ep_index);
void USBC_INT_ClearMiscPendingAll(void);
void USBC_INT_ClearMiscPending(u32 mask);
u32 USBC_INT_MiscPending(void);
u32 USBC_INT_EpPending(u32 ep_type);
void USBC_INT_DisableUsbMiscAll(void);
void USBC_INT_DisableEpAll(u32 ep_type);
void USBC_INT_EnableEp(u32 ep_type, u32 ep_index);
void USBC_INT_EnableUsbMiscUint(u32 mask);
void USBC_Dev_ConectSwitch(u32 is_on);
void USBC_Dev_ConfigTransferMode(u32 ts_type, u32 speed_mode);
u32 USBC_Dev_QueryTransferMode(void);
int USBC_Dev_ConfigEp(u32 ts_type, u32 ep_type, u32 is_double_fifo, u32 ep_MaxPkt);
void USBC_ConfigFifo(u32 ep_type, u32 is_double_fifo, u32 fifo_size, u32 fifo_addr);
void USBC_Clear_Feature_HALT(u32 ep_type);
#endif
