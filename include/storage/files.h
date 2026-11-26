/* Functions of nonmatching/storage/files/ (one file per function, named after it).
 * Addresses are of build 2025-06-19. Names are DERIVED unless the C file says otherwise. */
#ifndef STORAGE_FILES_H
#define STORAGE_FILES_H

#include "rt/bits.h"

void list_a_index_create(void);                                                                  /* 0x400347A0 */
void list_a_index_load(void);                                                                    /* 0x4003486C */
void list_a_index_save(void);                                                                    /* 0x40034920 */
void list_a_insert(u32 v);                                                                 /* 0x400349B4 */
void list_a_entry_remove(u32 v);                                                                 /* 0x40034A0C */
void list_a_next_free(void);                                                                     /* 0x40034AB4 */
void screenshot_path_make(int n);                                                                /* 0x40034B78 */
void screenshot_thumb_make(void);                                                                /* 0x40034E94 */
void list_a_save_new(void);                                                                      /* 0x4003506C */
void list_a_delete_marked(void);                                                                 /* 0x400350F4 */
void list_a_thumb_load(void);                                                                    /* 0x40035188 */
void list_a_cursor_clamp(void);                                                           /* 0x40035248 */
void list_a_page_mark(void);                                                                     /* 0x400352A8 */
void list_a_marks_clear(void);                                                                   /* 0x40035320 */
void list_a_marks_count(void);                                                          /* 0x400353A8 */
u32 screenshot_file_exists(u32 a);                                                                 /* 0x400353F4 */
void list_a_prune_missing(void);                                                                 /* 0x40035434 */
void list_a_page_validate(void);                                                                 /* 0x400354C0 */
void list_a_reload(void);                                                                        /* 0x40035540 */
void list_a_page_thumbs(void);                                                                   /* 0x40035578 */
void list_a_current_load(void);                                                                  /* 0x40035624 */
u32 file_write_360404(const char *path);                                                         /* 0x40035780 */
void list_b_index_create(void);                                                                  /* 0x400357EC */
void list_b_index_load(void);                                                                    /* 0x400358B8 */
void list_b_index_save(void);                                                                    /* 0x4003596C */
void list_b_insert(u32 v);                                                                 /* 0x40035A00 */
void list_b_entry_remove(u32 v);                                                                 /* 0x40035A58 */
void list_b_next_free(void);                                                                     /* 0x40035B00 */
void waveform_path_make(int n);                                                                  /* 0x40035BC4 */
void waveform_thumb_make(void);                                                                  /* 0x40035EE0 */
void list_b_save_new(void);                                                                      /* 0x400360F0 */
void list_b_delete_marked(void);                                                                 /* 0x400361CC */
void list_b_file_load(void);                                                                     /* 0x40036260 */
void list_b_cursor_clamp(void);                                                           /* 0x400362F4 */
void list_b_page_mark(void);                                                                     /* 0x40036354 */
void list_b_marks_clear(void);                                                                   /* 0x400363CC */
void list_b_marks_count(void);                                                          /* 0x40036454 */
u32 waveform_file_exists(u32 a);                                                                 /* 0x400364A0 */
void list_b_prune_missing(void);                                                                 /* 0x400364E0 */
void list_b_page_validate(void);                                                                 /* 0x4003656C */
void list_b_page_thumbs(void);                                                                   /* 0x400365F4 */
void list_b_current_load(void);                                                                  /* 0x400366A0 */
void cursor_3da534_clamp(int flag);                                                              /* 0x400367EC */
void saved_wave_trace_build(void);                                                               /* 0x40036864 */
void list_c_index_create(void);                                                                  /* 0x40036958 */
void list_c_index_load(void);                                                                    /* 0x40036B00 */
void list_c_data_load(void);                                                                     /* 0x40036BDC */
void list_c_index_save(void);                                                                    /* 0x40036C54 */
void list_c_data_save(void);                                                                     /* 0x40036D10 */
void list_c_insert(u32 v);                                                                 /* 0x40036D88 */
void record_append(u32 a0, u32 a1, u32 r2_in);                                                   /* 0x40036DE4 */
void list_c_record_select(u32 v);                                                                /* 0x40036EA4 */
void list_c_entry_remove(u32 v);                                                                 /* 0x40036F74 */
void record_clear(u32 id, u32 a1, u32 a2, u32 r3_in);                                            /* 0x40037020 */
void list_c_next_free(void);                                                                     /* 0x4003709C */
void list_current_advance(int i);                                                                /* 0x40037418 */
void list_c_delete_marked(void);                                                                 /* 0x40037460 */
void list_c_record_preview(u32 a);                                                               /* 0x4003754C */
void list_c_cursor_clamp(void);                                                           /* 0x4003771C */
void list_c_page_mark(void);                                                                     /* 0x4003777C */
void list_c_marks_clear(void);                                                                   /* 0x400377F8 */
void list_c_marks_count(void);                                                          /* 0x40037880 */
void record_c_fields(void);                                                                      /* 0x40043B0C */
int bmp_save(const char *path);                                                                  /* 0x4005C3F0 */
void check_pair_save(void);                                                                      /* 0x40064DC0 */

#endif
