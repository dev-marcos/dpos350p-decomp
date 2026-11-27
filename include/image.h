/*
 * Addresses inside the application image, as symbols instead of numbers.
 *
 * The C used to write a text, table or initialised global of the image as its stock address
 * (STR(0x4008E724), (const char *)0x40060B90u, 0x400857FCu...). That only works while the image
 * keeps the stock layout. These macros name the same thing as a symbol, d_<address>, which the
 * linker resolves:
 *   - in the stock-address builds (tools/nmbuild.py: emutest, selftest, testiso) every
 *     d_XXXXXXXX is defined at the address in its name;
 *   - in a relinked image (tools/relink_test.py, and later the build from C) it is the label the
 *     assembly defines at that place (tools/split.py emits one for every address used here).
 *
 *   IMG(4008e724)        pointer:  (unsigned char *) d_4008e724
 *   IMGU(400857fc)       integer:  (u32) address, where the C passes addresses as integers
 *   IMG_DECL(400860e4)   a file-scope declaration, for static initialisers ((u32)d_400860e4)
 *   CODE_ADDR(sqrt)      address of an image function as data (bit 0 set for Thumb), for the
 *                        calls through a pointer into the library code. The C name is
 *                        code_<name>, bound to the symbol by an asm label, so GCC never sees
 *                        sqrt/sin/pow... (its builtins) and keeps the indirect call.
 *
 * The argument is the address in lower-case hex without 0x: a preprocessing number, pasted
 * onto d_.
 */
#ifndef DPOS_IMAGE_H
#define DPOS_IMAGE_H

#define IMG(a)       ({ extern unsigned char d_##a[]; (unsigned char *)d_##a; })
#define IMGU(a)      ((unsigned int)IMG(a))
#define IMG_DECL(a)  extern unsigned char d_##a[]
#define CODE_ADDR(f) ({ extern unsigned char code_##f[] __asm__(#f); (void *)code_##f; })

#endif
