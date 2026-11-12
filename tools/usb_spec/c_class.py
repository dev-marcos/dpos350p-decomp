# Standard/class requests, descriptors, CDC/HID echo, mass storage (BOT + SCSI).  exec'd by gen_usb.py
C = "usb_cdc.c/usb_hid.c"
M = "FNIRSI mass storage (no EasyUSB source)"

GLOB = """
#define setup_req      ((unsigned char *)0x42E68C58u)   /* copy of the last setup packet */
#define line_coding    ((unsigned char *)0x40095484u)   /* 7 bytes */
#define line_coding_rx (*(u32 *)0x42E68CECu)
void usb_mass_send_data(unsigned char *buf, int len);
void usb_mass_send_csw(u32 status, u32 next);
void usb_mass_set_sense(u32 lun, u32 key, u32 asc);
"""
MASS = """
#define cbw            ((unsigned char *)0x42E68B90u)   /* 31-byte command block wrapper */
#define CBW_SIG        (*(u32 *)0x42E68B90u)
#define CBW_DLEN       (*(u32 *)0x42E68B98u)
#define CBW_FLAGS      cbw[0x0C]
#define CBW_LUN        cbw[0x0D]
#define CBW_CBLEN      cbw[0x0E]
#define CBW_OP         cbw[0x0F]
#define csw            ((unsigned char *)0x42E68C10u)   /* 13-byte command status wrapper */
#define CSW_SIG        (*(u32 *)0x42E68C10u)
#define CSW_TAG        (*(u32 *)0x42E68C14u)
#define CSW_RESIDUE    (*(u32 *)0x42E68C18u)
#define mass_rx_ptr    (*(unsigned char **)0x42E68CE0u)
#define mass_lba       (*(u32 *)0x42E68CE4u)
#define mass_count     (*(u32 *)0x42E68CE8u)
#define lun_blksize    ((u32 *)0x42E68D0Cu)
#define lun_blocks     ((u32 *)0x42E68D10u)
#define mass_rx_len    (*(unsigned short *)0x42E68D16u)
#define mass_flags     (*(unsigned char *)0x42E68D1Cu)
#define mass_stage     (*(unsigned char *)0x42E68D1Du)  /* 0 CBW, 1 data out, 2 data in, 3 sent, 4/5 CSW */
#define max_lun_rsp    (*(unsigned char *)0x42E68D1Eu)
#define max_lun        (*(volatile u32 *)0x42E68D23u)   /* read as an unaligned word by the CBW check */
#define sense_data     ((unsigned char *)0x40095400u)   /* 18-byte fixed-format sense */
void usb_mass_send_data(unsigned char *buf, int len);
void usb_mass_send_csw(u32 status, u32 next);
void usb_mass_set_sense(u32 lun, u32 key, u32 asc);
int usb_mass_write(int unused, unsigned char *buf, int len);
int scsi_lun_invalid(u32 lun);
int scsi_check_range(u32 lun, u32 op, u32 lba, u32 count);
void usb_mass_disk_read(u32 lun, u32 lba, u32 count);
void usb_mass_disk_write(u32 lun, u32 lba, u32 count);
"""


def g(addr, size, name, src, desc, body, extra=GLOB):
    f(addr, size, name, src, desc, extra.strip("\n") + "\n\n" + body.strip("\n"))


# ------------------------------------------------------------------ descriptors and requests
g(0x4006CFB0, 52, "usb_cdc_store_line_coding", C, "Copy 7 bytes of SET_LINE_CODING data to line_coding; clear the 'waiting for data' flag.", """
void usb_cdc_store_line_coding(unsigned char *buf)
{
    line_coding_rx = 0;
    *(u32 *)line_coding = *(u32 *)buf;
    line_coding[4] = buf[4];
    line_coding[5] = buf[5];
    line_coding[6] = buf[6];
}
""")
g(0x4006CFE4, 104, "usb_get_device_descriptor", C,
  "18-byte device descriptor of the current class (3 UVC 0x4009543C, 1 HID 0x40095428, 2 CDC 0x40095414, 4 mass storage 0x40095450). Another type leaves the pointer register unset (OPEN: not tested).", """
void usb_get_device_descriptor(unsigned char *req)
{
    u32 len = *(unsigned short *)(req + 6);
    unsigned char *d = 0;
    switch (current_usb_type) {
    case 3: d = (unsigned char *)0x4009543Cu; break;
    case 1: d = (unsigned char *)0x40095428u; break;
    case 2: d = (unsigned char *)0x40095414u; break;
    case 4: d = (unsigned char *)0x40095450u; break;
    }
    debug_printf_stub(STR(0x4008E148));
    usb_device_write_data_ep_pack(0, d, len < 0x12 ? len : 0x12);
}
""", GLOB + "int usb_device_write_data_ep_pack(int ep, unsigned char *databuf, int len);\n")
g(0x4006D04C, 44, "usb_get_cfg_descriptor", C, "Configuration descriptor of the current class.", """
void usb_hid_get_cfg_descriptor(unsigned char *req);
void usb_cdc_get_cfg_descriptor(unsigned char *req);
int usb_uvc_get_cfg_descriptor(unsigned char *req);
void usb_mass_get_cfg_descriptor(unsigned char *req);

void usb_get_cfg_descriptor(unsigned char *req)
{
    switch (current_usb_type) {
    case 1: usb_hid_get_cfg_descriptor(req); break;
    case 2: usb_cdc_get_cfg_descriptor(req); break;
    case 3: usb_uvc_get_cfg_descriptor(req); break;
    case 4: usb_mass_get_cfg_descriptor(req); break;
    }
}
""")
g(0x4006D078, 312, "udp_get_dev_descriptor_string", C,
  "String descriptors 0 (language, max 4 bytes), 1/3, 2 (header rewritten with the length), 4, 5, 6, 0xEE (MS OS string, header rewritten in the 1/3 buffer); max 0x20 or 0x26 bytes; others send nothing.", """
int usb_device_write_data_ep_pack(int ep, unsigned char *databuf, int len);

void udp_get_dev_descriptor_string(unsigned char *req)
{
    u32 len = *(unsigned short *)(req + 6), idx = req[2];
    unsigned char *d;
    debug_printf_stub(STR(0x4008D960), idx);
    switch (idx) {
    case 0:
        usb_device_write_data_ep_pack(0, (unsigned char *)0x400954A0u, len < 5 ? len : 4);
        return;
    case 2:
        if ((int)len >= 0x26) len = 0x26;
        *(unsigned short *)0x40095340u = (unsigned short)(len | 0x300);
        d = (unsigned char *)0x40095340u;
        break;
    case 1:
    case 3: if (len >= 0x20) len = 0x20; d = (unsigned char *)0x40095360u; break;
    case 4: if (len >= 0x20) len = 0x20; d = (unsigned char *)0x40095380u; break;
    case 5: if (len >= 0x20) len = 0x20; d = (unsigned char *)0x400953A0u; break;
    case 6: if (len >= 0x20) len = 0x20; d = (unsigned char *)0x400953C0u; break;
    case 0xEE:
        if (len >= 0x20) len = 0x20;
        *(unsigned short *)0x40095360u = (unsigned short)(len | 0x300);
        d = (unsigned char *)0x40095360u;
        break;
    default:
        usb_device_write_data_ep_pack(0, 0, 0);
        return;
    }
    usb_device_write_data_ep_pack(0, d, len);
}
""")
g(0x4006D1B0, 240, "usb_get_descriptor", C, "GET_DESCRIPTOR by type (wValue high byte): 1 device, 2 configuration, 3 string, 0x22 HID report; others are acknowledged with a message.", """
void usb_get_device_descriptor(unsigned char *req);
void usb_get_cfg_descriptor(unsigned char *req);
void udp_get_dev_descriptor_string(unsigned char *req);
void usb_hid_get_report_descriptor(void);
void usb_device_read_data_status_ep0(u32 is_complete);

void usb_get_descriptor(unsigned char *req)
{
    u32 type = *(unsigned short *)(req + 2) >> 8;
    switch (type) {
    case 1: usb_get_device_descriptor(req); return;
    case 2: usb_get_cfg_descriptor(req); return;
    case 3: udp_get_dev_descriptor_string(req); return;
    case 0x22: usb_hid_get_report_descriptor(); return;
    }
    debug_printf_stub(STR(0x4008D434), type);
    usb_device_read_data_status_ep0(1);
}
""")
g(0x4006D2A0, 56, "usb_set_address", C, "SET_ADDRESS: wait 1 ms, program FADDR, acknowledge, back to EP0 idle.", """
void delay_ms(u32 ms);
void usb_device_set_address(unsigned char addr);
void usb_device_read_data_status_ep0(u32 is_complete);
void usb_device_set_ep0_state(int state);

void usb_set_address(u32 addr)
{
    delay_ms(1);
    usb_device_set_address((unsigned char)addr);
    usb_device_read_data_status_ep0(1);
    debug_printf_stub(STR(0x4008E7CC), addr);
    usb_device_set_ep0_state(0);
}
""")
SETCFG = """
void usb_config_ep_in(int epidx, int maxpack, int type);
void usb_config_ep_out(int epidx, int maxpack, int type);
void usb_device_read_data_status_ep0(u32 is_complete);
void usb_device_clear_setup_end(void);
void usb_device_send_nullpack_ep0(void);
void usb_device_set_ep0_state(int state);
"""
SETCFG_BODY = """
    u32 t = *type;
    u32 ts = t == 4 ? 4 : (t == 1 || t == 2) ? 3 : t == 3 ? 2 : 0;
    if (ts) {
        usb_config_ep_out(1, 512, ts);
        usb_config_ep_in(1, 512, ts);
    }
    usb_device_read_data_status_ep0(1);
    usb_device_clear_setup_end();
    usb_device_send_nullpack_ep0();
    usb_device_set_ep0_state(0);
"""
g(0x4006D2D8, 4, "usb_set_configuration", C,
  "SET_CONFIGURATION: loads &current_usb_type and falls into 0x4006D2DC. Endpoint 1 OUT and IN, 512 bytes, type 4 (mass storage: bulk), 3 (HID/CDC: interrupt) or 2 (UVC: iso).", "void usb_set_configuration(void)\n{\n    unsigned char *type = &current_usb_type;" + SETCFG_BODY + "}\n", GLOB + SETCFG)
g(0x4006D2DC, 156, "usb_set_configuration_body", C, "Body of usb_set_configuration with the class-type pointer in r0 (second entry, name OPEN).",
  "void usb_set_configuration_body(unsigned char *type)\n{" + SETCFG_BODY + "}\n", GLOB + SETCFG)
g(0x4006D378, 312, "usb_standard_request", C, "Standard requests: GET_DESCRIPTOR, SET_CONFIGURATION, SET_ADDRESS, CLEAR_FEATURE (endpoint halt); the others are only reported. Ends with an EP0 status acknowledge.", """
void usb_get_descriptor(unsigned char *req);
void usb_set_configuration(void);
void usb_set_address(u32 addr);
void usb_device_clear_ep_halt(int epaddr);
void usb_device_read_data_status_ep0(u32 is_complete);

void usb_standard_request(unsigned char *req)
{
    u32 r = req[1];
    if (r == 6) { debug_printf_stub(STR(0x4008F92C)); usb_get_descriptor(req); return; }
    if (r == 9) { debug_printf_stub(STR(0x4008EE60)); usb_set_configuration(); return; }
    if (r == 8) { debug_printf_stub(STR(0x4008EE74)); debug_printf_stub(STR(0x4008D020)); }
    else if (r == 0x0B) { debug_printf_stub(STR(0x4008F93C)); debug_printf_stub(STR(0x4008D23C)); }
    else if (r == 0x0A) { debug_printf_stub(STR(0x4008D26C)); }
    else if (r == 5) { debug_printf_stub(STR(0x4008FF78)); usb_set_address(req[2] & 0x7F); return; }
    else if (r == 7) { debug_printf_stub(STR(0x4008F94C)); debug_printf_stub(STR(0x4008D054)); }
    else if (r == 0x0C) { debug_printf_stub(STR(0x4008FF84)); debug_printf_stub(STR(0x4008D29C)); }
    else if (r == 1) {
        u32 idx = *(unsigned short *)(req + 4);
        debug_printf_stub(STR(0x4008F95C));
        usb_device_clear_ep_halt(idx);
    } else {
        debug_printf_stub(STR(0x4008F96C));
        debug_printf_stub(STR(0x4008D460), r);
    }
    usb_device_read_data_status_ep0(1);
}
""")
g(0x4006D4B0, 312, "usb_class_request", C,
  "Class requests: HID SET_REPORT/SET_IDLE (message only), CDC SET_LINE_CODING (waits 200 us for the 7-byte data stage), GET_LINE_CODING, SET_CONTROL_LINE_STATE, mass storage GET_MAX_LUN (0xFE).", """
void delay_us(u32 us);
int usb_device_read_data_ep_pack(int ep, unsigned char *databuf, int len);
int usb_device_write_data_ep_pack(int ep, unsigned char *databuf, int len);
void usb_device_read_data_status_ep0(u32 is_complete);
void usb_device_set_ep0_state(int state);
void usb_device_clear_setup_end(void);
void usb_cdc_store_line_coding(unsigned char *buf);
unsigned char *usb_mass_max_lun_ptr(void);

void usb_class_request(unsigned char *req)
{
    unsigned char buf[8];
    u32 r = req[1], len;
    if (r == 9) { debug_printf_stub(STR(0x4008CF10)); return; }
    if (r == 0x0A) { debug_printf_stub(STR(0x4008CF48)); return; }
    if (r == 0x20) {
        debug_printf_stub(STR(0x4008EE88));
        line_coding_rx = 1;
        delay_us(200);
        len = USBC_ReadLenFromFifo(USBC_EP_TYPE_EP0);
        debug_printf_stub(STR(0x4008E164), len);
        if (len != 7) return;
        usb_device_read_data_ep_pack(0, buf, 7);
        usb_cdc_store_line_coding(buf);
        usb_device_read_data_status_ep0(1);
        usb_device_set_ep0_state(0);
        line_coding_rx = 0;
        return;
    }
    if (r == 0x21) {
        usb_device_write_data_ep_pack(0, line_coding, 7);
        debug_printf_stub(STR(0x4008EE9C));
        return;
    }
    if (r == 0x22) {
        debug_printf_stub(STR(0x4008E180));
        usb_device_clear_setup_end();
        usb_device_set_ep0_state(0);
        return;
    }
    if (r == 0xFE) {
        max_lun_rsp = *usb_mass_max_lun_ptr();
        debug_printf_stub(STR(0x4008E7E4), max_lun_rsp);
        usb_device_write_data_ep_pack(0, &max_lun_rsp, *(unsigned short *)(req + 6));
        return;
    }
    debug_printf_stub(STR(0x4008CE28), r);
}
""", GLOB + "#define max_lun_rsp (*(unsigned char *)0x42E68D1Eu)\n")
g(0x4006D5E8, 184, "usb_setup_handle", C, "Copy and print the 8-byte setup packet, then dispatch standard (type 0) or class (type 1) requests; vendor requests are ignored.", """
void usb_standard_request(unsigned char *req);
void usb_class_request(unsigned char *req);

void usb_setup_handle(unsigned char *req, int len)
{
    volatile int Timeout = 10000000;
    (void)Timeout;
    if (len != 8) {
        debug_printf_stub(STR(0x4008EEC4));
        return;
    }
    memcpy(setup_req, req, 8);
    debug_printf_stub(STR(0x4008E7FC), setup_req[0]);
    debug_printf_stub(STR(0x4008EEB0), setup_req[1]);
    debug_printf_stub(STR(0x4008F97C), *(unsigned short *)(setup_req + 2));
    debug_printf_stub(STR(0x4008F98C), *(unsigned short *)(setup_req + 4));
    debug_printf_stub(STR(0x4008F99C), *(unsigned short *)(setup_req + 6));
    if (!(setup_req[0] & 0x60))
        usb_standard_request(setup_req);
    else if ((setup_req[0] & 0x60) == 0x20)
        usb_class_request(setup_req);
}
""")
g(0x4006D764, 16, "usb_hid_get_report_descriptor", C, "35-byte HID report descriptor at 0x4008D9A8.", """
int usb_device_write_data_ep_pack(int ep, unsigned char *databuf, int len);

void usb_hid_get_report_descriptor(void) { usb_device_write_data_ep_pack(0, (unsigned char *)0x4008D9A8u, 0x23); }
""")
for a, n, full, d, s in ((0x4006D774, "usb_hid_get_cfg_descriptor", 0x29, 0x4008D48C, "HID (41 bytes)"),
                         (0x4006D7DC, "usb_cdc_get_cfg_descriptor", 0x4B, 0x4008C79C, "CDC (75 bytes; prints the HID message)"),
                         (0x4006B980, "usb_mass_get_cfg_descriptor", 0x20, 0x400953E0, "mass storage (32 bytes)")):
    g(a, 76, n, C if a != 0x4006B980 else M, f"Configuration descriptor, {s}: 8 or 9 bytes when asked for exactly that, else the whole descriptor.", f"""
int usb_device_write_data_ep_pack(int ep, unsigned char *databuf, int len);

void {n}(unsigned char *req)
{{
    u32 len = *(unsigned short *)(req + 6), n;
    debug_printf_stub(STR(0x{0x4008DC20 if a == 0x4006B980 else 0x4008E12C:08X}));
    n = len == 8 ? 8 : len == 9 ? 9 : 0x{full:X};
    usb_device_write_data_ep_pack(0, (unsigned char *)0x{d:08X}u, n);
}}
""")
g(0x4006D7CC, 16, "usb_hid_out_ep_callback", C, "HID OUT report: echo it on EP1 IN.", """
int usb_device_write_data(int ep, unsigned char *databuf, int len);

int usb_hid_out_ep_callback(unsigned char *buf, int len) { return usb_device_write_data(1, buf, len); }
""")
g(0x4006D830, 84, "usb_cdc_echo", C, "CDC: echo the data on EP1 IN; a multiple of 512 bytes is followed by a zero-length packet.", """
int usb_device_write_data(int ep, unsigned char *databuf, int len);

int usb_cdc_echo(unsigned char *buf, int len)
{
    int r = usb_device_write_data(1, buf, len);
    if (r == 0 && (len & 0x1FF) == 0)
        return usb_device_write_data(1, buf, 0);
    return r;
}
""")
g(0x4006D884, 4, "usb_cdc_out_ep_callback", C, "b usb_cdc_echo.", """
int usb_cdc_echo(unsigned char *buf, int len);

int usb_cdc_out_ep_callback(unsigned char *buf, int len) { return usb_cdc_echo(buf, len); }
""")
g(0x4006DDC0, 36, "usb_uvc_get_cfg_descriptor", C, "UVC configuration descriptor (0x400950B4), wLength bytes.", """
int usb_device_write_data_ep_pack(int ep, unsigned char *databuf, int len);

int usb_uvc_get_cfg_descriptor(unsigned char *req)
{
    debug_printf_stub(STR(0x4008E2B4));
    return usb_device_write_data_ep_pack(0, (unsigned char *)0x400950B4u, *(unsigned short *)(req + 6));
}
""")

# ------------------------------------------------------------------ mass storage: bulk-only transport
g(0x4006B95C, 28, "usb_mass_write", M, "usb_device_write_data(1, buf, len) ignoring its first argument; returns 0.", """
int usb_device_write_data(int ep, unsigned char *databuf, int len);

int usb_mass_write(int unused, unsigned char *buf, int len)
{
    (void)unused;
    usb_device_write_data(1, buf, len);
    return 0;
}
""", MASS)
g(0x4006B978, 8, "usb_mass_max_lun_ptr", M, "Address of the max-LUN byte (0x42E68D23).",
  "unsigned char *usb_mass_max_lun_ptr(void) { return (unsigned char *)0x42E68D23u; }", MASS)
g(0x4006B9D8, 24, "usb_mass_in_ep_callback", M, "EP1 IN done: run the IN state machine; returns 0.", """
void usb_mass_in_handler(void);

int usb_mass_in_ep_callback(void)
{
    usb_mass_in_handler();
    return 0;
}
""", MASS)
g(0x4006B9F0, 4, "usb_mass_out_ep_callback", M, "b usb_mass_out_handler.", """
void usb_mass_out_handler(unsigned char *buf, u32 len);

void usb_mass_out_ep_callback(unsigned char *buf, u32 len) { usb_mass_out_handler(buf, len); }
""", MASS)
g(0x4006B9F4, 164, "usb_mass_in_handler", M, "IN state machine: stage 2 continues a READ(10), stage 3 sends the CSW, stages 4/5 return to CBW.", """
void scsi_read10(u32 lun, u32 lba, u32 count);

void usb_mass_in_handler(void)
{
    u32 fl = mass_flags | 0x10;
    mass_flags = (unsigned char)fl;
    switch (mass_stage) {
    case 2:
        if (CBW_OP != 0x28) return;
        mass_flags = (unsigned char)(fl | 2);
        scsi_read10(CBW_LUN, mass_lba, mass_count);
        return;
    case 3:
        usb_mass_send_csw(0, 1);
        return;
    case 4:
    case 5:
        mass_stage = 0;
        return;
    }
}
""", MASS)
g(0x4006BA98, 1460, "usb_mass_out_handler", M,
  "OUT packet: in stage 0 parse a CBW (the CBW decoder at 0x4006BB6C is a separate stock function that the map merges into this one), in stage 1 continue a WRITE(10); otherwise phase error.", """
void scsi_test_unit_ready(u32 lun);
void scsi_request_sense(u32 lun);
void scsi_format_unit(u32 lun);
void scsi_invalid_cmd(u32 lun);
void scsi_inquiry(u32 lun);
void scsi_mode_sense6(u32 lun);
void scsi_start_stop(u32 lun);
void scsi_read_format_capacities(u32 lun);
void scsi_read_capacity10(u32 lun);
void scsi_read10(u32 lun, u32 lba, u32 count);
void scsi_write10(u32 lun, u32 lba, u32 count);
void scsi_verify10(u32 lun);
void scsi_mode_sense10(u32 lun);

/* stock 0x4006BB6C: decode a CBW (only reached by a tail branch from usb_mass_out_handler) */
static void usb_mass_decode_cbw(void)
{
    u32 len = mass_rx_len, i, asc;
    if (len) {
        unsigned char *s = mass_rx_ptr, *d = cbw;
        for (i = len; i; i--)
            *d++ = *s++;
    }
    CSW_TAG = *(u32 *)(cbw + 4);
    CSW_RESIDUE = CBW_DLEN;
    if (len != 31) {
        debug_printf_stub(STR(0x4008F8DC));
        CBW_SIG = 0;
        asc = 0x1A;
        goto fail;
    }
    if (CBW_OP == 0x28 || CBW_OP == 0x2A) {
        mass_lba = (u32)cbw[0x11] << 24 | (u32)cbw[0x12] << 16 | (u32)cbw[0x13] << 8 | cbw[0x14];
        mass_count = (u32)cbw[0x16] << 8 | cbw[0x17];
    }
    if (CBW_SIG != 0x43425355u) {
        debug_printf_stub(STR(0x4008F91C));
        asc = 0x20;
        goto fail;
    }
    if (max_lun < CBW_LUN || CBW_CBLEN == 0 || (int)CBW_CBLEN >= 0x11) {
        debug_printf_stub(STR(0x4008F8EC));
        asc = 0x24;
        goto fail;
    }
    debug_printf_stub(STR(0x4008F8FC), CBW_OP);
    switch (CBW_OP) {
    case 0x00: scsi_test_unit_ready(CBW_LUN); return;
    case 0x03: scsi_request_sense(CBW_LUN); return;
    case 0x04: scsi_format_unit(CBW_LUN); return;
    case 0x12: scsi_inquiry(CBW_LUN); return;
    case 0x1A: scsi_mode_sense6(CBW_LUN); return;
    case 0x1B:
    case 0x1E: scsi_start_stop(CBW_LUN); return;
    case 0x23: scsi_read_format_capacities(CBW_LUN); return;
    case 0x25: scsi_read_capacity10(CBW_LUN); return;
    case 0x28: scsi_read10(CBW_LUN, mass_lba, mass_count); return;
    case 0x2A: scsi_write10(CBW_LUN, mass_lba, mass_count); return;
    case 0x2F: scsi_verify10(CBW_LUN); return;
    case 0x5A: scsi_mode_sense10(CBW_LUN); return;
    case 0x08: case 0x0A: case 0x15: case 0x1D: case 0x55: case 0x88: case 0x8A: case 0x8F:
    case 0x9E: case 0xA8: case 0xAA: case 0xAF:
        scsi_invalid_cmd(CBW_LUN); return;
    }
    debug_printf_stub(STR(0x4008F90C));
    asc = 0x20;
fail:
    usb_mass_set_sense(CBW_LUN, 5, asc);
    usb_mass_send_csw(1, 0);
}

void usb_mass_out_handler(unsigned char *buf, u32 len)
{
    u32 fl = mass_flags | 0x10, lun;
    mass_flags = (unsigned char)fl;
    mass_rx_len = (unsigned short)len;
    mass_rx_ptr = buf;
    if (mass_stage == 0) {
        usb_mass_decode_cbw();
        return;
    }
    if (mass_stage == 1) {
        lun = CBW_LUN;
        if (CBW_OP == 0x2A) {
            mass_flags = (unsigned char)(fl | 1);
            scsi_write10(lun, mass_lba, mass_count);
            return;
        }
        usb_mass_set_sense(lun, 5, 0x24);
    } else {
        usb_mass_set_sense(CBW_LUN, 5, 0x24);
    }
    usb_mass_send_csw(2, 0);
}
""", MASS)
g(0x4006C04C, 64, "usb_mass_send_data", M, "Data-in phase: send on EP1, stage 3 (CSW next), status 0, residue -= len.", """
int usb_device_write_data(int ep, unsigned char *databuf, int len);

void usb_mass_send_data(unsigned char *buf, int len)
{
    usb_device_write_data(1, buf, len);
    mass_stage = 3;
    csw[12] = 0;
    CSW_RESIDUE -= len;
}
""", MASS)
g(0x4006C08C, 60, "usb_mass_send_csw", M, "Send the 13-byte CSW with this status; next stage 4 (next != 0) or 5.", """
void usb_mass_send_csw(u32 status, u32 next)
{
    csw[12] = (unsigned char)status;
    CSW_SIG = 0x53425355u;
    usb_mass_write(1, csw, 13);
    mass_stage = next ? 4 : 5;
}
""", MASS)
# ------------------------------------------------------------------ SCSI commands
g(0x4006D888, 16, "scsi_lun_invalid", M, "LUN > 1.", "int scsi_lun_invalid(u32 lun) { return lun > 1; }", MASS)
NOMEDIUM = """
    if (scsi_lun_invalid(lun)) {
        usb_mass_set_sense(CBW_LUN, 2, 0x3A);   /* NOT READY, MEDIUM NOT PRESENT */
        usb_mass_send_csw(1, 1);
        usb_phy_nop(0);
        return;
    }
"""
NOPDECL = "void usb_phy_nop(u32 x);\n"
g(0x4006D898, 52, "scsi_inquiry", M, "INQUIRY: EVPD -> 5 bytes at 0x42E68C40; else the standard data of LUN 0 or the other LUN, at most 36 bytes.", """
void scsi_inquiry(u32 lun)
{
    u32 n;
    if (cbw[0x10] & 1) {
        usb_mass_send_data((unsigned char *)0x42E68C40u, 5);
        return;
    }
    n = cbw[0x13];
    if ((int)n >= 0x25) n = 0x24;
    usb_mass_send_data(lun == 0 ? (unsigned char *)0x400952F8u : (unsigned char *)0x4009531Cu, n);
}
""", MASS)
g(0x4006D8CC, 144, "scsi_read_format_capacities", M, "READ FORMAT CAPACITIES: 12 bytes at 0x40095464 (block count, block size, big endian).", """
void scsi_read_format_capacities(u32 lun)
{
    unsigned char *b = (unsigned char *)0x40095464u;
    u32 n, s;""" + NOMEDIUM + """    n = lun_blocks[lun];
    b[7] = (unsigned char)n; b[4] = (unsigned char)(n >> 24); b[5] = (unsigned char)(n >> 16); b[6] = (unsigned char)(n >> 8);
    s = lun_blksize[lun];
    b[9] = (unsigned char)(s >> 16); b[11] = (unsigned char)s; b[10] = (unsigned char)(s >> 8);
    usb_mass_send_data(b, 12);
}
""", MASS + NOPDECL)
g(0x4006D95C, 160, "scsi_read_capacity10", M, "READ CAPACITY(10): last LBA and block size, big endian, at 0x42E68C48.", """
void scsi_read_capacity10(u32 lun)
{
    unsigned char *b = (unsigned char *)0x42E68C48u;
    u32 n, s;""" + NOMEDIUM + """    n = lun_blocks[lun] - 1;
    b[0] = (unsigned char)(n >> 24); b[1] = (unsigned char)(n >> 16); b[2] = (unsigned char)(n >> 8); b[3] = (unsigned char)n;
    s = lun_blksize[lun];
    b[4] = (unsigned char)(s >> 24); b[5] = (unsigned char)(s >> 16); b[6] = (unsigned char)(s >> 8); b[7] = (unsigned char)s;
    usb_mass_send_data(b, 8);
}
""", MASS + NOPDECL)
g(0x4006D9FC, 12, "scsi_mode_sense6", M, "MODE SENSE(6): 4 bytes at 0x40095494.",
  "void scsi_mode_sense6(u32 lun) { (void)lun; usb_mass_send_data((unsigned char *)0x40095494u, 4); }", MASS)
g(0x4006DA08, 12, "scsi_mode_sense10", M, "MODE SENSE(10): 8 bytes at 0x4009547C.",
  "void scsi_mode_sense10(u32 lun) { (void)lun; usb_mass_send_data((unsigned char *)0x4009547Cu, 8); }", MASS)
g(0x4006DA14, 24, "scsi_request_sense", M, "REQUEST SENSE: at most 18 bytes of sense_data.", """
void scsi_request_sense(u32 lun)
{
    u32 n = cbw[0x13];
    (void)lun;
    if ((int)n >= 0x13) n = 0x12;
    usb_mass_send_data(sense_data, n);
}
""", MASS)
g(0x4006DA2C, 16, "usb_mass_set_sense", M, "sense_data[2] = key, [12] = ASC (the LUN is ignored).", """
void usb_mass_set_sense(u32 lun, u32 key, u32 asc)
{
    (void)lun;
    sense_data[2] = (unsigned char)key;
    sense_data[12] = (unsigned char)asc;
}
""", MASS)
g(0x4006DA3C, 12, "scsi_start_stop", M, "START STOP UNIT / PREVENT ALLOW MEDIUM REMOVAL: good status.",
  "void scsi_start_stop(u32 lun) { (void)lun; usb_mass_send_csw(0, 1); }", MASS)
g(0x4006DA48, 196, "scsi_read10", M, "READ(10): stage 0 validates (range, direction) and starts; stage 2 continues. Data comes from the disk layer (0x4006A964).", """
void scsi_read10(u32 lun, u32 lba, u32 count)
{
    if (mass_stage == 0) {
        if (!scsi_check_range(CBW_LUN, 0x28, lba, count)) {
            debug_printf_stub(STR(0x4008D9CC));
            return;
        }
        if (!(CBW_FLAGS & 0x80)) {
            debug_printf_stub(STR(0x4008E8BC));
            usb_mass_set_sense(CBW_LUN, 5, 0x24);
            usb_mass_send_csw(1, 1);
            return;
        }
        debug_printf_stub(STR(0x4008FFA8));
        mass_stage = 2;
    } else if (mass_stage == 2) {
        debug_printf_stub(STR(0x4008FFA8));
    } else {
        return;
    }
    usb_mass_disk_read(lun, lba, count);
}
""", MASS)
g(0x4006DB0C, 152, "scsi_write10", M, "WRITE(10): stage 0 validates and waits for data (stage 1); stage 1 passes data to the disk layer (0x4006A978).", """
void scsi_write10(u32 lun, u32 lba, u32 count)
{
    if (mass_stage == 0) {
        if (!scsi_check_range(CBW_LUN, 0x2A, lba, count))
            return;
        if (!(CBW_FLAGS & 0x80)) {
            mass_stage = 1;
            return;
        }
        usb_mass_set_sense(CBW_LUN, 5, 0x24);
        usb_mass_send_csw(1, 0);
        return;
    }
    if (mass_stage == 1)
        usb_mass_disk_write(lun, lba, count);
}
""", MASS)
g(0x4006DBA4, 76, "scsi_verify10", M, "VERIFY(10): good status unless data is expected or BYTCHK is set.", """
void scsi_verify10(u32 lun)
{
    (void)lun;
    if (CBW_DLEN == 0 && !(cbw[0x10] & 4)) {
        usb_mass_send_csw(0, 1);
        return;
    }
    usb_mass_set_sense(CBW_LUN, 5, 0x24);
    usb_mass_send_csw(1, 0);
}
""", MASS)
g(0x4006DBF0, 84, "scsi_test_unit_ready", M, "TEST UNIT READY: good status, or NOT READY for LUN > 1.", """
void scsi_test_unit_ready(u32 lun)
{""" + NOMEDIUM + """    usb_mass_send_csw(0, 1);
}
""", MASS + NOPDECL)
g(0x4006DC44, 76, "scsi_format_unit", M, "FORMAT UNIT: nothing for a valid LUN (no CSW), NOT READY otherwise.", """
void scsi_format_unit(u32 lun)
{""" + NOMEDIUM + """}
""", MASS + NOPDECL)
g(0x4006DC90, 84, "scsi_invalid_cmd", M, "Unsupported command: ILLEGAL REQUEST / INVALID COMMAND OPERATION CODE.", """
void scsi_invalid_cmd(u32 lun)
{
    (void)lun;
    debug_printf_stub(STR(0x4008EFC8));
    usb_mass_set_sense(CBW_LUN, 5, 0x20);
    usb_mass_send_csw(1, 0);
}
""", MASS)
g(0x4006DCE4, 156, "scsi_check_range", M, "lba + count must fit the LUN (else LBA OUT OF RANGE, ASC 0x21) and the CBW length must equal count * block size (else ASC 0x24). Returns 1 when valid.", """
int scsi_check_range(u32 lun, u32 op, u32 lba, u32 count)
{
    (void)op;
    if (lun_blocks[lun] < lba + count) {
        usb_mass_set_sense(lun, 5, 0x21);
        usb_mass_send_csw(1, 0);
        return 0;
    }
    if (CBW_DLEN != lun_blksize[lun] * count) {
        usb_mass_set_sense(CBW_LUN, 5, 0x24);
        usb_mass_send_csw(1, 0);
        return 0;
    }
    return 1;
}
""", MASS)
