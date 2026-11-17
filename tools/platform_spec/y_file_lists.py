# Three file lists A, B and C. Each has an index file holding the s16 count (SYS+0xFA0) and the
# u16 entries (SYS+0xFA6..), paged by the u16 page size SYS+0xF9A; the page index is the s16
# SYS+0xF9E, the page count SYS+0xFA2. The per-page marks live at SYS+0x84D6 and the per-entry
# marks at SYS+0x5DC6. The lists differ only in their index path, entry length field (SYS+0x754,
# +0x756, +0x758) and helper functions.
# exec'd by gen_platform.py


def w(addr, size, name, desc, body, src="ui"):
    f(addr, size, name, src, desc, '#include "hw/fpga.h"' + "\n\n" + body.strip("\n"))


FHDR = """#define SS16(o) (*(volatile short *)(0x40544E68u + (o)))
#define PATHBUF ((const char *)0x4054D728u)   /* SYS+0x88C0: path built by the list's path helper */
int f_open(void *fp, const char *path, unsigned char mode);
int f_close(void *fp);
int f_read(void *fp, void *buff, u32 btr, u32 *br);
int f_write(void *fp, const void *buff, u32 btw, u32 *bw);
int f_lseek(void *fp, unsigned long long ofs);
int f_unlink(const char *path);
"""

# name, path helper, entry remover, index path, index fail handler, length field, cursor clamp
LISTS = {
    "a": dict(path=0x40034B78, remove=0x40034A0C, idx=0x4008DB60, fail=0x400347A0, cnt=0x754,
              clamp="list_cursor_clamp_40035248", exists="file_exists_40034b78"),
    "b": dict(path=0x40035BC4, remove=0x40035A58, idx=0x4008E2D0, fail=0x400357EC, cnt=0x756,
              clamp="list_cursor_clamp_400362f4", exists="file_exists_40035bc4"),
}

# ---------------------------------------------------------------- page marks
for a, l in ((0x400352A8, "a"), (0x40036354, "b"), (0x4003777C, "c")):
    w(a, 120, "list_%s_page_mark" % l,
      "Marks the current page: byte SYS+0x84D6+page = 1, and the per-entry marks SYS+0x5DC6 of the entries of that page (up to the count) are set to 1 with __aeabi_memset.", FHDR + """
void __aeabi_memset_entry(void *d, u32 n, u32 c);

void list_%s_page_mark(void)
{
    int ps, start, end, lim;
    SYS8(0x84D6 + SS16(0xF9E)) = 1;
    ps = SYS16(0xF9A);
    start = ps * SS16(0xF9E);
    end = ps + start;
    lim = SS16(0xFA0);
    if (lim > end)
        lim = end;
    if (start < lim)
        __aeabi_memset_entry((void *)(0x40544E68u + 0x5DC6u + start), lim - start, 1);
}
""" % l)
for a, l in ((0x40035320, "a"), (0x400363CC, "b"), (0x400377F8, "c")):
    w(a, 136, "list_%s_marks_clear" % l,
      "Clears the page marks SYS+0x84D6 (count / page size + 1 of them) and the entry marks SYS+0x5DC6 (count of them).", FHDR + """
void list_%s_marks_clear(void)
{
    int i;
    for (i = 0; i < SS16(0xFA0) / (int)SYS16(0xF9A) + 1; i++)
        SYS8(0x84D6 + i) = 0;
    for (i = 0; i < SS16(0xFA0); i++)
        SYS8(0x5DC6 + i) = 0;
}
""" % l)

# ---------------------------------------------------------------- per-list functions A and B
for l, P in LISTS.items():
    d = dict(P, l=l)
    load_a = {"a": 0x4003486C, "b": 0x400358B8}[l]
    save_a = {"a": 0x40034920, "b": 0x4003596C}[l]
    prune_a = {"a": 0x40035434, "b": 0x400364E0}[l]
    valid_a = {"a": 0x400354C0, "b": 0x4003656C}[l]
    del_a = {"a": 0x400350F4, "b": 0x400361CC}[l]
    clear_n = "list_%s_marks_clear" % l
    w(load_a, 180, "list_%s_index_load" % l,
      "Reads the index file 0x%(idx)08X: the count into SYS+0xFA0 (2 bytes at offset 0) and SYS+0x%(cnt)X * 2 bytes of entries from offset 2 into SYS+0xFA6. When it does not open, 0x%(fail)08X. Then the page count SYS+0xFA2 = count / page size, rounded up." % d, FHDR + """
void list_%(l)s_index_create(void);

void list_%(l)s_index_load(void)
{
    unsigned char fil[0x250];
    int c, ps, q;
    if (f_open(fil, (const char *)0x%(idx)08Xu, 1) != 0) {
        list_%(l)s_index_create();
    } else {
        f_lseek(fil, 0);
        f_read(fil, (void *)(0x40544E68u + 0xFA0u), 2, 0);
        f_lseek(fil, 2);
        f_read(fil, (void *)(0x40544E68u + 0xFA6u), SYS16(0x%(cnt)X) * 2u, 0);
        f_close(fil);
    }
    c = SS16(0xFA0);
    ps = SYS16(0xF9A);
    q = c / ps;
    if (c - ps * q)
        q++;
    SS16(0xFA2) = q;
}
""" % d)
    w(save_a, 148, "list_%s_index_save" % l,
      "Writes the index file 0x%(idx)08X (opened read/write): the count at offset 0 and SYS+0x%(cnt)X * 2 bytes of entries at offset 2. When it does not open, 0x%(fail)08X." % d, FHDR + """
void list_%(l)s_index_create(void);

void list_%(l)s_index_save(void)
{
    unsigned char fil[0x250];
    if (f_open(fil, (const char *)0x%(idx)08Xu, 3) != 0) {
        list_%(l)s_index_create();
        return;
    }
    f_lseek(fil, 0);
    f_write(fil, (void *)(0x40544E68u + 0xFA0u), 2, 0);
    f_lseek(fil, 2);
    f_write(fil, (void *)(0x40544E68u + 0xFA6u), SYS16(0x%(cnt)X) * 2u, 0);
    f_close(fil);
}
""" % d)
    d.update(save="list_%s_index_save" % l, load="list_%s_index_load" % l)
    w(prune_a, 140, "list_%s_prune_missing" % l,
      "From the last entry down: builds its path (0x%(path)08X) and, when f_open of it fails, removes the entry (0x%(remove)08X); then saves and reloads the index and clamps the cursor." % d, FHDR + """
void func_%(path)08x(u32 v);
void list_%(l)s_entry_remove(u32 v);
void %(save)s(void);
void %(load)s(void);
void %(clamp)s(void);

void list_%(l)s_prune_missing(void)
{
    unsigned char fil[0x250];
    int i;
    for (i = SS16(0xFA0) - 1; i >= 0; i--) {
        u32 v = SYS16(0xFA6 + 2 * i);
        func_%(path)08x(v);
        if (f_open(fil, PATHBUF, 1) == 0)
            f_close(fil);
        else
            list_%(l)s_entry_remove(v);
    }
    %(save)s();
    %(load)s();
    %(clamp)s();
}
""" % d)
    d["prune"] = "list_%s_prune_missing" % l
    w(valid_a, 120, "list_%s_page_validate" % l,
      "Page index = selected entry SYS+0xFA4 / page size; walks the entries of that page until a zero entry, and when one of them has no file (%(exists)s), runs %(prune)s and stops. Then the cursor clamp." % d, FHDR + """
u32 %(exists)s(u32 v);
void %(prune)s(void);
void %(clamp)s(void);

void list_%(l)s_page_validate(void)
{
    volatile unsigned short *p;
    int i;
    SS16(0xF9E) = SS16(0xFA4) / (int)SYS16(0xF9A);
    p = (volatile unsigned short *)(0x40544E68u + 0xFA6u + 2u * (u32)(SYS16(0xF9A) * SS16(0xF9E)));
    for (i = 0; i < SYS16(0xF9A); i++) {
        u32 v = *p++;
        if (v == 0)
            break;
        if (%(exists)s(v) == 0) {
            %(prune)s();
            break;
        }
    }
    %(clamp)s();
}
""" % d)
    d["clear"] = clear_n
    w(del_a, 148, "list_%s_delete_marked" % l,
      "From the last entry down: each entry whose mark SYS+0x5DC6+i is set is removed (0x%(remove)08X) and its file deleted (path 0x%(path)08X, f_unlink); then the marks are cleared, the index saved and reloaded, and SYS+0xF90 = 0." % d, FHDR + """
void func_%(path)08x(u32 v);
void list_%(l)s_entry_remove(u32 v);
void %(clear)s(void);
void %(save)s(void);
void %(load)s(void);

void list_%(l)s_delete_marked(void)
{
    int i;
    for (i = SS16(0xFA0) - 1; i >= 0; i--) {
        if (SYS8(0x5DC6 + i)) {
            u32 v = SYS16(0xFA6 + 2 * i);
            list_%(l)s_entry_remove(v);
            func_%(path)08x(v);
            f_unlink(PATHBUF);
        }
    }
    %(clear)s();
    %(save)s();
    %(load)s();
    SYS8(0xF90) = 0;
}
""" % d)

# ---------------------------------------------------------------- list C files
w(0x40036C54, 188, "list_c_index_save",
  "Writes the index file 0x4008E2EC (opened read/write): the count at offset 0, the u16 SYS+0x3DA52E at offset 2 and SYS+0x758 * 2 bytes of entries at offset 4. When it does not open, 0x40036958.", FHDR + """
void list_c_index_create(void);

void list_c_index_save(void)
{
    unsigned char fil[0x250];
    if (f_open(fil, (const char *)0x4008E2ECu, 3) != 0) {
        list_c_index_create();
        return;
    }
    f_lseek(fil, 0);
    f_write(fil, (void *)(0x40544E68u + 0xFA0u), 2, 0);
    f_lseek(fil, 2);
    f_write(fil, (void *)(0x40544E68u + 0x3DA52Eu), 2, 0);
    f_lseek(fil, 4);
    f_write(fil, (void *)(0x40544E68u + 0xFA6u), SYS16(0x758) * 2u, 0);
    f_close(fil);
}
""")
for a, name, mode, op in ((0x40036BDC, "list_c_data_load", 1, "f_read"), (0x40036D10, "list_c_data_save", 3, "f_write")):
    w(a, 120, name,
      "The file 0x4008E8D4 (mode %d) at offset 0: %s of word SYS+0x3DA544 * u16 SYS+0x758 bytes at SYS+0xD744. When it does not open, 0x40036958." % (mode, op), FHDR + """
void list_c_index_create(void);

void %s(void)
{
    unsigned char fil[0x250];
    if (f_open(fil, (const char *)0x4008E8D4u, %d) != 0) {
        list_c_index_create();
        return;
    }
    f_lseek(fil, 0);
    %s(fil, (void *)(0x40544E68u + 0xD744u), SYS32(0x3DA544) * SYS16(0x758), 0);
    f_close(fil);
}
""" % (name, mode, op))
