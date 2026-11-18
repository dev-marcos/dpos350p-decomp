# File lists A (screenshots), B (waveforms) and C (captures), second part: removing an entry,
# finding the next free file number, creating the index files, loading files and thumbnails,
# saving a new item, and list C's records. Layout of the lists: see y_file_lists.py.
# exec'd by gen_platform.py (do not define HDR, SPEC, OUT or f here)


def wz(addr, size, name, desc, body, src="ui"):
    f(addr, size, name, src, desc, '#include "hw/fpga.h"' + "\n\n" + body.strip("\n"))


ZHDR = """#define SS16(o) (*(volatile short *)(0x40544E68u + (o)))
#define PATHBUF ((const char *)0x4054D728u)   /* SYS+0x88C0: path built by the list's path helper */
int f_open(void *fp, const char *path, unsigned char mode);
int f_close(void *fp);
int f_read(void *fp, void *buff, u32 btr, u32 *br);
int f_write(void *fp, const void *buff, u32 btw, u32 *bw);
int f_lseek(void *fp, unsigned long long ofs);
int f_mkdir(const char *path);
int f_chmod(const char *path, unsigned char attr, unsigned char mask);
void fill_u16(unsigned short *d, unsigned short v, u32 n);
"""

ZL = {
    "a": dict(cnt=0x754, rm=0x40034A0C, nf=0x40034AB4, path=0x40034B78, pathfn="screenshot_path_make", idx=0x4008DB60, dir=0x4008F82C,
              mk=0x400347A0),
    "b": dict(cnt=0x756, rm=0x40035A58, nf=0x40035B00, path=0x40035BC4, pathfn="waveform_path_make", idx=0x4008E2D0, dir=0x4008FFB4,
              mk=0x400357EC),
    "c": dict(cnt=0x758, rm=0x40036F74, nf=0x4003709C),
}

# ---------------------------------------------------------------- remove entry, next free number
for l, P in ZL.items():
    d = dict(P, l=l)
    wz(P["rm"], 168, "list_%s_entry_remove" % l,
       "Finds the entry equal to v among the first SYS+0x%(cnt)X entries (index 0 when absent), shifts the following ones down, zeroes the last one and decrements the count SYS+0xFA0 when it is not zero." % d, ZHDR + """
void list_%(l)s_entry_remove(u32 v)
{
    int n = SYS16(0x%(cnt)X), i, k = 0, c;
    for (i = 0; i < n; i++) {
        if (SYS16(0xFA6 + 2 * i) == v) {
            k = i;
            break;
        }
    }
    for (; k < SYS16(0x%(cnt)X) - 1; k++)
        SYS16(0xFA6 + 2 * k) = SYS16(0xFA8 + 2 * k);
    SYS16(0xFA4 + 2 * SYS16(0x%(cnt)X)) = 0;
    c = SS16(0xFA0);
    if (c)
        SS16(0xFA0) = c - 1;
}
""" % d)
    wz(P["nf"], 196, "list_%s_next_free" % l,
       "Clears the used table SYS+0x8924 (SYS+0x%(cnt)X halfwords), sets used[e] = 1 for each non-zero entry e, then stores in SYS+0xF9C the first number from 1 whose slot is free (unchanged when none is)." % d, ZHDR + """
void list_%(l)s_next_free(void)
{
    int i, n;
    for (i = 0; i < SYS16(0x%(cnt)X); i++)
        SYS16(0x8924 + 2 * i) = 0;
    for (i = 0; i < SS16(0xFA0); i++) {
        u32 e = SYS16(0xFA6 + 2 * i);
        if (e)
            SYS16(0x8924 + 2 * e) = 1;
    }
    n = SYS16(0x%(cnt)X);
    for (i = 1; i < n; i++) {
        if (SYS16(0x8924 + 2 * i) == 0) {
            SYS16(0xF9C) = i;
            return;
        }
    }
}
""" % d)

# ---------------------------------------------------------------- index files created from scratch
for l in "ab":
    d = dict(ZL[l], l=l)
    wz(d["mk"], 204, "list_%s_index_create" % l,
       "Run when the index file does not open: creates the directory 0x%(dir)08X, opens 0x%(idx)08X (mode 7), sets its attributes (f_chmod 2/2 and 4/4), zeroes SYS+0x%(cnt)X + 2 entries and the count, and writes the empty index." % d, ZHDR + """
void list_%(l)s_index_create(void)
{
    unsigned char fil[0x250];
    f_mkdir((const char *)0x%(dir)08Xu);
    f_open(fil, (const char *)0x%(idx)08Xu, 7);
    f_chmod((const char *)0x%(idx)08Xu, 2, 2);
    f_chmod((const char *)0x%(idx)08Xu, 4, 4);
    fill_u16((unsigned short *)(0x40544E68u + 0xFA6u), 0, SYS16(0x%(cnt)X) + 2);
    SS16(0xFA0) = 0;
    f_lseek(fil, 0);
    f_write(fil, (void *)(0x40544E68u + 0xFA0u), 2, 0);
    f_lseek(fil, 2);
    f_write(fil, (void *)(0x40544E68u + 0xFA6u), SYS16(0x%(cnt)X) * 2u, 0);
    f_close(fil);
}
""" % d)

wz(0x40036958, 424, "list_c_index_create",
   "Run when a list C file does not open: creates 0x4008FA2C, writes an empty index 0x4008E2EC (count 0, SYS+0x3DA52E = 1, zeroed entries) and a data file 0x4008E8D4 of SYS+0x758 * SYS+0x3DA544 halfwords. The data block is cleared with copy_u16 from address 0, as the firmware does.", ZHDR + """
void list_c_index_create(void)
{
    unsigned char fil[0x250];
    f_mkdir((const char *)0x4008FA2Cu);
    f_chmod((const char *)0x4008FA2Cu, 2, 2);
    f_chmod((const char *)0x4008FA2Cu, 4, 4);
    f_open(fil, (const char *)0x4008E2ECu, 0xB);
    f_chmod((const char *)0x4008E2ECu, 2, 2);
    f_chmod((const char *)0x4008E2ECu, 4, 4);
    fill_u16((unsigned short *)(0x40544E68u + 0xFA6u), 0, SYS16(0x758));
    SYS16(0x3DA52E) = 1;
    SS16(0xFA0) = 0;
    f_lseek(fil, 0);
    f_write(fil, (void *)(0x40544E68u + 0xFA0u), 2, 0);
    f_lseek(fil, 2);
    f_write(fil, (void *)(0x40544E68u + 0x3DA52Eu), 2, 0);
    f_lseek(fil, 4);
    f_write(fil, (void *)(0x40544E68u + 0xFA6u), SYS16(0x758) * 2u, 0);
    f_close(fil);
    f_open(fil, (const char *)0x4008E8D4u, 0xB);
    f_chmod((const char *)0x4008E8D4u, 2, 2);
    f_chmod((const char *)0x4008E8D4u, 4, 4);
    copy_u16((unsigned short *)(0x40544E68u + 0xD744u), 0, SYS16(0x758) * SYS32(0x3DA544));
    f_lseek(fil, 0);
    f_write(fil, (void *)(0x40544E68u + 0xD744u), SYS16(0x758) * SYS32(0x3DA544), 0);
    f_close(fil);
}
""")

wz(0x40036B00, 220, "list_c_index_load",
   "Reads the index file 0x4008E2EC: the count (offset 0), the u16 SYS+0x3DA52E (offset 2) and SYS+0x758 * 2 bytes of entries (offset 4). When it does not open, list_c_index_create. Then the page count SYS+0xFA2 = count / page size, rounded up.", ZHDR + """
void list_c_index_create(void);

void list_c_index_load(void)
{
    unsigned char fil[0x250];
    int c, ps, q;
    if (f_open(fil, (const char *)0x4008E2ECu, 1) != 0) {
        list_c_index_create();
    } else {
        f_lseek(fil, 0);
        f_read(fil, (void *)(0x40544E68u + 0xFA0u), 2, 0);
        f_lseek(fil, 2);
        f_read(fil, (void *)(0x40544E68u + 0x3DA52Eu), 2, 0);
        f_lseek(fil, 4);
        f_read(fil, (void *)(0x40544E68u + 0xFA6u), SYS16(0x758) * 2u, 0);
        f_close(fil);
    }
    c = SS16(0xFA0);
    ps = SYS16(0xF9A);
    q = c / ps;
    if (c - ps * q)
        q++;
    SS16(0xFA2) = q;
}
""")

# ---------------------------------------------------------------- loading files
wz(0x40035780, 108, "file_write_360404",
   "Creates the file at path (mode 0xB) and writes word SYS+0x774 bytes of the block at SYS+0x360404 from offset 0. Returns 1 when f_write succeeds, 0 otherwise (also when the file does not open).", ZHDR + """
u32 file_write_360404(const char *path)
{
    unsigned char fil[0x250];
    u32 r = 0;
    if (f_open(fil, path, 0xB) == 0) {
        f_lseek(fil, 0);
        if (f_write(fil, (void *)(0x40544E68u + 0x360404u), SYS32(0x774), 0) == 0)
            r = 1;
        f_close(fil);
    }
    return r;
}
""")

wz(0x40035188, 192, "list_a_thumb_load",
   "Opens the file at SYS+0x88C0 and reads the u16 at offset 0x46 into SYS+0x109684, the u16 at 0x48 into SYS+0x109686 and their product * 2 bytes from 0x4A into SYS+0x109688.", ZHDR + """
void list_a_thumb_load(void)
{
    unsigned char fil[0x250];
    if (f_open(fil, PATHBUF, 1) != 0)
        return;
    f_lseek(fil, 0x46);
    f_read(fil, (void *)(0x40544E68u + 0x109684u), 2, 0);
    f_lseek(fil, 0x48);
    f_read(fil, (void *)(0x40544E68u + 0x109686u), 2, 0);
    f_lseek(fil, 0x4A);
    f_read(fil, (void *)(0x40544E68u + 0x109688u), SYS16(0x109686) * SYS16(0x109684) * 2u, 0);
    f_close(fil);
}
""")

wz(0x40036260, 148, "list_b_file_load",
   "Opens the file at SYS+0x88C0, reads its 0x1C-byte header into SYS+0x360404 and decodes it (settings_h_decode), then reads word SYS+0x76C bytes from offset SYS+0x760 to the same place in the block and decodes them (settings_i_decode).", ZHDR + """
void settings_h_decode(void);
void settings_i_decode(void);

void list_b_file_load(void)
{
    unsigned char fil[0x250];
    if (f_open(fil, PATHBUF, 1) != 0)
        return;
    f_lseek(fil, 0);
    f_read(fil, (void *)(0x40544E68u + 0x360404u), 0x1C, 0);
    settings_h_decode();
    f_lseek(fil, SYS32(0x760));
    f_read(fil, (void *)(0x40544E68u + 0x360404u + SYS32(0x760)), SYS32(0x76C), 0);
    f_close(fil);
    settings_i_decode();
}
""")

for l, a, load in (("a", 0x40035578, "list_a_thumb_load"), ("b", 0x400365F4, "list_b_file_load")):
    d = dict(ZL[l], l=l, load=load)
    wz(a, 172, "list_%s_page_thumbs" % l,
       "For up to 3 x 5 entries of the current page (stopping at the first zero one): builds the path (0x%(path)08X), loads the file (%(load)s) and copies u16 SYS+0x109684 * u16 SYS+0x109686 halfwords from SYS+0x109684 to slot n (SYS+0x12ED5C + n * 0x256D8)." % d, ZHDR + """
void %(pathfn)s(u32 v);
void %(load)s(void);

void list_%(l)s_page_thumbs(void)
{
    int k = SYS16(0xF9A) * SS16(0xF9E), n = 0, row, col;
    for (row = 0; row < 3; row++) {
        for (col = 0; col < 5; col++) {
            u32 v = SYS16(0xFA6 + 2 * k);
            if (v == 0)
                return;
            %(pathfn)s(v);
            %(load)s();
            k++;
            copy_u16((unsigned short *)(0x40544E68u + 0x12ED5Cu + (u32)(unsigned short)n * 0x256D8u),
                     (const unsigned short *)(0x40544E68u + 0x109684u), SYS16(0x109684) * SYS16(0x109686));
            n++;
        }
    }
}
""" % d)

wz(0x40035624, 116, "list_a_current_load",
   "Builds the path of the selected entry (SYS+0xFA4) and reads 0x12B6A0 bytes from offset 0xDEA8 of that file to 0x42468A58.", ZHDR + """
void screenshot_path_make(u32 v);

void list_a_current_load(void)
{
    unsigned char fil[0x250];
    screenshot_path_make(SYS16(0xFA6 + 2 * SS16(0xFA4)));
    if (f_open(fil, PATHBUF, 1) != 0)
        return;
    f_lseek(fil, 0xDEA8);
    f_read(fil, (void *)0x42468A58u, 0x12B6A0, 0);
    f_close(fil);
}
""")

wz(0x400366A0, 200, "list_b_current_load",
   "Builds the path of the selected entry (SYS+0xFA4); when the file opens: state_restore_all when SYS+0x665 is set, state_backup_all, the 0x1C-byte header into SYS+0x360404 (settings_h_decode), then word SYS+0x774 bytes from offset 0 into the same block (settings_image_decode); finally SYS+0x665 = SYS+0x3C = 1.", ZHDR + """
void waveform_path_make(u32 v);
void settings_h_decode(void);
void settings_image_decode(void);

void list_b_current_load(void)
{
    unsigned char fil[0x250];
    waveform_path_make(SYS16(0xFA6 + 2 * SS16(0xFA4)));
    if (f_open(fil, PATHBUF, 1) != 0)
        return;
    if (SYS8(0x665))
        state_restore_all();
    state_backup_all();
    f_lseek(fil, 0);
    f_read(fil, (void *)(0x40544E68u + 0x360404u), 0x1C, 0);
    settings_h_decode();
    f_lseek(fil, 0);
    f_read(fil, (void *)(0x40544E68u + 0x360404u), SYS32(0x774), 0);
    f_close(fil);
    settings_image_decode();
    SYS8(0x665) = 1;
    SYS8(0x3C) = 1;
}
""")

# ---------------------------------------------------------------- saving a new item
wz(0x4003506C, 136, "list_a_save_new",
   "Saves a screenshot: screenshot_thumb_make, shows a message box (text pointer word at 0x42C3C994) at the screen centre plus its offsets, reloads the index, takes the next free number, builds its path and writes the file (bmp_save); when that returns non-zero the number is inserted and the index saved. Ends with rect_42e68c38_draw.", ZHDR + """
void screenshot_thumb_make(void);
void list_a_index_load(void);
void list_a_next_free(void);
void screenshot_path_make(u32 v);
int bmp_save(const char *path);
void list_fa4_insert_754(u32 v);
void list_a_index_save(void);
void rect_42e68c38_draw(void);

void list_a_save_new(void)
{
    const signed char *t = (const signed char *)(0x42C2FE74u + 0xCB14u);
    int x = SYS16(4) + (SYS16(8) >> 1);
    int y = SYS16(6) + (SYS16(0xA) >> 1);
    screenshot_thumb_make();
    message_box_draw(x + t[0], y + t[1], *(const u32 *)(t + 0xC), 0);
    list_a_index_load();
    list_a_next_free();
    screenshot_path_make(SYS16(0xF9C));
    if (bmp_save(PATHBUF)) {
        list_fa4_insert_754(SYS16(0xF9C));
        list_a_index_save();
    }
    rect_42e68c38_draw();
}
""")

wz(0x400360F0, 220, "list_b_save_new",
   "Saves a waveform: when disk_free_bytes is below twice word SYS+0x774, shows the message box of the text word 0x42C3D820 at the screen centre (offsets at 0x42C3D828) and waits 1 s; otherwise reloads the index, takes the next free number, waveform_thumb_make, shows the message box of the text word 0x42C3C7F4, builds the path, encodes the settings image and writes it (file_write_360404); on success the number is inserted and the index saved. Ends with rect_42e68c38_draw.", ZHDR + """
u32 disk_free_bytes(void);
void list_b_index_load(void);
void list_b_next_free(void);
void waveform_thumb_make(void);
void waveform_path_make(u32 v);
void settings_image_encode(void);
u32 file_write_360404(const char *path);
void list_fa4_insert_756(u32 v);
void list_b_index_save(void);
void rect_42e68c38_draw(void);

void list_b_save_new(void)
{
    const unsigned char *p = (const unsigned char *)(0x42C2FE74u + 0xC980u);
    int x = SYS16(4) + (SYS16(8) >> 1);
    int y = SYS16(6) + (SYS16(0xA) >> 1);
    if (disk_free_bytes() < SYS32(0x774) * 2u) {
        const signed char *t = (const signed char *)(0x42C2FE74u + 0xD9B4u);
        message_box_draw(x + t[0], y + t[1], *(const u32 *)(p + 0xEA0), 0);
        delay_ms(1000);
    } else {
        list_b_index_load();
        list_b_next_free();
        waveform_thumb_make();
        message_box_draw(x + (signed char)p[0x194], y + (signed char)p[0x195], *(const u32 *)p, 0);
        waveform_path_make(SYS16(0xF9C));
        settings_image_encode();
        if (file_write_360404(PATHBUF)) {
            list_fa4_insert_756(SYS16(0xF9C));
            list_b_index_save();
        }
    }
    rect_42e68c38_draw();
}
""")

# ---------------------------------------------------------------- list C records and deletion
wz(0x40036EA4, 208, "list_c_record_select",
   "Looks for the record of key v in the data block SYS+0xD744 (records of word SYS+0x3DA544 halfwords, SYS+0x758 of them; key = low byte of halfword 0 << 8 | low byte of halfword 1). When found: SYS+0xF9C = its key, word SYS+0x3DA53C = halfwords 2 and 3 packed the same way, and that many halfwords from record + 8 bytes are copied to SYS+0x3DA548.", ZHDR + """
void list_c_record_select(u32 v)
{
    volatile unsigned short *p = (volatile unsigned short *)(0x40544E68u + 0xD744u);
    int st = SYS32(0x3DA544), n = st * SYS16(0x758), i;
    for (i = 0; i < n; i += st, p += st) {
        if (((u32)((p[0] << 8) & 0xFF00) | (p[1] & 0xFF)) == v) {
            u32 k = (u32)(short)p[0] << 8, c;
            SYS16(0xF9C) = k;
            SYS16(0xF9C) = (p[1] & 0xFF) | k;
            c = (p[2] << 8) & 0xFF00;
            SYS32(0x3DA53C) = c;
            c |= p[3] & 0xFF;
            SYS32(0x3DA53C) = c;
            copy_u16((unsigned short *)(0x40544E68u + 0x3DA548u), (const unsigned short *)(p + 4), c);
            return;
        }
    }
}
""")

wz(0x400367EC, 120, "cursor_3da534_clamp",
   "Cursor pair a = word SYS+0x3DA534, b = word SYS+0x3DA538, width w = SYS+0x3DA540: when b < a + w, a = b - w (flag 0) or b = a + w (flag non-zero). Then both are clamped to 12 .. u16 SYS+8 - 12.", ZHDR + """
void cursor_3da534_clamp(int flag)
{
    int a = SYS32(0x3DA534), w = SYS32(0x3DA540), b = SYS32(0x3DA538);
    if (b < (int)((u32)w + (u32)a)) {
        if (flag == 0)
            a = (int)((u32)b - (u32)w);
        else
            b = (int)((u32)w + (u32)a);
    }
    if (a < 12)
        a = 12;
    else if (SYS16(8) - 12 < a)
        a = SYS16(8) - 12;
    if (b < 12)
        b = 12;
    else if (SYS16(8) - 12 < b)
        b = SYS16(8) - 12;
    SYS32(0x3DA534) = a;
    SYS32(0x3DA538) = b;
}
""")

wz(0x40037460, 140, "list_c_delete_marked",
   "From the last entry down: for each marked entry (SYS+0x5DC6+i), list_current_advance(i), list_c_entry_remove and record_clear of its value. Then list_a_marks_clear, the index and data saved and reloaded, SYS+0xF90 = 0.", ZHDR + """
void list_current_advance(int i);
void list_c_entry_remove(u32 v);
void record_clear(u32 v);
void list_a_marks_clear(void);
void list_c_index_save(void);
void list_c_data_save(void);
void list_c_index_load(void);
void list_c_data_load(void);

void list_c_delete_marked(void)
{
    int i;
    for (i = SS16(0xFA0) - 1; i >= 0; i--) {
        if (SYS8(0x5DC6 + i)) {
            u32 v = SYS16(0xFA6 + 2 * i);
            list_current_advance((unsigned short)i);
            list_c_entry_remove(v);
            record_clear(v);
        }
    }
    list_a_marks_clear();
    list_c_index_save();
    list_c_data_save();
    list_c_index_load();
    list_c_data_load();
    SYS8(0xF90) = 0;
}
""")
