# ISCR (0x01C19400) pull-up / force-ID / force-VBUS helpers (EasyUSB usb_phy.c).  exec'd by gen_usb.py
ISCR = "(USBC0_BASE + 0x400)"
f(0x4006C108, 4, "usb_phy_nop", P, "Empty function (bx lr); no EasyUSB counterpart identified (name OPEN).", "void usb_phy_nop(void) { }")
f(0x4006C10C, 8, "USBC_WakeUp_ClearChangeDetect", P, "Clear the write-1-to-clear VBUS/ID/DPDM change bits (6:4) from a value.",
  "u32 USBC_WakeUp_ClearChangeDetect(u32 reg_val) { return reg_val & ~0x70u; }")
for a, size, n, expr, desc in (
        (0x4006C114, 32, "USBC_EnableDpDmPullUp", "v | 0x10C00", "DP/DM pull-up (bit 16) and VBUS valid source 3 (bits 11:10)."),
        (0x4006C134, 32, "USBC_EnableIdPullUp", "v | 0x20000", "ID pull-up (bit 17)."),
        (0x4006C154, 36, "USBC_ForceIdToLow", "(v & ~0xC000u) | 0x8000", "Force ID = 10 (host)."),
        (0x4006C178, 32, "USBC_ForceIdToHigh", "v | 0xC000", "Force ID = 11 (device)."),
        (0x4006C198, 32, "USBC_ForceIdDisable", "v & ~0xC000u", "Force ID off."),
        (0x4006C1DC, 32, "USBC_ForceVbusValidDisable", "v & ~0x3000u", "Force VBUS valid off."),
        (0x4006C1FC, 36, "USBC_ForceVbusValidToLow", "(v & ~0x3000u) | 0x2000", "Force VBUS valid = 10."),
        (0x4006C220, 32, "USBC_ForceVbusValidToHigh", "v | 0x3000", "Force VBUS valid = 11.")):
    f(a, size, n, P, "ISCR: " + desc, f"""
u32 USBC_WakeUp_ClearChangeDetect(u32 reg_val);

void {n}(void)
{{
    u32 v = REG32({ISCR});
    REG32({ISCR}) = USBC_WakeUp_ClearChangeDetect({expr});
}}
""")
for a, n, lo, hi, dis in ((0x4006C1B8, "USBC_ForceId", "USBC_ForceIdToLow", "USBC_ForceIdToHigh", "USBC_ForceIdDisable"),
                          (0x4006C240, "USBC_ForceVbusValid", "USBC_ForceVbusValidToLow", "USBC_ForceVbusValidToHigh", "USBC_ForceVbusValidDisable")):
    f(a, 36, n, P, "1 = low, 2 = high, anything else = disable.", f"""
void {lo}(void);
void {hi}(void);
void {dis}(void);

void {n}(u32 type)
{{
    switch (type) {{
    case 1: {lo}(); break;
    case 2: {hi}(); break;
    default: {dis}(); break;
    }}
}}
""")
