/*
 * font_glyph_index -- stock 0x4002DD70 (2408 bytes, ARM)
 * Status: NONMATCHING; behaviour (incl. the ordered MMIO access sequence) verified by tools/emu
 * Glyph index of character code c in the proportional fonts (font type 1): 'A'..'Z' = 0..25, 'a'..'z' = 26..51, Cyrillic 52..99, accented Latin and German letters 100..109, '0'..'9' = 110..119, punctuation 120..134. Accented vowels with pinyin tones map to the base letter. Any other code gives 0xFFFF. The full 32-bit value is compared. The table was read from the stock code (a 186-entry switch).
 */
#include "hw/platform.h"

#include "hw/fpga.h"

unsigned short font_glyph_index(u32 c)
{
    if (c - 0x30u <= 9)
        return (unsigned short)(c - 0x30u + 110);
    if (c - 0x41u <= 25)
        return (unsigned short)(c - 0x41u);
    if (c - 0x61u <= 25)
        return (unsigned short)(c - 0x61u + 26);
    switch (c) {
    case 0x0020:
        return 134;
    case 0x0021:
        return 126;
    case 0x0025:
        return 125;
    case 0x0028: case 0xFF08:
        return 127;
    case 0x0029: case 0xFF09:
        return 128;
    case 0x002A:
        return 123;
    case 0x002B:
        return 121;
    case 0x002C:
        return 132;
    case 0x002D:
        return 122;
    case 0x002E:
        return 120;
    case 0x002F:
        return 124;
    case 0x003A:
        return 131;
    case 0x003D:
        return 130;
    case 0x003F:
        return 133;
    case 0x005F:
        return 129;
    case 0x00C4:
        return 102;
    case 0x00D1:
        return 100;
    case 0x00D6:
        return 103;
    case 0x00DC:
        return 104;
    case 0x00DF:
        return 109;
    case 0x00E0: case 0x00E1: case 0x00E3: case 0x0101: case 0x01CE: case 0x0430:
        return 26;
    case 0x00E4:
        return 106;
    case 0x00E7: case 0x0441:
        return 28;
    case 0x00E8: case 0x00E9: case 0x00EA: case 0x0113: case 0x011B: case 0x0435:
        return 30;
    case 0x00EC: case 0x00ED: case 0x012B: case 0x01D0:
        return 34;
    case 0x00F1:
        return 101;
    case 0x00F2: case 0x00F3: case 0x00F5: case 0x014D: case 0x01D2: case 0x043E:
        return 40;
    case 0x00F6:
        return 107;
    case 0x00F9: case 0x00FA: case 0x016B: case 0x01D4:
        return 46;
    case 0x00FC: case 0x01D6: case 0x01D8: case 0x01DA: case 0x01DC:
        return 108;
    case 0x0144: case 0x0148:
        return 39;
    case 0x0261:
        return 32;
    case 0x0401:
        return 55;
    case 0x0410:
        return 0;
    case 0x0411:
        return 52;
    case 0x0412:
        return 1;
    case 0x0413:
        return 53;
    case 0x0414:
        return 54;
    case 0x0415:
        return 4;
    case 0x0416:
        return 56;
    case 0x0417:
        return 57;
    case 0x0418:
        return 58;
    case 0x0419:
        return 59;
    case 0x041A:
        return 10;
    case 0x041B:
        return 60;
    case 0x041C:
        return 12;
    case 0x041D:
        return 7;
    case 0x041E:
        return 14;
    case 0x041F:
        return 61;
    case 0x0420:
        return 15;
    case 0x0421:
        return 2;
    case 0x0422:
        return 19;
    case 0x0423:
        return 62;
    case 0x0424:
        return 63;
    case 0x0425:
        return 23;
    case 0x0426:
        return 64;
    case 0x0427:
        return 65;
    case 0x0428:
        return 66;
    case 0x0429:
        return 67;
    case 0x042A:
        return 68;
    case 0x042B:
        return 69;
    case 0x042C:
        return 70;
    case 0x042D:
        return 71;
    case 0x042E:
        return 72;
    case 0x042F:
        return 73;
    case 0x0431:
        return 74;
    case 0x0432:
        return 75;
    case 0x0433:
        return 76;
    case 0x0434:
        return 77;
    case 0x0436:
        return 79;
    case 0x0437:
        return 80;
    case 0x0438:
        return 81;
    case 0x0439:
        return 82;
    case 0x043A:
        return 83;
    case 0x043B:
        return 84;
    case 0x043C:
        return 85;
    case 0x043D:
        return 86;
    case 0x043F:
        return 87;
    case 0x0440:
        return 41;
    case 0x0442:
        return 88;
    case 0x0443:
        return 50;
    case 0x0444:
        return 89;
    case 0x0445:
        return 49;
    case 0x0446:
        return 90;
    case 0x0447:
        return 91;
    case 0x0448:
        return 92;
    case 0x0449:
        return 93;
    case 0x044A:
        return 94;
    case 0x044B:
        return 95;
    case 0x044C:
        return 96;
    case 0x044D:
        return 97;
    case 0x044E:
        return 98;
    case 0x044F:
        return 99;
    case 0x0451:
        return 78;
    case 0x1E9E:
        return 105;
    default:
        return 0xFFFF;
    }
}
