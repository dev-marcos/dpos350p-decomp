/*
 * csi_int_get_status -- stock 0x40065BD4 (124 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Channel INT_STA (+0x74) bits 0..7 to eight bytes (capture done, frame done, overflows, ...).
 */
#include "hw/platform.h"

#define csi_base_tbl ((u32 *)0x42E68C50u)
u32 csi_readl(u32 addr);
void csi_writel(u32 addr, u32 v);
void csi_setbits(u32 addr, u32 bits);
void csi_clrbits(u32 addr, u32 bits);
void csi_clrsetbits(u32 addr, u32 clr, u32 set);
void dcache_clean_inv_all(void);
void csi_writel_t(u32 addr, u32 v);
u32 csi_readl_t(u32 addr);

void csi_int_get_status(u32 sel, u32 ch, unsigned char *st)
{
    u32 v = csi_readl_t(csi_base_tbl[sel] + (ch << 8) + 0x74);
    int i;
    for (i = 0; i < 8; i++)
        st[i] = (v >> i) & 1;
}
