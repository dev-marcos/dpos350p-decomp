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
#define STR(a) ((const char *)(a))    /* stock string literal (keeps call arguments identical) */



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

#endif
