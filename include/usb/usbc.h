/* Functions of nonmatching/usb/usbc/ (one file per function, named after it).
 * Addresses are of build 2025-06-19. Names are DERIVED unless the C file says otherwise. */
#ifndef USB_USBC_H
#define USB_USBC_H

#include "rt/bits.h"

u32 USBC_WakeUp_ClearChangeDetect(u32 reg_val);                                                  /* 0x4006C10C */
void USBC_EnableDpDmPullUp(void);                                                                /* 0x4006C114 */
void USBC_EnableIdPullUp(void);                                                                  /* 0x4006C134 */
void USBC_ForceIdToLow(void);                                                                    /* 0x4006C154 */
void USBC_ForceIdToHigh(void);                                                                   /* 0x4006C178 */
void USBC_ForceIdDisable(void);                                                                  /* 0x4006C198 */
void USBC_ForceId(u32 type);                                                                     /* 0x4006C1B8 */
void USBC_ForceVbusValidDisable(void);                                                           /* 0x4006C1DC */
void USBC_ForceVbusValidToLow(void);                                                             /* 0x4006C1FC */
void USBC_ForceVbusValidToHigh(void);                                                            /* 0x4006C220 */
void USBC_ForceVbusValid(u32 type);                                                              /* 0x4006C240 */
void USBC_SelectBus(u32 io_type, u32 ep_type, u32 ep_index);                                     /* 0x4006C264 */
void USBC_PhyConfig(void);                                                                       /* 0x4006C328 */
void USBC_ConfigFIFO_Base(void);                                                                 /* 0x4006C368 */
void USBC_INT_DisableUsbMiscAll(void);                                                           /* 0x4006C424 */
u32 USBC_INT_TxPending(void);                                                                    /* 0x4006C434 */
void USBC_INT_ClearTxPending(u32 ep_index);                                                      /* 0x4006C440 */
void USBC_INT_ClearTxPendingAll(void);                                                           /* 0x4006C454 */
u32 USBC_INT_RxPending(void);                                                                    /* 0x4006C468 */
void USBC_INT_ClearRxPending(u32 ep_index);                                                      /* 0x4006C474 */
void USBC_INT_ClearRxPendingAll(void);                                                           /* 0x4006C488 */
void USBC_INT_EnableTxEp(u32 ep_index);                                                          /* 0x4006C49C */
void USBC_INT_EnableRxEp(u32 ep_index);                                                          /* 0x4006C4B4 */
void USBC_INT_DisableTxAll(void);                                                                /* 0x4006C4CC */
void USBC_INT_WriteTxE(u32 v);                                                                   /* 0x4006C4D0 */
void USBC_INT_DisableRxAll(void);                                                                /* 0x4006C4DC */
void USBC_INT_WriteRxE(u32 v);                                                                   /* 0x4006C4E0 */
u32 USBC_INT_EpPending(u32 ep_type);                                                             /* 0x4006C4EC */
void USBC_INT_ClearEpPending(u32 ep_type, u32 ep_index);                                         /* 0x4006C510 */
void USBC_INT_ClearEpPendingAll(u32 ep_type);                                                    /* 0x4006C538 */
u32 USBC_INT_MiscPending(void);                                                                  /* 0x4006C558 */
void USBC_INT_ClearMiscPending(u32 mask);                                                        /* 0x4006C564 */
void USBC_INT_ClearMiscPendingAll(void);                                                         /* 0x4006C570 */
void USBC_INT_EnableEp(u32 ep_type, u32 ep_index);                                               /* 0x4006C580 */
void USBC_INT_EnableUsbMiscUint(u32 mask);                                                       /* 0x4006C5AC */
u32 USBC_GetActiveEp(void);                                                                      /* 0x4006C5C0 */
void USBC_SelectActiveEp(u32 ep_index);                                                          /* 0x4006C5CC */
void USBC_Dev_ConectSwitch(u32 is_on);                                                           /* 0x4006C5D8 */
void __USBC_Dev_TsType_default(void);                                                            /* 0x4006C604 */
void __USBC_Dev_TsType_Ctrl(void);                                                               /* 0x4006C618 */
void __USBC_Dev_TsType_Iso(void);                                                                /* 0x4006C62C */
void __USBC_Dev_TsType_Int(void);                                                                /* 0x4006C640 */
void __USBC_Dev_TsType_Bulk(void);                                                               /* 0x4006C654 */
void __USBC_Dev_TsMode_default(void);                                                            /* 0x4006C668 */
void __USBC_Dev_TsMode_Hs(void);                                                                 /* 0x4006C67C */
void __USBC_Dev_TsMode_Fs(void);                                                                 /* 0x4006C690 */
void __USBC_Dev_TsMode_Ls(void);                                                                 /* 0x4006C6A4 */
void USBC_Dev_ConfigTransferMode(u32 ts_type, u32 speed_mode);                                   /* 0x4006C6A8 */
u32 USBC_Dev_QueryTransferMode(void);                                                            /* 0x4006C730 */
void USBC_INT_DisableEpAll(u32 ep_type);                                                         /* 0x4006C748 */
void USBC_Dev_SetAddress_default(void);                                                          /* 0x4006C76C */
void USBC_Dev_WriteFAddr(u32 v);                                                                 /* 0x4006C770 */
void USBC_Dev_SetAddress(u32 address);                                                           /* 0x4006C77C */
u32 __USBC_Dev_ep0_IsEpStall(void);                                                              /* 0x4006C788 */
void __USBC_Dev_ep0_SendStall(void);                                                             /* 0x4006C798 */
void __USBC_Dev_ep0_ClearStall(void);                                                            /* 0x4006C7AC */
void __USBC_Dev_Tx_SendStall(void);                                                              /* 0x4006C7D4 */
u32 __USBC_Dev_Tx_IsEpStall(void);                                                               /* 0x4006C7E8 */
void __USBC_Dev_Tx_ClearStall(void);                                                             /* 0x4006C7F8 */
void __USBC_Dev_Rx_SendStall(void);                                                              /* 0x4006C810 */
u32 __USBC_Dev_Rx_IsEpStall(void);                                                               /* 0x4006C824 */
void __USBC_Dev_Rx_ClearStall(void);                                                             /* 0x4006C834 */
int USBC_Dev_IsEpStall(u32 ep_type);                                                             /* 0x4006C85C */
int USBC_Dev_EpSendStall(u32 ep_type);                                                           /* 0x4006C8B4 */
int USBC_Dev_EpClearStall(u32 ep_type);                                                          /* 0x4006C90C */
u32 __USBC_Dev_ep0_IsSetupEnd(void);                                                             /* 0x4006C964 */
u32 USBC_Dev_Ctrl_IsSetupEnd(void);                                                              /* 0x4006C974 */
void USBC_Dev_Ctrl_ClearSetupEnd(void);                                                          /* 0x4006C978 */
u32 USBC_Dev_Read_EP0_CSR(void);                                                                 /* 0x4006C98C */
void *USBC_SelectFIFO(u32 ep_index);                                                             /* 0x4006C998 */
void __USBC_ConfigFifo_TxEp(u32 is_double_fifo, u32 fifo_size, u32 fifo_addr);                   /* 0x4006C9A8 */
void __USBC_ConfigFifo_RxEp(u32 is_double_fifo, u32 fifo_size, u32 fifo_addr);                   /* 0x4006C9FC */
void USBC_ConfigFifo(u32 ep_type, u32 is_double_fifo, u32 fifo_size, u32 fifo_addr);             /* 0x4006CA50 */
void __USBC_Dev_ep0_WriteDataHalf(void);                                                         /* 0x4006CA84 */
void __USBC_Dev_ep0_WriteDataComplete(void);                                                     /* 0x4006CA94 */
void __USBC_Dev_Tx_WriteDataHalf(void);                                                          /* 0x4006CAA4 */
void __USBC_Dev_Tx_WriteDataComplete(void);                                                      /* 0x4006CAC0 */
int __USBC_Dev_WriteDataHalf(u32 ep_type);                                                       /* 0x4006CADC */
int __USBC_Dev_WriteDataComplete(u32 ep_type);                                                   /* 0x4006CB28 */
void __USBC_Dev_ep0_ReadDataHalf(void);                                                          /* 0x4006CB74 */
void __USBC_Dev_ep0_ReadDataComplete(void);                                                      /* 0x4006CB84 */
void __USBC_Dev_Rx_ReadDataHalf(void);                                                           /* 0x4006CB94 */
void __USBC_Dev_Rx_ReadDataComplete(void);                                                       /* 0x4006CBAC */
int __USBC_Dev_ReadDataHalf(u32 ep_type);                                                        /* 0x4006CBC4 */
int __USBC_Dev_ReadDataComplete(u32 ep_type);                                                    /* 0x4006CC00 */
int USBC_Dev_WriteDataStatus(u32 ep_type, u32 complete);                                         /* 0x4006CC3C */
int USBC_Dev_ReadDataStatus(u32 ep_type, u32 complete);                                          /* 0x4006CC48 */
u32 __USBC_Dev_Tx_IsWriteDataReady(void);                                                        /* 0x4006CC54 */
u32 __USBC_Dev_Rx_IsReadDataReady(void);                                                         /* 0x4006CC64 */
u32 __USBC_Dev_ep0_IsReadDataReady(void);                                                        /* 0x4006CC74 */
u32 __USBC_Dev_ep0_IsWriteDataReady(void);                                                       /* 0x4006CC84 */
u32 USBC_Dev_IsReadDataReady(u32 ep_type);                                                       /* 0x4006CC94 */
u32 USBC_Dev_IsWriteDataReady(u32 ep_type);                                                      /* 0x4006CCB4 */
u32 USBC_ReadLenFromFifo(u32 ep_type);                                                           /* 0x4006CCE0 */
u32 USBC_WritePacket(void *fifo, u32 cnt, void *buff);                                           /* 0x4006CD10 */
u32 USBC_ReadPacket(void *fifo, u32 cnt, void *buff);                                            /* 0x4006CD54 */
void __USBC_Dev_Tx_EnableIsoEp(void);                                                            /* 0x4006CD98 */
void __USBC_Dev_Tx_EnableIntEp(void);                                                            /* 0x4006CDAC */
void __USBC_Dev_Tx_EnableBulkEp(void);                                                           /* 0x4006CDC4 */
void __USBC_Dev_Tx_ConfigEp(u32 ts_type, u32 is_double_fifo, u32 ep_MaxPkt);                     /* 0x4006CDDC */
void __USBC_Dev_Rx_EnableIsoEp(void);                                                            /* 0x4006CE24 */
void __USBC_Dev_Rx_EnableIntEp(void);                                                            /* 0x4006CE38 */
void __USBC_Dev_Rx_EnableBulkEp(void);                                                           /* 0x4006CE50 */
void __USBC_Dev_Rx_ConfigEp(u32 ts_type, u32 is_double_fifo, u32 ep_MaxPkt);                     /* 0x4006CE68 */
void __USBC_Dev_ep0_ConfigEp0(void);                                                             /* 0x4006CEB0 */
int USBC_Dev_ConfigEp(u32 ts_type, u32 ep_type, u32 is_double_fifo, u32 ep_MaxPkt);              /* 0x4006CEC0 */
void USBC_Clear_Feature_HALT(u32 ep_type);                                                       /* 0x4006CF24 */

#endif
