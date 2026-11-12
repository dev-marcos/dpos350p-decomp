# usb_dev.c core (EasyUSB, FNIRSI-modified).  exec'd by gen_usb.py
D = "usb_dev.c"
f(0x4006AA28, 196, "usb_device_write_data_ep_pack", D,
  "Queue an IN transfer: copy to usb_tx_buf, keep the caller's buffer (FNIRSI) and length, send the first packet; usb_tx_pos = 64 if more follows.", """
int usb_device_write_data_ep_pack(int ep, unsigned char *databuf, int len)
{
    memcpy(usb_tx_buf, databuf, len);
    usb_tx_data = databuf;
    usb_tx_buf_len = len;
    debug_printf_stub(STR(0x4008EED8), len);
    if (len > ep_max_len[ep]) {
        USBC_WritePacket(USBC_SelectFIFO(ep), ep_max_len[ep], databuf);
        USBC_Dev_WriteDataStatus(ep == 0 ? USBC_EP_TYPE_EP0 : USBC_EP_TYPE_TX, 0);
        usb_tx_pos = 64;
    } else {
        USBC_WritePacket(USBC_SelectFIFO(ep), len, databuf);
        USBC_Dev_WriteDataStatus(ep == 0 ? USBC_EP_TYPE_EP0 : USBC_EP_TYPE_TX, 1);
        usb_tx_pos = 0;
    }
    return 0;
}
""")
f(0x4006AAEC, 44, "usb_device_read_data_ep_pack", D, "Read len bytes from an endpoint FIFO.", """
int usb_device_read_data_ep_pack(int ep, unsigned char *databuf, int len)
{
    USBC_ReadPacket(USBC_SelectFIFO(ep), len, databuf);
    return 0;
}
""")
f(0x4006AB18, 12, "usb_device_send_nullpack_ep0", D, "EP0 write status, complete.",
  "void usb_device_send_nullpack_ep0(void) { USBC_Dev_WriteDataStatus(USBC_EP_TYPE_EP0, 1); }")
f(0x4006AB24, 12, "usb_device_read_data_status_ep0", D, "EP0 read status.",
  "void usb_device_read_data_status_ep0(u32 is_complete) { USBC_Dev_ReadDataStatus(USBC_EP_TYPE_EP0, is_complete); }")
f(0x4006AB30, 272, "usb_device_write_data", D,
  "Bulk IN: ep_max_len-sized packets, up to 10 000 000 polls for each FIFO slot; -1 on timeout or when not connected.", """
int usb_device_write_data(int ep, unsigned char *databuf, int len)
{
    volatile int Timeout = 10000000;
    int pack_len = ep_max_len[ep], write_len, data_pos = 0, length = len;
    u32 old;
    void *fifo;
    if (!usb_connect) {
        debug_printf_stub(STR(0x4008D2CC));
        return -1;
    }
    debug_printf_stub(STR(0x4008DBA0), len);
    old = USBC_GetActiveEp();
    fifo = USBC_SelectFIFO(ep);
    USBC_SelectActiveEp((unsigned char)ep);
    do {
        while (USBC_Dev_IsWriteDataReady(USBC_EP_TYPE_TX) && --Timeout)
            ;
        write_len = pack_len > length ? length : pack_len;
        USBC_WritePacket(fifo, write_len, databuf + data_pos);
        USBC_Dev_WriteDataStatus(USBC_EP_TYPE_TX, 1);
        data_pos += write_len;
        length -= write_len;
        if (Timeout == 0) {
            USBC_SelectActiveEp((unsigned char)old);
            debug_printf_stub(STR(0x4008DBC0));
            return -1;
        }
        Timeout = 10000000;
    } while (length > 0);
    USBC_SelectActiveEp((unsigned char)old);
    return 0;
}
""")
f(0x4006AC40, 4, "usb_device_clear_setup_end", D, "b USBC_Dev_Ctrl_ClearSetupEnd.",
  "void usb_device_clear_setup_end(void) { USBC_Dev_Ctrl_ClearSetupEnd(); }")
f(0x4006AC44, 24, "usb_device_set_address", D, "FADDR and usb_addr.", """
void usb_device_set_address(unsigned char addr)
{
    USBC_Dev_SetAddress(addr);
    usb_addr = addr;
}
""")
f(0x4006AC5C, 12, "usb_device_set_ep0_state", D, "usb_ep0_state = state.",
  "void usb_device_set_ep0_state(int state) { usb_ep0_state = state; }")
f(0x4006AC68, 80, "usb_dev_bsp_init", D,
  "Clocks, PHY CSR = 2 around the PHY setup, FIFO base, pull-ups, force ID = device and VBUS valid high, PIO bus.", """
void USBC_EnableDpDmPullUp(void);
void USBC_EnableIdPullUp(void);
void USBC_ForceId(u32 type);
void USBC_ForceVbusValid(u32 type);

int usb_dev_bsp_init(void)
{
    usb_phy_open_clock_Device();
    REG32(USBC_PHY_CSR) = 2;
    USBC_PhyConfig();
    USBC_ConfigFIFO_Base();
    USBC_EnableDpDmPullUp();
    USBC_EnableIdPullUp();
    USBC_ForceId(2);
    USBC_ForceVbusValid(2);
    USBC_SelectBus(0, 0, 0);
    REG32(USBC_PHY_CSR) = 0;
    return 0;
}
""")
f(0x4006ACB8, 36, "clear_all_irq", D, "Clear TX, RX and misc pending interrupts.", """
void clear_all_irq(void)
{
    USBC_INT_ClearEpPendingAll(USBC_EP_TYPE_TX);
    USBC_INT_ClearEpPendingAll(USBC_EP_TYPE_RX);
    USBC_INT_ClearMiscPendingAll();
}
""")
f(0x4006ACDC, 48, "filtrate_irq_misc", D, "Clear VBUS error, session request, connect and SOF; return the other bits.", """
u32 filtrate_irq_misc(u32 irq_misc)
{
    USBC_INT_ClearMiscPending(0x80);
    USBC_INT_ClearMiscPending(0x40);
    USBC_INT_ClearMiscPending(0x10);
    USBC_INT_ClearMiscPending(0x08);
    return irq_misc & ~0xD8u;
}
""")
f(0x4006AD0C, 32, "usbd_start_work", D, "Soft connect on.", """
void usbd_start_work(void)
{
    debug_printf_stub(STR(0x4008EEEC));
    USBC_Dev_ConectSwitch(1);
}
""")
f(0x4006AD2C, 32, "usbd_stop_work", D, "Soft connect off.", """
void usbd_stop_work(void)
{
    debug_printf_stub(STR(0x4008F9AC));
    USBC_Dev_ConectSwitch(0);
}
""")
f(0x4006AD4C, 28, "cfg_udc_command", D, "1 = start, 2 = stop, 3 = 'reset not supported', else 'unknown cmd'.", """
void usbd_start_work(void);
void usbd_stop_work(void);

void cfg_udc_command(u32 cmd)
{
    switch (cmd) {
    case 1: usbd_start_work(); break;
    case 2: usbd_stop_work(); break;
    case 3: debug_printf_stub(STR(0x4008E19C)); break;
    default: debug_printf_stub(STR(0x4008E814), cmd); break;
    }
}
""")
f(0x4006AD84, 56, "sunxi_udc_disable", D, "All USB interrupts off and cleared, then soft disconnect.", """
void clear_all_irq(void);
void cfg_udc_command(u32 cmd);

void sunxi_udc_disable(void)
{
    debug_printf_stub(STR(0x4008EF00));
    USBC_INT_DisableUsbMiscAll();
    USBC_INT_DisableEpAll(USBC_EP_TYPE_RX);
    USBC_INT_DisableEpAll(USBC_EP_TYPE_TX);
    clear_all_irq();
    cfg_udc_command(2);
}
""")
f(0x4006ADBC, 804, "usb_handle_ep0", D,
  "EP0 state machine: IDLE reads the 8-byte setup packet, IN continues a long reply from usb_tx_data (the last packet sets usb_tx_pos = 0), OUT reads and prints, END clears usb_xfer_type. FNIRSI sends every setup packet to usb_setup_handle.", """
int usb_device_read_data_ep_pack(int ep, unsigned char *databuf, int len);
void usb_device_read_data_status_ep0(u32 is_complete);
void usb_setup_handle(unsigned char *req, int len);

void usb_handle_ep0(void)
{
    unsigned char d[8];
    int len, timeout;
    USBC_SelectActiveEp(0);
    if (USBC_Dev_IsEpStall(USBC_EP_TYPE_EP0)) {
        debug_printf_stub(STR(0x4008F9BC));
        USBC_Dev_EpClearStall(USBC_EP_TYPE_EP0);
        usb_ep0_state = 0;
        return;
    }
    if (USBC_Dev_Ctrl_IsSetupEnd()) {
        debug_printf_stub(STR(0x4008E1B8));
        USBC_Dev_Ctrl_ClearSetupEnd();
        usb_ep0_state = 0;
    }
    debug_printf_stub(STR(0x4008EF14), usb_ep0_state);
    debug_printf_stub(STR(0x4008F9CC), USBC_Dev_Read_EP0_CSR());
    if (USBC_Dev_IsReadDataReady(USBC_EP_TYPE_EP0)) {
        usb_ep0_state = 0;
    } else {
        switch ((u32)usb_ep0_state) {
        case 0: break;
        case 1: goto in_phase;
        case 2: goto out_phase;
        case 3: usb_xfer_type = 0; usb_ep0_state = 0; return;
        default: usb_ep0_state = 0; return;
        }
    }
    if (!USBC_Dev_IsReadDataReady(USBC_EP_TYPE_EP0)) {
        debug_printf_stub(STR(0x4008D738));
        return;
    }
    len = USBC_ReadLenFromFifo(USBC_EP_TYPE_EP0);
    debug_printf_stub(STR(0x4008F9DC), len);
    if (len != 8) {
        timeout = 16;
        while (timeout-- && len != 8)
            len = USBC_ReadLenFromFifo(USBC_EP_TYPE_EP0);
        debug_printf_stub(STR(0x4008EF28), len);
        if (len != 8) {
            debug_printf_stub(STR(0x4008D760), len);
            USBC_Dev_ReadDataStatus(USBC_EP_TYPE_EP0, 0);
            USBC_Dev_EpSendStall(USBC_EP_TYPE_EP0);
            return;
        }
    }
    usb_device_read_data_ep_pack(0, d, 8);
    usb_device_read_data_status_ep0(0);
    usb_ep0_state = (d[0] & 0x80) ? 1 : 2;
    usb_setup_handle(d, 8);
    return;

in_phase:
    if (usb_tx_pos == 0) {
        debug_printf_stub(STR(0x4008E82C));
        usb_ep0_state = 0;
        return;
    }
    debug_printf_stub(STR(0x4008E1D4));
    debug_printf_stub(STR(0x4008EF3C), usb_tx_pos);
    debug_printf_stub(STR(0x4008E844), usb_tx_buf_len);
    if (usb_tx_buf_len - usb_tx_pos > 64) {
        USBC_WritePacket(USBC_SelectFIFO(0), ep_max_len[0], usb_tx_data + usb_tx_pos);
        USBC_Dev_WriteDataStatus(USBC_EP_TYPE_EP0, 0);
        usb_tx_pos += 64;
    } else {
        debug_printf_stub(STR(0x4008FF90));
        USBC_WritePacket(USBC_SelectFIFO(0), usb_tx_buf_len - usb_tx_pos, usb_tx_data + usb_tx_pos);
        USBC_Dev_WriteDataStatus(USBC_EP_TYPE_EP0, 1);
        usb_ep0_state = 0;
        usb_tx_pos = 0;
    }
    return;

out_phase:
    if (!USBC_Dev_IsReadDataReady(USBC_EP_TYPE_EP0)) {
        debug_printf_stub(STR(0x4008D088));
        return;
    }
    len = USBC_ReadLenFromFifo(USBC_EP_TYPE_EP0);
    debug_printf_stub(STR(0x4008E85C), len);
    usb_device_read_data_ep_pack(0, d, len);
    usb_device_read_data_status_ep0(0);
    usb_ep0_state = 0;
    debug_printf_stub(STR(0x4008C940), d[0], d[1], d[2], d[3], d[4], d[5], d[6], d[7]);
}
""")
f(0x4006B0E0, 156, "usb_handle_epn_in", D, "TX endpoint interrupt: clear a stall; when the FIFO is free and the class is mass storage (4), run its IN callback (the HID callback of EasyUSB is gone).", """
void usb_mass_in_ep_callback(void);

void usb_handle_epn_in(int ep)
{
    u32 old;
    debug_printf_stub(STR(0x4008EF50), ep);
    old = USBC_GetActiveEp();
    USBC_SelectActiveEp((unsigned char)ep);
    if (USBC_Dev_IsEpStall(USBC_EP_TYPE_TX)) {
        debug_printf_stub(STR(0x4008E1F0), ep);
        USBC_Dev_EpClearStall(USBC_EP_TYPE_TX);
    }
    if (!USBC_Dev_IsWriteDataReady(USBC_EP_TYPE_TX)) {
        debug_printf_stub(STR(0x4008E874), ep);
        if (current_usb_type == 4)
            usb_mass_in_ep_callback();
    }
    USBC_SelectActiveEp((unsigned char)old);
}
""")
f(0x4006B17C, 276, "usb_handle_epn_out", D, "RX endpoint interrupt: read the packet into usb_rx_buf and pass it to the class (1 HID, 2 CDC, 3 none, 4 mass storage).", """
void usb_hid_out_ep_callback(unsigned char *buf, int len);
void usb_cdc_out_ep_callback(unsigned char *buf, int len);
void usb_mass_out_ep_callback(unsigned char *buf, int len);
int usb_device_read_data_ep_pack(int ep, unsigned char *databuf, int len);

void usb_handle_epn_out(int ep)
{
    u32 old, len;
    debug_printf_stub(STR(0x4008EF64), ep);
    old = USBC_GetActiveEp();
    USBC_SelectActiveEp((unsigned char)ep);
    if (USBC_Dev_IsEpStall(USBC_EP_TYPE_RX)) {
        debug_printf_stub(STR(0x4008E20C), ep);
        USBC_Dev_EpClearStall(USBC_EP_TYPE_RX);
    }
    if (USBC_Dev_IsReadDataReady(USBC_EP_TYPE_RX)) {
        len = USBC_ReadLenFromFifo(USBC_EP_TYPE_RX);
        debug_printf_stub(STR(0x4008DBE0), ep, len);
        usb_device_read_data_ep_pack(ep, usb_rx_buf, len);
        usb_rx_buf_len = len;
        switch (current_usb_type) {
        case 1: usb_hid_out_ep_callback(usb_rx_buf, len); break;
        case 2: usb_cdc_out_ep_callback(usb_rx_buf, len); break;
        case 3: break;
        case 4: usb_mass_out_ep_callback(usb_rx_buf, len); break;
        }
        USBC_Dev_ReadDataStatus(USBC_EP_TYPE_RX, 1);
    }
    USBC_SelectActiveEp((unsigned char)old);
}
""")
f(0x4006B290, 552, "usb_irq_handle", D, "USB OTG interrupt (IRQ 103): reset, resume, suspend, disconnect; EP0 (the first EP0 interrupt picks HS or FS packet sizes); RX then TX endpoints 1..3.", """
u32 filtrate_irq_misc(u32 irq_misc);
void clear_all_irq(void);
void usb_handle_ep0(void);
void usb_handle_epn_in(int ep);
void usb_handle_epn_out(int ep);

void usb_irq_handle(void)
{
    u32 old = USBC_GetActiveEp(), misc = USBC_INT_MiscPending();
    u32 tx = USBC_INT_EpPending(USBC_EP_TYPE_TX), rx = USBC_INT_EpPending(USBC_EP_TYPE_RX);
    int i;
    misc = filtrate_irq_misc(misc);
    debug_printf_stub(STR(0x4008D984));
    debug_printf_stub(STR(0x4008CE64), misc, tx, rx, 0);
    if (misc & 4) {
        debug_printf_stub(STR(0x4008FF9C));
        USBC_INT_ClearMiscPending(4);
        clear_all_irq();
        fifo_offset = 0;
        usb_connect = 1;
        usb_ep0_state = 0;
        USBC_SelectActiveEp(0);
        USBC_Dev_SetAddress_default();
        return;
    }
    if (misc & 2) {
        debug_printf_stub(STR(0x4008F9EC));
        USBC_INT_ClearMiscPending(2);
        usb_connect = 1;
    }
    if (misc & 1) {
        debug_printf_stub(STR(0x4008F9FC));
        USBC_INT_ClearMiscPending(1);
        usb_connect = 0;
        usb_ep0_state = 0;
    }
    if (misc & 0x20) {
        debug_printf_stub(STR(0x4008EF78));
        USBC_INT_ClearMiscPending(0x20);
        usb_ep0_state = 0;
        usb_connect = 0;
    }
    if (tx & 1) {
        debug_printf_stub(STR(0x4008FA0C));
        USBC_INT_ClearEpPending(USBC_EP_TYPE_TX, 0);
        if (current_speed == 0) {
            if (USBC_Dev_QueryTransferMode() == 3) {
                current_speed = 3;
                ep_max_len[1] = 512;
                ep_max_len[2] = 512;
                debug_printf_stub(STR(0x4008D788));
                debug_printf_stub(STR(0x4008E88C));
            } else {
                current_speed = 2;
                ep_max_len[1] = 64;
                ep_max_len[2] = 64;
                debug_printf_stub(STR(0x4008D788));
                debug_printf_stub(STR(0x4008E8A4));
            }
            debug_printf_stub(STR(0x4008D788));
        }
        usb_handle_ep0();
    }
    for (i = 1; i < 4; i++)
        if ((rx >> i) & 1) {
            USBC_INT_ClearEpPending(USBC_EP_TYPE_RX, (unsigned char)i);
            usb_handle_epn_out(i);
        }
    for (i = 1; i < 4; i++)
        if ((tx >> i) & 1) {
            USBC_INT_ClearEpPending(USBC_EP_TYPE_TX, (unsigned char)i);
            usb_handle_epn_in(i);
        }
    USBC_SelectActiveEp((unsigned char)old);
}
""")
for a, n, t, s in ((0x4006B4B8, "usb_config_ep_in", 2, 0x4008E228), (0x4006B550, "usb_config_ep_out", 3, 0x4008E244)):
    f(a, 152, n, D, "Endpoint epidx != 0: configure it (double FIFO), give it 2*maxpack bytes of FIFO at fifo_offset, enable its interrupt, set ep_max_len (FNIRSI dropped the 4 KiB bound check).", f"""
void {n}(int epidx, int maxpack, int type)
{{
    u32 old;
    if (!epidx) return;
    debug_printf_stub(STR(0x{s:08X}), epidx, maxpack, type);
    old = USBC_GetActiveEp();
    USBC_SelectActiveEp((unsigned char)epidx);
    USBC_Dev_ConfigEp(type, {t}, 1, maxpack);
    USBC_ConfigFifo({t}, 1, maxpack * 2, fifo_offset);
    fifo_offset += maxpack * 2;
    USBC_INT_EnableEp({t}, (unsigned char)epidx);
    USBC_SelectActiveEp((unsigned char)old);
    ep_max_len[epidx] = maxpack;
}}
""")
f(0x4006B5E8, 60, "usb_device_clear_ep_halt", D, "Clear HALT on the TX (bit 7 set) or RX endpoint.", """
void usb_device_clear_ep_halt(int epaddr)
{
    u32 old = USBC_GetActiveEp();
    USBC_SelectActiveEp(epaddr & 0x7F);
    USBC_Clear_Feature_HALT((epaddr & 0x80) ? USBC_EP_TYPE_TX : USBC_EP_TYPE_RX);
    USBC_SelectActiveEp((unsigned char)old);
}
""")
f(0x4006B624, 108, "sunxi_udc_enable", D, "Speed unknown, bulk/HS transfer mode, misc interrupt enables (FNIRSI passes the bit numbers 0, 1, 2, 5 as masks), EP0 interrupt, soft connect.", """
void cfg_udc_command(u32 cmd);

void sunxi_udc_enable(void)
{
    debug_printf_stub(STR(0x4008E260));
    current_speed = 0;
    debug_printf_stub(STR(0x4008D2FC));
    USBC_Dev_ConfigTransferMode(4, 3);
    USBC_INT_EnableUsbMiscUint(0);
    USBC_INT_EnableUsbMiscUint(1);
    USBC_INT_EnableUsbMiscUint(2);
    USBC_INT_EnableUsbMiscUint(5);
    USBC_INT_EnableEp(USBC_EP_TYPE_TX, 0);
    cfg_udc_command(1);
}
""")
f(0x4006B6A4, 128, "usb_device_init", D, "Start the device as class type (1 HID, 2 CDC, 4 mass storage): BSP init, disable, IRQ 103, enable. -1 if it already runs as that type.", """
int usb_dev_bsp_init(void);
void sunxi_udc_disable(void);
void sunxi_udc_enable(void);
void usb_irq_handle(void);

int usb_device_init(unsigned char type)
{
    dbg_printf(STR(0x4008FA1C));
    if (current_usb_type == type) {
        dbg_printf(STR(0x4008EFA0));
        return -1;
    }
    current_usb_type = type;
    if (usb_dev_bsp_init() != 0) {
        debug_printf_stub(STR(0x4008DC00));
        return -1;
    }
    sunxi_udc_disable();
    irq_register(0x67, usb_irq_handle, 0, 1);
    irq_enable();
    sunxi_udc_enable();
    dbg_printf(STR(0x4008EF8C));
    return 0;
}
""")
f(0x4006B728, 76, "usb_device_exit", D, "Disable, close the clocks, reset the stack state.", """
void sunxi_udc_disable(void);

void usb_device_exit(void)
{
    dbg_printf(STR(0x4008EFB4));
    sunxi_udc_disable();
    usb_phy_close_clock_Device();
    current_usb_type = 0;
    current_speed = 0;
    usb_connect = 0;
    usb_ep0_state = 0;
    usb_addr = 0;
    usb_xfer_type = 0;
}
""")
