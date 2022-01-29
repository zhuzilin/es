
#include <utility>
#include <algorithm>
#include <functional>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <string_view>
#include <array>
#include <vector>
#include <unordered_map>
#include <map>
#include <bitset>
#include <set>
#include <stack>
#include <codecvt>
#include <locale>
#include <math.h>
#include <string.h>
#include <assert.h>

/* here cometh the local files.... */

/* file: "/mnt/c/Users/john/Desktop/interps/lic.permissive/hastry/zhuzilin_es/es/parser/unicode.h" */
/*
 * The code in this file is extracted directly from iv.
 */

#define START_POS size_t start = lexer_.Pos()
#define SOURCE_PARSED source_.substr(start, lexer_.Pos() - start)

namespace es
{
    namespace character
    {
        namespace unicode
        {
            enum Category
            {
                UNASSIGNED = 0,// Cn
                UPPERCASE_LETTER = 1,// Lu
                LOWERCASE_LETTER = 2,// Ll
                TITLECASE_LETTER = 3,// Lt
                MODIFIER_LETTER = 4,// Lm
                OTHER_LETTER = 5,// Lo
                NON_SPACING_MARK = 6,// Mn
                ENCLOSING_MARK = 7,// Me
                COMBINING_SPACING_MARK = 8,// Mc
                DECIMAL_DIGIT_NUMBER = 9,// Nd
                LETTER_NUMBER = 10,// Nl
                OTHER_NUMBER = 11,// No
                SPACE_SEPARATOR = 12,// Zs
                LINE_SEPARATOR = 13,// Zl
                PARAGRAPH_SEPARATOR = 14,// Zp
                CONTROL = 15,// Cc
                FORMAT = 16,// Cf
                PRIVATE_USE = 18,// Co
                SURROGATE = 19,// Cs
                DASH_PUNCTUATION = 20,// Pd
                START_PUNCTUATION = 21,// Ps
                END_PUNCTUATION = 22,// Pe
                CONNECTOR_PUNCTUATION = 23,// Pc
                OTHER_PUNCTUATION = 24,// Po
                MATH_SYMBOL = 25,// Sm
                CURRENCY_SYMBOL = 26,// Sc
                MODIFIER_SYMBOL = 27,// Sk
                OTHER_SYMBOL = 28,// So
                INITIAL_QUOTE_PUNCTUATION = 29,// Pi
                FINAL_QUOTE_PUNCTUATION = 30// Pf
            };

            enum CategoryFlag
            {
                Cn = 1 << UNASSIGNED,
                Lu = 1 << UPPERCASE_LETTER,
                Ll = 1 << LOWERCASE_LETTER,
                Lt = 1 << TITLECASE_LETTER,
                Lm = 1 << MODIFIER_LETTER,
                Lo = 1 << OTHER_LETTER,
                Mn = 1 << NON_SPACING_MARK,
                Me = 1 << ENCLOSING_MARK,
                Mc = 1 << COMBINING_SPACING_MARK,
                Nd = 1 << DECIMAL_DIGIT_NUMBER,
                Nl = 1 << LETTER_NUMBER,
                No = 1 << OTHER_NUMBER,
                Zs = 1 << SPACE_SEPARATOR,
                Zl = 1 << LINE_SEPARATOR,
                Zp = 1 << PARAGRAPH_SEPARATOR,
                Cc = 1 << CONTROL,
                Cf = 1 << FORMAT,
                Co = 1 << PRIVATE_USE,
                Cs = 1 << SURROGATE,
                Pd = 1 << DASH_PUNCTUATION,
                Ps = 1 << START_PUNCTUATION,
                Pe = 1 << END_PUNCTUATION,
                Pc = 1 << CONNECTOR_PUNCTUATION,
                Po = 1 << OTHER_PUNCTUATION,
                Sm = 1 << MATH_SYMBOL,
                Sc = 1 << CURRENCY_SYMBOL,
                Sk = 1 << MODIFIER_SYMBOL,
                So = 1 << OTHER_SYMBOL,
                Pi = 1 << INITIAL_QUOTE_PUNCTUATION,
                Pf = 1 << FINAL_QUOTE_PUNCTUATION
            };

            // unicode bidirectional constant
            static const int kDirectionalityUndefined = -1;
            static const int kDirectionalityLeftToRight = 0;// L
            static const int kDirectionalityRightToLeft = 1;// R
            static const int kDirectionalityRightToLeftArabic = 2;// AL
            static const int kDirectionalityEnuropeanNumber = 3;// EN
            static const int kDirectionalityEnuropeanNumberSeparator = 4;// ES
            static const int kDirectionalityEnuropeanNumberTerminator = 5;// ET
            static const int kDirectionalityEnuropeanArabicNumber = 6;// AN
            static const int kDirectionalityCommonNumberSeparator = 7;// CS
            static const int kDirectionalityNonspacingMark = 8;// NSM
            static const int kDirectionalityBoundaryNeutral = 9;// BN
            static const int kDirectionalityParagraphSeparator = 10;// B
            static const int kDirectionalitySegmentSeparator = 11;// S
            static const int kDirectionalityWhiteSpace = 12;// WS
            static const int kDirectionalityOtherNeutrals = 13;// ON
            static const int kDirectionalityLeftToRightEmbedding = 14;// LRE
            static const int kDirectionalityLeftToRightOverride = 15;// LRO
            static const int kDirectionalityRightToLeftEmbedding = 16;// RLE
            static const int kDirectionalityRightToLeftOverride = 17;// RLO
            static const int kDirectionalityPopDirectionalFormat = 18;// PDF

            static const int kSize = 16;

            static const std::array<uint8_t, 1000> kCategoryCache
            = { { 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
                  15, 15, 15, 15, 15, 15, 15, 12, 24, 24, 24, 26, 24, 24, 24, 21, 22, 24, 25, 24, 20, 24, 24, 9,  9,
                  9,  9,  9,  9,  9,  9,  9,  9,  24, 24, 25, 25, 25, 24, 24, 1,  1,  1,  1,  1,  1,  1,  1,  1,  1,
                  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  21, 24, 22, 27, 23, 27, 2,  2,  2,
                  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  21, 25,
                  22, 25, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
                  15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 12, 24, 26, 26, 26, 26, 28, 28, 27, 28, 2,  29, 25, 16, 28,
                  27, 28, 25, 11, 11, 27, 2,  28, 24, 27, 11, 2,  30, 11, 11, 11, 24, 1,  1,  1,  1,  1,  1,  1,  1,
                  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  25, 1,  1,  1,  1,  1,  1,  1,  2,  2,
                  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  25, 2,  2,
                  2,  2,  2,  2,  2,  2,  1,  2,  1,  2,  1,  2,  1,  2,  1,  2,  1,  2,  1,  2,  1,  2,  1,  2,  1,
                  2,  1,  2,  1,  2,  1,  2,  1,  2,  1,  2,  1,  2,  1,  2,  1,  2,  1,  2,  1,  2,  1,  2,  1,  2,
                  1,  2,  1,  2,  1,  2,  1,  2,  1,  2,  1,  2,  2,  1,  2,  1,  2,  1,  2,  1,  2,  1,  2,  1,  2,
                  1,  2,  1,  2,  2,  1,  2,  1,  2,  1,  2,  1,  2,  1,  2,  1,  2,  1,  2,  1,  2,  1,  2,  1,  2,
                  1,  2,  1,  2,  1,  2,  1,  2,  1,  2,  1,  2,  1,  2,  1,  2,  1,  2,  1,  2,  1,  2,  1,  2,  1,
                  2,  1,  1,  2,  1,  2,  1,  2,  2,  2,  1,  1,  2,  1,  2,  1,  1,  2,  1,  1,  1,  2,  2,  1,  1,
                  1,  1,  2,  1,  1,  2,  1,  1,  1,  2,  2,  2,  1,  1,  2,  1,  1,  2,  1,  2,  1,  2,  1,  1,  2,
                  1,  2,  2,  1,  2,  1,  1,  2,  1,  1,  1,  2,  1,  2,  1,  1,  2,  2,  5,  1,  2,  2,  2,  5,  5,
                  5,  5,  1,  3,  2,  1,  3,  2,  1,  3,  2,  1,  2,  1,  2,  1,  2,  1,  2,  1,  2,  1,  2,  1,  2,
                  1,  2,  2,  1,  2,  1,  2,  1,  2,  1,  2,  1,  2,  1,  2,  1,  2,  1,  2,  1,  2,  2,  1,  3,  2,
                  1,  2,  1,  1,  1,  2,  1,  2,  1,  2,  1,  2,  1,  2,  1,  2,  1,  2,  1,  2,  1,  2,  1,  2,  1,
                  2,  1,  2,  1,  2,  1,  2,  1,  2,  1,  2,  1,  2,  1,  2,  1,  2,  1,  2,  1,  2,  1,  2,  1,  2,
                  1,  2,  1,  2,  1,  2,  1,  2,  1,  2,  1,  2,  1,  2,  2,  2,  2,  2,  2,  2,  1,  1,  2,  1,  1,
                  2,  2,  1,  2,  1,  1,  1,  1,  2,  1,  2,  1,  2,  1,  2,  1,  2,  2,  2,  2,  2,  2,  2,  2,  2,
                  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,
                  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,
                  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  5,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,
                  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
                  4,  4,  4,  4,  4,  4,  27, 27, 27, 27, 4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  27, 27, 27,
                  27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 4,  4,  4,  4,  4,  27, 27, 27, 27, 27, 27, 27, 4,  27,
                  4,  27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 6,  6,  6,  6,  6,  6,  6,
                  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,
                  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,
                  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,
                  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,  6,
                  6,  6,  6,  6,  6,  1,  2,  1,  2,  4,  27, 1,  2,  0,  0,  4,  2,  2,  2,  24, 0,  0,  0,  0,  0,
                  27, 27, 1,  24, 1,  1,  1,  0,  1,  0,  1,  1,  2,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,
                  1,  1,  1,  1,  1,  0,  1,  1,  1,  1,  1,  1,  1,  1,  1,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,
                  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,  2,
                  1,  2,  2,  1,  1,  1,  2,  2,  2,  1,  2,  1,  2,  1,  2,  1,  2,  1,  2,  1,  2,  1,  2,  1,  2 } };

            static const std::array<char16_t, 1327> kCategoryKeys
            = { { 0x3E8,  0x3EF,  0x3F4,  0x3F5,  0x3F6,  0x3F7,  0x3F8,  0x3F9,  0x3FB,  0x3FD,  0x430,  0x460,
                  0x461,  0x462,  0x463,  0x464,  0x465,  0x466,  0x467,  0x468,  0x469,  0x46A,  0x46B,  0x46C,
                  0x46D,  0x46E,  0x46F,  0x470,  0x471,  0x472,  0x473,  0x474,  0x475,  0x476,  0x477,  0x478,
                  0x479,  0x47A,  0x47B,  0x47C,  0x47D,  0x47E,  0x47F,  0x480,  0x481,  0x482,  0x483,  0x488,
                  0x48A,  0x4C0,  0x4C2,  0x4CE,  0x4D0,  0x531,  0x559,  0x55A,  0x561,  0x589,  0x591,  0x5BE,
                  0x5BF,  0x5C0,  0x5C1,  0x5C3,  0x5C4,  0x5C6,  0x5D0,  0x5F0,  0x5F3,  0x600,  0x606,  0x609,
                  0x60B,  0x60C,  0x60E,  0x610,  0x61B,  0x61E,  0x620,  0x640,  0x641,  0x64B,  0x660,  0x66A,
                  0x66E,  0x670,  0x671,  0x6D4,  0x6D6,  0x6DD,  0x6DF,  0x6E5,  0x6E7,  0x6E9,  0x6EA,  0x6EE,
                  0x6F0,  0x6FA,  0x6FD,  0x6FF,  0x700,  0x70F,  0x710,  0x711,  0x712,  0x730,  0x74D,  0x7A6,
                  0x7B1,  0x7C0,  0x7CA,  0x7EB,  0x7F4,  0x7F6,  0x7F7,  0x7FA,  0x800,  0x816,  0x81A,  0x81B,
                  0x824,  0x825,  0x828,  0x829,  0x830,  0x840,  0x859,  0x85E,  0x900,  0x903,  0x904,  0x93A,
                  0x93B,  0x93C,  0x93D,  0x93E,  0x941,  0x949,  0x94D,  0x94E,  0x950,  0x951,  0x958,  0x962,
                  0x964,  0x966,  0x970,  0x972,  0x979,  0x981,  0x982,  0x985,  0x98F,  0x993,  0x9AA,  0x9B2,
                  0x9B6,  0x9BC,  0x9BE,  0x9C1,  0x9C7,  0x9CB,  0x9CD,  0x9D7,  0x9DC,  0x9DF,  0x9E2,  0x9E6,
                  0x9F0,  0x9F2,  0x9F4,  0x9FA,  0xA01,  0xA03,  0xA05,  0xA0F,  0xA13,  0xA2A,  0xA32,  0xA35,
                  0xA38,  0xA3C,  0xA3E,  0xA41,  0xA47,  0xA4B,  0xA51,  0xA59,  0xA5E,  0xA66,  0xA70,  0xA72,
                  0xA75,  0xA81,  0xA83,  0xA85,  0xA8F,  0xA93,  0xAAA,  0xAB2,  0xAB5,  0xABC,  0xABE,  0xAC1,
                  0xAC7,  0xAC9,  0xACB,  0xACD,  0xAD0,  0xAE0,  0xAE2,  0xAE6,  0xAF1,  0xB01,  0xB02,  0xB05,
                  0xB0F,  0xB13,  0xB2A,  0xB32,  0xB35,  0xB3C,  0xB3D,  0xB3E,  0xB3F,  0xB40,  0xB41,  0xB47,
                  0xB4B,  0xB4D,  0xB56,  0xB5C,  0xB5F,  0xB62,  0xB66,  0xB70,  0xB72,  0xB82,  0xB85,  0xB8E,
                  0xB92,  0xB99,  0xB9C,  0xB9E,  0xBA3,  0xBA8,  0xBAE,  0xBBE,  0xBC0,  0xBC1,  0xBC6,  0xBCA,
                  0xBCD,  0xBD0,  0xBD7,  0xBE6,  0xBF0,  0xBF3,  0xBF9,  0xC01,  0xC05,  0xC0E,  0xC12,  0xC2A,
                  0xC35,  0xC3D,  0xC3E,  0xC41,  0xC46,  0xC4A,  0xC55,  0xC58,  0xC60,  0xC62,  0xC66,  0xC78,
                  0xC7F,  0xC82,  0xC85,  0xC8E,  0xC92,  0xCAA,  0xCB5,  0xCBC,  0xCBD,  0xCBE,  0xCBF,  0xCC0,
                  0xCC6,  0xCC7,  0xCCA,  0xCCC,  0xCD5,  0xCDE,  0xCE0,  0xCE2,  0xCE6,  0xCF1,  0xD02,  0xD05,
                  0xD0E,  0xD12,  0xD3D,  0xD3E,  0xD41,  0xD46,  0xD4A,  0xD4D,  0xD57,  0xD60,  0xD62,  0xD66,
                  0xD70,  0xD79,  0xD7A,  0xD82,  0xD85,  0xD9A,  0xDB3,  0xDBD,  0xDC0,  0xDCA,  0xDCF,  0xDD2,
                  0xDD6,  0xDD8,  0xDF2,  0xDF4,  0xE01,  0xE31,  0xE32,  0xE34,  0xE3F,  0xE40,  0xE46,  0xE47,
                  0xE4F,  0xE50,  0xE5A,  0xE81,  0xE84,  0xE87,  0xE8A,  0xE8D,  0xE94,  0xE99,  0xEA1,  0xEA5,
                  0xEA7,  0xEAA,  0xEAD,  0xEB1,  0xEB2,  0xEB4,  0xEBB,  0xEBD,  0xEC0,  0xEC6,  0xEC8,  0xED0,
                  0xEDC,  0xF00,  0xF01,  0xF04,  0xF13,  0xF18,  0xF1A,  0xF20,  0xF2A,  0xF34,  0xF35,  0xF36,
                  0xF37,  0xF38,  0xF39,  0xF3A,  0xF3B,  0xF3C,  0xF3D,  0xF3E,  0xF40,  0xF49,  0xF71,  0xF7F,
                  0xF80,  0xF85,  0xF86,  0xF88,  0xF8D,  0xF99,  0xFBE,  0xFC6,  0xFC7,  0xFCE,  0xFD0,  0xFD5,
                  0xFD9,  0x1000, 0x102B, 0x102D, 0x1031, 0x1032, 0x1038, 0x1039, 0x103B, 0x103D, 0x103F, 0x1040,
                  0x104A, 0x1050, 0x1056, 0x1058, 0x105A, 0x105E, 0x1061, 0x1062, 0x1065, 0x1067, 0x106E, 0x1071,
                  0x1075, 0x1082, 0x1083, 0x1085, 0x1087, 0x108D, 0x108E, 0x108F, 0x1090, 0x109A, 0x109D, 0x109E,
                  0x10A0, 0x10D0, 0x10FB, 0x1100, 0x124A, 0x1250, 0x1258, 0x125A, 0x1260, 0x128A, 0x1290, 0x12B2,
                  0x12B8, 0x12C0, 0x12C2, 0x12C8, 0x12D8, 0x1312, 0x1318, 0x135D, 0x1360, 0x1361, 0x1369, 0x1380,
                  0x1390, 0x13A0, 0x1400, 0x1401, 0x166D, 0x166F, 0x1680, 0x1681, 0x169B, 0x16A0, 0x16EB, 0x16EE,
                  0x1700, 0x170E, 0x1712, 0x1720, 0x1732, 0x1735, 0x1740, 0x1752, 0x1760, 0x176E, 0x1772, 0x1780,
                  0x17B4, 0x17B6, 0x17B7, 0x17BE, 0x17C6, 0x17C7, 0x17C9, 0x17D4, 0x17D7, 0x17D8, 0x17DB, 0x17DC,
                  0x17DD, 0x17E0, 0x17F0, 0x1800, 0x1806, 0x1807, 0x180B, 0x180E, 0x1810, 0x1820, 0x1843, 0x1844,
                  0x1880, 0x18A9, 0x18B0, 0x1900, 0x1920, 0x1923, 0x1927, 0x1929, 0x1930, 0x1932, 0x1933, 0x1939,
                  0x1940, 0x1944, 0x1946, 0x1950, 0x1970, 0x1980, 0x19B0, 0x19C1, 0x19C8, 0x19D0, 0x19DA, 0x19DE,
                  0x1A00, 0x1A17, 0x1A19, 0x1A1E, 0x1A20, 0x1A55, 0x1A58, 0x1A60, 0x1A63, 0x1A65, 0x1A6D, 0x1A73,
                  0x1A7F, 0x1A80, 0x1A90, 0x1AA0, 0x1AA7, 0x1AA8, 0x1B00, 0x1B04, 0x1B05, 0x1B34, 0x1B36, 0x1B3B,
                  0x1B3D, 0x1B42, 0x1B43, 0x1B45, 0x1B50, 0x1B5A, 0x1B61, 0x1B6B, 0x1B74, 0x1B80, 0x1B82, 0x1B83,
                  0x1BA1, 0x1BA2, 0x1BA6, 0x1BA8, 0x1BAA, 0x1BAE, 0x1BB0, 0x1BC0, 0x1BE6, 0x1BE8, 0x1BEA, 0x1BED,
                  0x1BEF, 0x1BF2, 0x1BFC, 0x1C00, 0x1C24, 0x1C2C, 0x1C34, 0x1C36, 0x1C3B, 0x1C40, 0x1C4D, 0x1C50,
                  0x1C5A, 0x1C78, 0x1C7E, 0x1CD0, 0x1CD3, 0x1CD4, 0x1CE1, 0x1CE2, 0x1CE9, 0x1CED, 0x1CEE, 0x1CF2,
                  0x1D00, 0x1D2C, 0x1D62, 0x1D78, 0x1D79, 0x1D9B, 0x1DC0, 0x1DFC, 0x1E00, 0x1E95, 0x1E9E, 0x1EFF,
                  0x1F08, 0x1F10, 0x1F18, 0x1F20, 0x1F28, 0x1F30, 0x1F38, 0x1F40, 0x1F48, 0x1F50, 0x1F59, 0x1F5B,
                  0x1F5D, 0x1F5F, 0x1F60, 0x1F68, 0x1F70, 0x1F80, 0x1F88, 0x1F90, 0x1F98, 0x1FA0, 0x1FA8, 0x1FB0,
                  0x1FB6, 0x1FB8, 0x1FBC, 0x1FBD, 0x1FBE, 0x1FBF, 0x1FC2, 0x1FC6, 0x1FC8, 0x1FCC, 0x1FCD, 0x1FD0,
                  0x1FD6, 0x1FD8, 0x1FDD, 0x1FE0, 0x1FE8, 0x1FED, 0x1FF2, 0x1FF6, 0x1FF8, 0x1FFC, 0x1FFD, 0x2000,
                  0x200B, 0x2010, 0x2016, 0x2018, 0x2019, 0x201A, 0x201B, 0x201D, 0x201E, 0x201F, 0x2020, 0x2028,
                  0x202A, 0x202F, 0x2030, 0x2039, 0x203B, 0x203F, 0x2041, 0x2044, 0x2045, 0x2046, 0x2047, 0x2052,
                  0x2053, 0x2054, 0x2055, 0x205F, 0x2060, 0x206A, 0x2070, 0x2074, 0x207A, 0x207D, 0x207E, 0x207F,
                  0x2080, 0x208A, 0x208D, 0x2090, 0x20A0, 0x20D0, 0x20DD, 0x20E1, 0x20E2, 0x20E5, 0x2100, 0x2102,
                  0x2103, 0x2107, 0x2108, 0x210A, 0x210B, 0x210E, 0x2110, 0x2113, 0x2114, 0x2115, 0x2116, 0x2118,
                  0x2119, 0x211E, 0x2124, 0x212A, 0x212E, 0x2130, 0x2134, 0x2135, 0x2139, 0x213A, 0x213C, 0x213E,
                  0x2140, 0x2145, 0x2146, 0x214A, 0x214C, 0x214E, 0x2150, 0x2160, 0x2183, 0x2185, 0x2189, 0x2190,
                  0x2195, 0x219A, 0x219C, 0x21A0, 0x21A1, 0x21A3, 0x21A4, 0x21A6, 0x21A7, 0x21AE, 0x21AF, 0x21CE,
                  0x21D0, 0x21D2, 0x21D5, 0x21F4, 0x2300, 0x2308, 0x230C, 0x2320, 0x2322, 0x2329, 0x232B, 0x237C,
                  0x237D, 0x239B, 0x23B4, 0x23DC, 0x23E2, 0x2400, 0x2440, 0x2460, 0x249C, 0x24EA, 0x2500, 0x25B7,
                  0x25B8, 0x25C1, 0x25C2, 0x25F8, 0x2600, 0x266F, 0x2670, 0x2701, 0x2768, 0x2776, 0x2794, 0x27C0,
                  0x27C5, 0x27C7, 0x27CC, 0x27CE, 0x27E6, 0x27F0, 0x2800, 0x2900, 0x2983, 0x2999, 0x29D8, 0x29DC,
                  0x29FC, 0x29FE, 0x2B00, 0x2B30, 0x2B45, 0x2B47, 0x2B50, 0x2C00, 0x2C30, 0x2C60, 0x2C62, 0x2C65,
                  0x2C67, 0x2C6D, 0x2C71, 0x2C73, 0x2C75, 0x2C76, 0x2C7D, 0x2C7E, 0x2C81, 0x2CE3, 0x2CE5, 0x2CEB,
                  0x2CEF, 0x2CF9, 0x2CFD, 0x2CFE, 0x2D00, 0x2D30, 0x2D6F, 0x2D7F, 0x2D80, 0x2DA0, 0x2DA8, 0x2DB0,
                  0x2DB8, 0x2DC0, 0x2DC8, 0x2DD0, 0x2DD8, 0x2DE0, 0x2E00, 0x2E02, 0x2E06, 0x2E09, 0x2E0A, 0x2E0B,
                  0x2E0C, 0x2E0D, 0x2E0E, 0x2E17, 0x2E18, 0x2E1A, 0x2E1B, 0x2E1C, 0x2E1D, 0x2E1E, 0x2E20, 0x2E21,
                  0x2E22, 0x2E23, 0x2E24, 0x2E25, 0x2E26, 0x2E27, 0x2E28, 0x2E29, 0x2E2A, 0x2E2F, 0x2E30, 0x2E80,
                  0x2E9B, 0x2F00, 0x2FF0, 0x3000, 0x3001, 0x3004, 0x3005, 0x3006, 0x3007, 0x3008, 0x3009, 0x300A,
                  0x300B, 0x300C, 0x300D, 0x300E, 0x300F, 0x3010, 0x3011, 0x3012, 0x3014, 0x3015, 0x3016, 0x3017,
                  0x3018, 0x3019, 0x301A, 0x301B, 0x301C, 0x301D, 0x301E, 0x3020, 0x3021, 0x302A, 0x3030, 0x3031,
                  0x3036, 0x3038, 0x303B, 0x303C, 0x303D, 0x303E, 0x3041, 0x3099, 0x309B, 0x309D, 0x309F, 0x30A1,
                  0x30FB, 0x30FC, 0x30FF, 0x3105, 0x3131, 0x3190, 0x3192, 0x3196, 0x31A0, 0x31C0, 0x31F0, 0x3200,
                  0x3220, 0x322A, 0x3251, 0x3260, 0x3280, 0x328A, 0x32B1, 0x32C0, 0x3300, 0x3400, 0x4DC0, 0x4E00,
                  0xA000, 0xA015, 0xA016, 0xA490, 0xA4D0, 0xA4F8, 0xA4FE, 0xA500, 0xA60C, 0xA60D, 0xA610, 0xA620,
                  0xA62A, 0xA640, 0xA641, 0xA642, 0xA643, 0xA644, 0xA645, 0xA646, 0xA647, 0xA648, 0xA649, 0xA64A,
                  0xA64B, 0xA64C, 0xA64D, 0xA64E, 0xA64F, 0xA650, 0xA651, 0xA652, 0xA653, 0xA654, 0xA655, 0xA656,
                  0xA657, 0xA658, 0xA659, 0xA65A, 0xA65B, 0xA65C, 0xA65D, 0xA65E, 0xA65F, 0xA660, 0xA661, 0xA662,
                  0xA663, 0xA664, 0xA665, 0xA666, 0xA667, 0xA668, 0xA669, 0xA66A, 0xA66B, 0xA66C, 0xA66D, 0xA66E,
                  0xA66F, 0xA670, 0xA673, 0xA67C, 0xA67E, 0xA67F, 0xA680, 0xA681, 0xA682, 0xA683, 0xA684, 0xA685,
                  0xA686, 0xA687, 0xA688, 0xA689, 0xA68A, 0xA68B, 0xA68C, 0xA68D, 0xA68E, 0xA68F, 0xA690, 0xA691,
                  0xA692, 0xA693, 0xA694, 0xA695, 0xA696, 0xA697, 0xA6A0, 0xA6E6, 0xA6F0, 0xA6F2, 0xA700, 0xA717,
                  0xA720, 0xA722, 0xA72F, 0xA732, 0xA733, 0xA734, 0xA735, 0xA736, 0xA737, 0xA738, 0xA739, 0xA73A,
                  0xA73B, 0xA73C, 0xA73D, 0xA73E, 0xA73F, 0xA740, 0xA741, 0xA742, 0xA743, 0xA744, 0xA745, 0xA746,
                  0xA747, 0xA748, 0xA749, 0xA74A, 0xA74B, 0xA74C, 0xA74D, 0xA74E, 0xA74F, 0xA750, 0xA751, 0xA752,
                  0xA753, 0xA754, 0xA755, 0xA756, 0xA757, 0xA758, 0xA759, 0xA75A, 0xA75B, 0xA75C, 0xA75D, 0xA75E,
                  0xA75F, 0xA760, 0xA761, 0xA762, 0xA763, 0xA764, 0xA765, 0xA766, 0xA767, 0xA768, 0xA769, 0xA76A,
                  0xA76B, 0xA76C, 0xA76D, 0xA76E, 0xA76F, 0xA770, 0xA771, 0xA779, 0xA77D, 0xA77F, 0xA780, 0xA781,
                  0xA782, 0xA783, 0xA784, 0xA785, 0xA786, 0xA787, 0xA788, 0xA789, 0xA78B, 0xA790, 0xA7A0, 0xA7FA,
                  0xA7FB, 0xA802, 0xA803, 0xA806, 0xA807, 0xA80B, 0xA80C, 0xA823, 0xA825, 0xA827, 0xA828, 0xA830,
                  0xA836, 0xA838, 0xA840, 0xA874, 0xA880, 0xA882, 0xA8B4, 0xA8C4, 0xA8CE, 0xA8D0, 0xA8E0, 0xA8F2,
                  0xA8F8, 0xA8FB, 0xA900, 0xA90A, 0xA926, 0xA92E, 0xA930, 0xA947, 0xA952, 0xA95F, 0xA960, 0xA980,
                  0xA983, 0xA984, 0xA9B3, 0xA9B4, 0xA9B6, 0xA9BA, 0xA9BC, 0xA9BD, 0xA9C1, 0xA9CF, 0xA9D0, 0xA9DE,
                  0xAA00, 0xAA29, 0xAA2F, 0xAA31, 0xAA33, 0xAA35, 0xAA40, 0xAA43, 0xAA44, 0xAA4C, 0xAA50, 0xAA5C,
                  0xAA60, 0xAA70, 0xAA71, 0xAA77, 0xAA7A, 0xAA80, 0xAAB0, 0xAAB2, 0xAAB5, 0xAAB7, 0xAAB9, 0xAABE,
                  0xAAC0, 0xAADB, 0xAADD, 0xAADE, 0xAB01, 0xAB09, 0xAB11, 0xAB20, 0xAB28, 0xABC0, 0xABE3, 0xABE5,
                  0xABE6, 0xABE8, 0xABE9, 0xABEB, 0xABEC, 0xABED, 0xABF0, 0xAC00, 0xD7B0, 0xD7CB, 0xD800, 0xE000,
                  0xF900, 0xFA30, 0xFA70, 0xFB00, 0xFB13, 0xFB1D, 0xFB1F, 0xFB29, 0xFB2A, 0xFB38, 0xFB3E, 0xFB40,
                  0xFB43, 0xFB46, 0xFBB2, 0xFBD3, 0xFD3E, 0xFD50, 0xFD92, 0xFDF0, 0xFDFC, 0xFE00, 0xFE10, 0xFE17,
                  0xFE18, 0xFE19, 0xFE20, 0xFE30, 0xFE31, 0xFE33, 0xFE35, 0xFE45, 0xFE47, 0xFE49, 0xFE4D, 0xFE50,
                  0xFE54, 0xFE58, 0xFE59, 0xFE5A, 0xFE5B, 0xFE5C, 0xFE5D, 0xFE5E, 0xFE5F, 0xFE62, 0xFE64, 0xFE68,
                  0xFE6A, 0xFE70, 0xFE76, 0xFEFF, 0xFF01, 0xFF04, 0xFF05, 0xFF08, 0xFF09, 0xFF0A, 0xFF0B, 0xFF0C,
                  0xFF0D, 0xFF0E, 0xFF10, 0xFF1A, 0xFF1C, 0xFF1F, 0xFF21, 0xFF3B, 0xFF3C, 0xFF3D, 0xFF3E, 0xFF3F,
                  0xFF40, 0xFF41, 0xFF5B, 0xFF5C, 0xFF5D, 0xFF5E, 0xFF5F, 0xFF60, 0xFF61, 0xFF62, 0xFF63, 0xFF64,
                  0xFF66, 0xFF70, 0xFF71, 0xFF9E, 0xFFA0, 0xFFC2, 0xFFCA, 0xFFD2, 0xFFDA, 0xFFE0, 0xFFE2, 0xFFE3,
                  0xFFE4, 0xFFE5, 0xFFE8, 0xFFE9, 0xFFED, 0xFFF9, 0xFFFC } };

            static const std::array<char16_t, 2654> kCategoryValues
            = { { 0x3EE,  0x201,  0x3F3,  0x2,    0x3F4,  0x1,    0x3F5,  0x2,    0x3F6,  0x19,   0x3F7,  0x1,
                  0x3F8,  0x2,    0x3FA,  0x1,    0x3FC,  0x2,    0x42F,  0x1,    0x45F,  0x2,    0x460,  0x1,
                  0x461,  0x2,    0x462,  0x1,    0x463,  0x2,    0x464,  0x1,    0x465,  0x2,    0x466,  0x1,
                  0x467,  0x2,    0x468,  0x1,    0x469,  0x2,    0x46A,  0x1,    0x46B,  0x2,    0x46C,  0x1,
                  0x46D,  0x2,    0x46E,  0x1,    0x46F,  0x2,    0x470,  0x1,    0x471,  0x2,    0x472,  0x1,
                  0x473,  0x2,    0x474,  0x1,    0x475,  0x2,    0x476,  0x1,    0x477,  0x2,    0x478,  0x1,
                  0x479,  0x2,    0x47A,  0x1,    0x47B,  0x2,    0x47C,  0x1,    0x47D,  0x2,    0x47E,  0x1,
                  0x47F,  0x2,    0x480,  0x1,    0x481,  0x2,    0x482,  0x1C,   0x487,  0x6,    0x489,  0x7,
                  0x4BF,  0x201,  0x4C1,  0x1,    0x4CD,  0x102,  0x4CF,  0x2,    0x527,  0x201,  0x556,  0x1,
                  0x559,  0x4,    0x55F,  0x18,   0x587,  0x2,    0x58A,  0x1814, 0x5BD,  0x6,    0x5BE,  0x14,
                  0x5BF,  0x6,    0x5C0,  0x18,   0x5C2,  0x6,    0x5C3,  0x18,   0x5C5,  0x6,    0x5C7,  0x618,
                  0x5EA,  0x5,    0x5F2,  0x5,    0x5F4,  0x18,   0x603,  0x10,   0x608,  0x19,   0x60A,  0x18,
                  0x60B,  0x1A,   0x60D,  0x18,   0x60F,  0x1C,   0x61A,  0x6,    0x61B,  0x18,   0x61F,  0x18,
                  0x63F,  0x5,    0x640,  0x4,    0x64A,  0x5,    0x65F,  0x6,    0x669,  0x9,    0x66D,  0x18,
                  0x66F,  0x5,    0x670,  0x6,    0x6D3,  0x5,    0x6D5,  0x518,  0x6DC,  0x6,    0x6DE,  0x101C,
                  0x6E4,  0x6,    0x6E6,  0x4,    0x6E8,  0x6,    0x6E9,  0x1C,   0x6ED,  0x6,    0x6EF,  0x5,
                  0x6F9,  0x9,    0x6FC,  0x5,    0x6FE,  0x1C,   0x6FF,  0x5,    0x70D,  0x18,   0x70F,  0x10,
                  0x710,  0x5,    0x711,  0x6,    0x72F,  0x5,    0x74A,  0x6,    0x7A5,  0x5,    0x7B0,  0x6,
                  0x7B1,  0x5,    0x7C9,  0x9,    0x7EA,  0x5,    0x7F3,  0x6,    0x7F5,  0x4,    0x7F6,  0x1C,
                  0x7F9,  0x18,   0x7FA,  0x4,    0x815,  0x5,    0x819,  0x6,    0x81A,  0x4,    0x823,  0x6,
                  0x824,  0x4,    0x827,  0x6,    0x828,  0x4,    0x82D,  0x6,    0x83E,  0x18,   0x858,  0x5,
                  0x85B,  0x6,    0x85E,  0x18,   0x902,  0x6,    0x903,  0x8,    0x939,  0x5,    0x93A,  0x6,
                  0x93B,  0x8,    0x93C,  0x6,    0x93D,  0x5,    0x940,  0x8,    0x948,  0x6,    0x94C,  0x8,
                  0x94D,  0x6,    0x94F,  0x8,    0x950,  0x5,    0x957,  0x6,    0x961,  0x5,    0x963,  0x6,
                  0x965,  0x18,   0x96F,  0x9,    0x971,  0x418,  0x977,  0x5,    0x97F,  0x5,    0x981,  0x6,
                  0x983,  0x8,    0x98C,  0x5,    0x990,  0x5,    0x9A8,  0x5,    0x9B0,  0x5,    0x9B2,  0x5,
                  0x9B9,  0x5,    0x9BD,  0x506,  0x9C0,  0x8,    0x9C4,  0x6,    0x9C8,  0x8,    0x9CC,  0x8,
                  0x9CE,  0x605,  0x9D7,  0x8,    0x9DD,  0x5,    0x9E1,  0x5,    0x9E3,  0x6,    0x9EF,  0x9,
                  0x9F1,  0x5,    0x9F3,  0x1A,   0x9F9,  0xB,    0x9FB,  0x1A1C, 0xA02,  0x6,    0xA03,  0x8,
                  0xA0A,  0x5,    0xA10,  0x5,    0xA28,  0x5,    0xA30,  0x5,    0xA33,  0x5,    0xA36,  0x5,
                  0xA39,  0x5,    0xA3C,  0x6,    0xA40,  0x8,    0xA42,  0x6,    0xA48,  0x6,    0xA4D,  0x6,
                  0xA51,  0x6,    0xA5C,  0x5,    0xA5E,  0x5,    0xA6F,  0x9,    0xA71,  0x6,    0xA74,  0x5,
                  0xA75,  0x6,    0xA82,  0x6,    0xA83,  0x8,    0xA8D,  0x5,    0xA91,  0x5,    0xAA8,  0x5,
                  0xAB0,  0x5,    0xAB3,  0x5,    0xAB9,  0x5,    0xABD,  0x506,  0xAC0,  0x8,    0xAC5,  0x6,
                  0xAC8,  0x6,    0xAC9,  0x8,    0xACC,  0x8,    0xACD,  0x6,    0xAD0,  0x5,    0xAE1,  0x5,
                  0xAE3,  0x6,    0xAEF,  0x9,    0xAF1,  0x1A,   0xB01,  0x6,    0xB03,  0x8,    0xB0C,  0x5,
                  0xB10,  0x5,    0xB28,  0x5,    0xB30,  0x5,    0xB33,  0x5,    0xB39,  0x5,    0xB3C,  0x6,
                  0xB3D,  0x5,    0xB3E,  0x8,    0xB3F,  0x6,    0xB40,  0x8,    0xB44,  0x6,    0xB48,  0x8,
                  0xB4C,  0x8,    0xB4D,  0x6,    0xB57,  0x806,  0xB5D,  0x5,    0xB61,  0x5,    0xB63,  0x6,
                  0xB6F,  0x9,    0xB71,  0x51C,  0xB77,  0xB,    0xB83,  0x506,  0xB8A,  0x5,    0xB90,  0x5,
                  0xB95,  0x5,    0xB9A,  0x5,    0xB9C,  0x5,    0xB9F,  0x5,    0xBA4,  0x5,    0xBAA,  0x5,
                  0xBB9,  0x5,    0xBBF,  0x8,    0xBC0,  0x6,    0xBC2,  0x8,    0xBC8,  0x8,    0xBCC,  0x8,
                  0xBCD,  0x6,    0xBD0,  0x5,    0xBD7,  0x8,    0xBEF,  0x9,    0xBF2,  0xB,    0xBF8,  0x1C,
                  0xBFA,  0x1A1C, 0xC03,  0x8,    0xC0C,  0x5,    0xC10,  0x5,    0xC28,  0x5,    0xC33,  0x5,
                  0xC39,  0x5,    0xC3D,  0x5,    0xC40,  0x6,    0xC44,  0x8,    0xC48,  0x6,    0xC4D,  0x6,
                  0xC56,  0x6,    0xC59,  0x5,    0xC61,  0x5,    0xC63,  0x6,    0xC6F,  0x9,    0xC7E,  0xB,
                  0xC7F,  0x1C,   0xC83,  0x8,    0xC8C,  0x5,    0xC90,  0x5,    0xCA8,  0x5,    0xCB3,  0x5,
                  0xCB9,  0x5,    0xCBC,  0x6,    0xCBD,  0x5,    0xCBE,  0x8,    0xCBF,  0x6,    0xCC4,  0x8,
                  0xCC6,  0x6,    0xCC8,  0x8,    0xCCB,  0x8,    0xCCD,  0x6,    0xCD6,  0x8,    0xCDE,  0x5,
                  0xCE1,  0x5,    0xCE3,  0x6,    0xCEF,  0x9,    0xCF2,  0x5,    0xD03,  0x8,    0xD0C,  0x5,
                  0xD10,  0x5,    0xD3A,  0x5,    0xD3D,  0x5,    0xD40,  0x8,    0xD44,  0x6,    0xD48,  0x8,
                  0xD4C,  0x8,    0xD4E,  0x605,  0xD57,  0x8,    0xD61,  0x5,    0xD63,  0x6,    0xD6F,  0x9,
                  0xD75,  0xB,    0xD79,  0x1C,   0xD7F,  0x5,    0xD83,  0x8,    0xD96,  0x5,    0xDB1,  0x5,
                  0xDBB,  0x5,    0xDBD,  0x5,    0xDC6,  0x5,    0xDCA,  0x6,    0xDD1,  0x8,    0xDD4,  0x6,
                  0xDD6,  0x6,    0xDDF,  0x8,    0xDF3,  0x8,    0xDF4,  0x18,   0xE30,  0x5,    0xE31,  0x6,
                  0xE33,  0x5,    0xE3A,  0x6,    0xE3F,  0x1A,   0xE45,  0x5,    0xE46,  0x4,    0xE4E,  0x6,
                  0xE4F,  0x18,   0xE59,  0x9,    0xE5B,  0x18,   0xE82,  0x5,    0xE84,  0x5,    0xE88,  0x5,
                  0xE8A,  0x5,    0xE8D,  0x5,    0xE97,  0x5,    0xE9F,  0x5,    0xEA3,  0x5,    0xEA5,  0x5,
                  0xEA7,  0x5,    0xEAB,  0x5,    0xEB0,  0x5,    0xEB1,  0x6,    0xEB3,  0x5,    0xEB9,  0x6,
                  0xEBC,  0x6,    0xEBD,  0x5,    0xEC4,  0x5,    0xEC6,  0x4,    0xECD,  0x6,    0xED9,  0x9,
                  0xEDD,  0x5,    0xF00,  0x5,    0xF03,  0x1C,   0xF12,  0x18,   0xF17,  0x1C,   0xF19,  0x6,
                  0xF1F,  0x1C,   0xF29,  0x9,    0xF33,  0xB,    0xF34,  0x1C,   0xF35,  0x6,    0xF36,  0x1C,
                  0xF37,  0x6,    0xF38,  0x1C,   0xF39,  0x6,    0xF3A,  0x15,   0xF3B,  0x16,   0xF3C,  0x15,
                  0xF3D,  0x16,   0xF3F,  0x8,    0xF47,  0x5,    0xF6C,  0x5,    0xF7E,  0x6,    0xF7F,  0x8,
                  0xF84,  0x6,    0xF85,  0x18,   0xF87,  0x6,    0xF8C,  0x5,    0xF97,  0x6,    0xFBC,  0x6,
                  0xFC5,  0x1C,   0xFC6,  0x6,    0xFCC,  0x1C,   0xFCF,  0x1C,   0xFD4,  0x18,   0xFD8,  0x1C,
                  0xFDA,  0x18,   0x102A, 0x5,    0x102C, 0x8,    0x1030, 0x6,    0x1031, 0x8,    0x1037, 0x6,
                  0x1038, 0x8,    0x103A, 0x6,    0x103C, 0x8,    0x103E, 0x6,    0x103F, 0x5,    0x1049, 0x9,
                  0x104F, 0x18,   0x1055, 0x5,    0x1057, 0x8,    0x1059, 0x6,    0x105D, 0x5,    0x1060, 0x6,
                  0x1061, 0x5,    0x1064, 0x8,    0x1066, 0x5,    0x106D, 0x8,    0x1070, 0x5,    0x1074, 0x6,
                  0x1081, 0x5,    0x1082, 0x6,    0x1084, 0x8,    0x1086, 0x6,    0x108C, 0x8,    0x108D, 0x6,
                  0x108E, 0x5,    0x108F, 0x8,    0x1099, 0x9,    0x109C, 0x8,    0x109D, 0x6,    0x109F, 0x1C,
                  0x10C5, 0x1,    0x10FA, 0x5,    0x10FC, 0x1804, 0x1248, 0x5,    0x124D, 0x5,    0x1256, 0x5,
                  0x1258, 0x5,    0x125D, 0x5,    0x1288, 0x5,    0x128D, 0x5,    0x12B0, 0x5,    0x12B5, 0x5,
                  0x12BE, 0x5,    0x12C0, 0x5,    0x12C5, 0x5,    0x12D6, 0x5,    0x1310, 0x5,    0x1315, 0x5,
                  0x135A, 0x5,    0x135F, 0x6,    0x1360, 0x1C,   0x1368, 0x18,   0x137C, 0xB,    0x138F, 0x5,
                  0x1399, 0x1C,   0x13F4, 0x5,    0x1400, 0x14,   0x166C, 0x5,    0x166E, 0x18,   0x167F, 0x5,
                  0x1680, 0xC,    0x169A, 0x5,    0x169C, 0x1516, 0x16EA, 0x5,    0x16ED, 0x18,   0x16F0, 0xA,
                  0x170C, 0x5,    0x1711, 0x5,    0x1714, 0x6,    0x1731, 0x5,    0x1734, 0x6,    0x1736, 0x18,
                  0x1751, 0x5,    0x1753, 0x6,    0x176C, 0x5,    0x1770, 0x5,    0x1773, 0x6,    0x17B3, 0x5,
                  0x17B5, 0x10,   0x17B6, 0x8,    0x17BD, 0x6,    0x17C5, 0x8,    0x17C6, 0x6,    0x17C8, 0x8,
                  0x17D3, 0x6,    0x17D6, 0x18,   0x17D7, 0x4,    0x17DA, 0x18,   0x17DB, 0x1A,   0x17DC, 0x5,
                  0x17DD, 0x6,    0x17E9, 0x9,    0x17F9, 0xB,    0x1805, 0x18,   0x1806, 0x14,   0x180A, 0x18,
                  0x180D, 0x6,    0x180E, 0xC,    0x1819, 0x9,    0x1842, 0x5,    0x1843, 0x4,    0x1877, 0x5,
                  0x18A8, 0x5,    0x18AA, 0x605,  0x18F5, 0x5,    0x191C, 0x5,    0x1922, 0x6,    0x1926, 0x8,
                  0x1928, 0x6,    0x192B, 0x8,    0x1931, 0x8,    0x1932, 0x6,    0x1938, 0x8,    0x193B, 0x6,
                  0x1940, 0x1C,   0x1945, 0x18,   0x194F, 0x9,    0x196D, 0x5,    0x1974, 0x5,    0x19AB, 0x5,
                  0x19C0, 0x8,    0x19C7, 0x5,    0x19C9, 0x8,    0x19D9, 0x9,    0x19DA, 0xB,    0x19FF, 0x1C,
                  0x1A16, 0x5,    0x1A18, 0x6,    0x1A1B, 0x8,    0x1A1F, 0x18,   0x1A54, 0x5,    0x1A57, 0x806,
                  0x1A5E, 0x6,    0x1A62, 0x806,  0x1A64, 0x8,    0x1A6C, 0x6,    0x1A72, 0x8,    0x1A7C, 0x6,
                  0x1A7F, 0x6,    0x1A89, 0x9,    0x1A99, 0x9,    0x1AA6, 0x18,   0x1AA7, 0x4,    0x1AAD, 0x18,
                  0x1B03, 0x6,    0x1B04, 0x8,    0x1B33, 0x5,    0x1B35, 0x806,  0x1B3A, 0x6,    0x1B3C, 0x806,
                  0x1B41, 0x8,    0x1B42, 0x6,    0x1B44, 0x8,    0x1B4B, 0x5,    0x1B59, 0x9,    0x1B60, 0x18,
                  0x1B6A, 0x1C,   0x1B73, 0x6,    0x1B7C, 0x1C,   0x1B81, 0x6,    0x1B82, 0x8,    0x1BA0, 0x5,
                  0x1BA1, 0x8,    0x1BA5, 0x6,    0x1BA7, 0x8,    0x1BA9, 0x6,    0x1BAA, 0x8,    0x1BAF, 0x5,
                  0x1BB9, 0x9,    0x1BE5, 0x5,    0x1BE7, 0x806,  0x1BE9, 0x6,    0x1BEC, 0x8,    0x1BEE, 0x608,
                  0x1BF1, 0x6,    0x1BF3, 0x8,    0x1BFF, 0x18,   0x1C23, 0x5,    0x1C2B, 0x8,    0x1C33, 0x6,
                  0x1C35, 0x8,    0x1C37, 0x6,    0x1C3F, 0x18,   0x1C49, 0x9,    0x1C4F, 0x5,    0x1C59, 0x9,
                  0x1C77, 0x5,    0x1C7D, 0x4,    0x1C7F, 0x18,   0x1CD2, 0x6,    0x1CD3, 0x18,   0x1CE0, 0x6,
                  0x1CE1, 0x8,    0x1CE8, 0x6,    0x1CEC, 0x5,    0x1CED, 0x6,    0x1CF1, 0x5,    0x1CF2, 0x8,
                  0x1D2B, 0x2,    0x1D61, 0x4,    0x1D77, 0x2,    0x1D78, 0x4,    0x1D9A, 0x2,    0x1DBF, 0x4,
                  0x1DE6, 0x6,    0x1DFF, 0x6,    0x1E94, 0x201,  0x1E9D, 0x2,    0x1EFE, 0x201,  0x1F07, 0x2,
                  0x1F0F, 0x1,    0x1F15, 0x2,    0x1F1D, 0x1,    0x1F27, 0x2,    0x1F2F, 0x1,    0x1F37, 0x2,
                  0x1F3F, 0x1,    0x1F45, 0x2,    0x1F4D, 0x1,    0x1F57, 0x2,    0x1F59, 0x1,    0x1F5B, 0x1,
                  0x1F5D, 0x1,    0x1F5F, 0x1,    0x1F67, 0x2,    0x1F6F, 0x1,    0x1F7D, 0x2,    0x1F87, 0x2,
                  0x1F8F, 0x3,    0x1F97, 0x2,    0x1F9F, 0x3,    0x1FA7, 0x2,    0x1FAF, 0x3,    0x1FB4, 0x2,
                  0x1FB7, 0x2,    0x1FBB, 0x1,    0x1FBC, 0x3,    0x1FBD, 0x1B,   0x1FBE, 0x2,    0x1FC1, 0x1B,
                  0x1FC4, 0x2,    0x1FC7, 0x2,    0x1FCB, 0x1,    0x1FCC, 0x3,    0x1FCF, 0x1B,   0x1FD3, 0x2,
                  0x1FD7, 0x2,    0x1FDB, 0x1,    0x1FDF, 0x1B,   0x1FE7, 0x2,    0x1FEC, 0x1,    0x1FEF, 0x1B,
                  0x1FF4, 0x2,    0x1FF7, 0x2,    0x1FFB, 0x1,    0x1FFC, 0x3,    0x1FFE, 0x1B,   0x200A, 0xC,
                  0x200F, 0x10,   0x2015, 0x14,   0x2017, 0x18,   0x2018, 0x1D,   0x2019, 0x1E,   0x201A, 0x15,
                  0x201C, 0x1D,   0x201D, 0x1E,   0x201E, 0x15,   0x201F, 0x1D,   0x2027, 0x18,   0x2029, 0xE0D,
                  0x202E, 0x10,   0x202F, 0xC,    0x2038, 0x18,   0x203A, 0x1D1E, 0x203E, 0x18,   0x2040, 0x17,
                  0x2043, 0x18,   0x2044, 0x19,   0x2045, 0x15,   0x2046, 0x16,   0x2051, 0x18,   0x2052, 0x19,
                  0x2053, 0x18,   0x2054, 0x17,   0x205E, 0x18,   0x205F, 0xC,    0x2064, 0x10,   0x206F, 0x10,
                  0x2071, 0x40B,  0x2079, 0xB,    0x207C, 0x19,   0x207D, 0x15,   0x207E, 0x16,   0x207F, 0x4,
                  0x2089, 0xB,    0x208C, 0x19,   0x208E, 0x1516, 0x209C, 0x4,    0x20B9, 0x1A,   0x20DC, 0x6,
                  0x20E0, 0x7,    0x20E1, 0x6,    0x20E4, 0x7,    0x20F0, 0x6,    0x2101, 0x1C,   0x2102, 0x1,
                  0x2106, 0x1C,   0x2107, 0x1,    0x2109, 0x1C,   0x210A, 0x2,    0x210D, 0x1,    0x210F, 0x2,
                  0x2112, 0x1,    0x2113, 0x2,    0x2114, 0x1C,   0x2115, 0x1,    0x2117, 0x1C,   0x2118, 0x19,
                  0x211D, 0x1,    0x2123, 0x1C,   0x2129, 0x1C01, 0x212D, 0x1,    0x212F, 0x21C,  0x2133, 0x1,
                  0x2134, 0x2,    0x2138, 0x5,    0x2139, 0x2,    0x213B, 0x1C,   0x213D, 0x2,    0x213F, 0x1,
                  0x2144, 0x19,   0x2145, 0x1,    0x2149, 0x2,    0x214B, 0x191C, 0x214D, 0x1C,   0x214F, 0x1C02,
                  0x215F, 0xB,    0x2182, 0xA,    0x2184, 0x102,  0x2188, 0xA,    0x2189, 0xB,    0x2194, 0x19,
                  0x2199, 0x1C,   0x219B, 0x19,   0x219F, 0x1C,   0x21A0, 0x19,   0x21A2, 0x1C,   0x21A3, 0x19,
                  0x21A5, 0x1C,   0x21A6, 0x19,   0x21AD, 0x1C,   0x21AE, 0x19,   0x21CD, 0x1C,   0x21CF, 0x19,
                  0x21D1, 0x1C,   0x21D4, 0x1C19, 0x21F3, 0x1C,   0x22FF, 0x19,   0x2307, 0x1C,   0x230B, 0x19,
                  0x231F, 0x1C,   0x2321, 0x19,   0x2328, 0x1C,   0x232A, 0x1516, 0x237B, 0x1C,   0x237C, 0x19,
                  0x239A, 0x1C,   0x23B3, 0x19,   0x23DB, 0x1C,   0x23E1, 0x19,   0x23F3, 0x1C,   0x2426, 0x1C,
                  0x244A, 0x1C,   0x249B, 0xB,    0x24E9, 0x1C,   0x24FF, 0xB,    0x25B6, 0x1C,   0x25B7, 0x19,
                  0x25C0, 0x1C,   0x25C1, 0x19,   0x25F7, 0x1C,   0x25FF, 0x19,   0x266E, 0x1C,   0x266F, 0x19,
                  0x26FF, 0x1C,   0x2767, 0x1C,   0x2775, 0x1615, 0x2793, 0xB,    0x27BF, 0x1C,   0x27C4, 0x19,
                  0x27C6, 0x1516, 0x27CA, 0x19,   0x27CC, 0x19,   0x27E5, 0x19,   0x27EF, 0x1615, 0x27FF, 0x19,
                  0x28FF, 0x1C,   0x2982, 0x19,   0x2998, 0x1516, 0x29D7, 0x19,   0x29DB, 0x1615, 0x29FB, 0x19,
                  0x29FD, 0x1615, 0x2AFF, 0x19,   0x2B2F, 0x1C,   0x2B44, 0x19,   0x2B46, 0x1C,   0x2B4C, 0x19,
                  0x2B59, 0x1C,   0x2C2E, 0x1,    0x2C5E, 0x2,    0x2C61, 0x201,  0x2C64, 0x1,    0x2C66, 0x2,
                  0x2C6C, 0x102,  0x2C70, 0x1,    0x2C72, 0x201,  0x2C74, 0x2,    0x2C75, 0x1,    0x2C7C, 0x2,
                  0x2C7D, 0x4,    0x2C80, 0x1,    0x2CE2, 0x201,  0x2CE4, 0x2,    0x2CEA, 0x1C,   0x2CEE, 0x102,
                  0x2CF1, 0x6,    0x2CFC, 0x18,   0x2CFD, 0xB,    0x2CFF, 0x18,   0x2D25, 0x2,    0x2D65, 0x5,
                  0x2D70, 0x418,  0x2D7F, 0x6,    0x2D96, 0x5,    0x2DA6, 0x5,    0x2DAE, 0x5,    0x2DB6, 0x5,
                  0x2DBE, 0x5,    0x2DC6, 0x5,    0x2DCE, 0x5,    0x2DD6, 0x5,    0x2DDE, 0x5,    0x2DFF, 0x6,
                  0x2E01, 0x18,   0x2E05, 0x1E1D, 0x2E08, 0x18,   0x2E09, 0x1D,   0x2E0A, 0x1E,   0x2E0B, 0x18,
                  0x2E0C, 0x1D,   0x2E0D, 0x1E,   0x2E16, 0x18,   0x2E17, 0x14,   0x2E19, 0x18,   0x2E1A, 0x14,
                  0x2E1B, 0x18,   0x2E1C, 0x1D,   0x2E1D, 0x1E,   0x2E1F, 0x18,   0x2E20, 0x1D,   0x2E21, 0x1E,
                  0x2E22, 0x15,   0x2E23, 0x16,   0x2E24, 0x15,   0x2E25, 0x16,   0x2E26, 0x15,   0x2E27, 0x16,
                  0x2E28, 0x15,   0x2E29, 0x16,   0x2E2E, 0x18,   0x2E2F, 0x4,    0x2E31, 0x18,   0x2E99, 0x1C,
                  0x2EF3, 0x1C,   0x2FD5, 0x1C,   0x2FFB, 0x1C,   0x3000, 0xC,    0x3003, 0x18,   0x3004, 0x1C,
                  0x3005, 0x4,    0x3006, 0x5,    0x3007, 0xA,    0x3008, 0x15,   0x3009, 0x16,   0x300A, 0x15,
                  0x300B, 0x16,   0x300C, 0x15,   0x300D, 0x16,   0x300E, 0x15,   0x300F, 0x16,   0x3010, 0x15,
                  0x3011, 0x16,   0x3013, 0x1C,   0x3014, 0x15,   0x3015, 0x16,   0x3016, 0x15,   0x3017, 0x16,
                  0x3018, 0x15,   0x3019, 0x16,   0x301A, 0x15,   0x301B, 0x16,   0x301C, 0x14,   0x301D, 0x15,
                  0x301F, 0x16,   0x3020, 0x1C,   0x3029, 0xA,    0x302F, 0x6,    0x3030, 0x14,   0x3035, 0x4,
                  0x3037, 0x1C,   0x303A, 0xA,    0x303B, 0x4,    0x303C, 0x5,    0x303D, 0x18,   0x303F, 0x1C,
                  0x3096, 0x5,    0x309A, 0x6,    0x309C, 0x1B,   0x309E, 0x4,    0x30A0, 0x514,  0x30FA, 0x5,
                  0x30FB, 0x18,   0x30FE, 0x4,    0x30FF, 0x5,    0x312D, 0x5,    0x318E, 0x5,    0x3191, 0x1C,
                  0x3195, 0xB,    0x319F, 0x1C,   0x31BA, 0x5,    0x31E3, 0x1C,   0x31FF, 0x5,    0x321E, 0x1C,
                  0x3229, 0xB,    0x3250, 0x1C,   0x325F, 0xB,    0x327F, 0x1C,   0x3289, 0xB,    0x32B0, 0x1C,
                  0x32BF, 0xB,    0x32FE, 0x1C,   0x33FF, 0x1C,   0x4DB5, 0x5,    0x4DFF, 0x1C,   0x9FCB, 0x5,
                  0xA014, 0x5,    0xA015, 0x4,    0xA48C, 0x5,    0xA4C6, 0x1C,   0xA4F7, 0x5,    0xA4FD, 0x4,
                  0xA4FF, 0x18,   0xA60B, 0x5,    0xA60C, 0x4,    0xA60F, 0x18,   0xA61F, 0x5,    0xA629, 0x9,
                  0xA62B, 0x5,    0xA640, 0x1,    0xA641, 0x2,    0xA642, 0x1,    0xA643, 0x2,    0xA644, 0x1,
                  0xA645, 0x2,    0xA646, 0x1,    0xA647, 0x2,    0xA648, 0x1,    0xA649, 0x2,    0xA64A, 0x1,
                  0xA64B, 0x2,    0xA64C, 0x1,    0xA64D, 0x2,    0xA64E, 0x1,    0xA64F, 0x2,    0xA650, 0x1,
                  0xA651, 0x2,    0xA652, 0x1,    0xA653, 0x2,    0xA654, 0x1,    0xA655, 0x2,    0xA656, 0x1,
                  0xA657, 0x2,    0xA658, 0x1,    0xA659, 0x2,    0xA65A, 0x1,    0xA65B, 0x2,    0xA65C, 0x1,
                  0xA65D, 0x2,    0xA65E, 0x1,    0xA65F, 0x2,    0xA660, 0x1,    0xA661, 0x2,    0xA662, 0x1,
                  0xA663, 0x2,    0xA664, 0x1,    0xA665, 0x2,    0xA666, 0x1,    0xA667, 0x2,    0xA668, 0x1,
                  0xA669, 0x2,    0xA66A, 0x1,    0xA66B, 0x2,    0xA66C, 0x1,    0xA66D, 0x2,    0xA66E, 0x5,
                  0xA66F, 0x6,    0xA672, 0x7,    0xA673, 0x18,   0xA67D, 0x6,    0xA67E, 0x18,   0xA67F, 0x4,
                  0xA680, 0x1,    0xA681, 0x2,    0xA682, 0x1,    0xA683, 0x2,    0xA684, 0x1,    0xA685, 0x2,
                  0xA686, 0x1,    0xA687, 0x2,    0xA688, 0x1,    0xA689, 0x2,    0xA68A, 0x1,    0xA68B, 0x2,
                  0xA68C, 0x1,    0xA68D, 0x2,    0xA68E, 0x1,    0xA68F, 0x2,    0xA690, 0x1,    0xA691, 0x2,
                  0xA692, 0x1,    0xA693, 0x2,    0xA694, 0x1,    0xA695, 0x2,    0xA696, 0x1,    0xA697, 0x2,
                  0xA6E5, 0x5,    0xA6EF, 0xA,    0xA6F1, 0x6,    0xA6F7, 0x18,   0xA716, 0x1B,   0xA71F, 0x4,
                  0xA721, 0x1B,   0xA72E, 0x201,  0xA731, 0x2,    0xA732, 0x1,    0xA733, 0x2,    0xA734, 0x1,
                  0xA735, 0x2,    0xA736, 0x1,    0xA737, 0x2,    0xA738, 0x1,    0xA739, 0x2,    0xA73A, 0x1,
                  0xA73B, 0x2,    0xA73C, 0x1,    0xA73D, 0x2,    0xA73E, 0x1,    0xA73F, 0x2,    0xA740, 0x1,
                  0xA741, 0x2,    0xA742, 0x1,    0xA743, 0x2,    0xA744, 0x1,    0xA745, 0x2,    0xA746, 0x1,
                  0xA747, 0x2,    0xA748, 0x1,    0xA749, 0x2,    0xA74A, 0x1,    0xA74B, 0x2,    0xA74C, 0x1,
                  0xA74D, 0x2,    0xA74E, 0x1,    0xA74F, 0x2,    0xA750, 0x1,    0xA751, 0x2,    0xA752, 0x1,
                  0xA753, 0x2,    0xA754, 0x1,    0xA755, 0x2,    0xA756, 0x1,    0xA757, 0x2,    0xA758, 0x1,
                  0xA759, 0x2,    0xA75A, 0x1,    0xA75B, 0x2,    0xA75C, 0x1,    0xA75D, 0x2,    0xA75E, 0x1,
                  0xA75F, 0x2,    0xA760, 0x1,    0xA761, 0x2,    0xA762, 0x1,    0xA763, 0x2,    0xA764, 0x1,
                  0xA765, 0x2,    0xA766, 0x1,    0xA767, 0x2,    0xA768, 0x1,    0xA769, 0x2,    0xA76A, 0x1,
                  0xA76B, 0x2,    0xA76C, 0x1,    0xA76D, 0x2,    0xA76E, 0x1,    0xA76F, 0x2,    0xA770, 0x4,
                  0xA778, 0x2,    0xA77C, 0x102,  0xA77E, 0x1,    0xA77F, 0x2,    0xA780, 0x1,    0xA781, 0x2,
                  0xA782, 0x1,    0xA783, 0x2,    0xA784, 0x1,    0xA785, 0x2,    0xA786, 0x1,    0xA787, 0x2,
                  0xA788, 0x4,    0xA78A, 0x1B,   0xA78E, 0x102,  0xA791, 0x201,  0xA7A9, 0x201,  0xA7FA, 0x2,
                  0xA801, 0x5,    0xA802, 0x6,    0xA805, 0x5,    0xA806, 0x6,    0xA80A, 0x5,    0xA80B, 0x6,
                  0xA822, 0x5,    0xA824, 0x8,    0xA826, 0x6,    0xA827, 0x8,    0xA82B, 0x1C,   0xA835, 0xB,
                  0xA837, 0x1C,   0xA839, 0x1C1A, 0xA873, 0x5,    0xA877, 0x18,   0xA881, 0x8,    0xA8B3, 0x5,
                  0xA8C3, 0x8,    0xA8C4, 0x6,    0xA8CF, 0x18,   0xA8D9, 0x9,    0xA8F1, 0x6,    0xA8F7, 0x5,
                  0xA8FA, 0x18,   0xA8FB, 0x5,    0xA909, 0x9,    0xA925, 0x5,    0xA92D, 0x6,    0xA92F, 0x18,
                  0xA946, 0x5,    0xA951, 0x6,    0xA953, 0x8,    0xA95F, 0x18,   0xA97C, 0x5,    0xA982, 0x6,
                  0xA983, 0x8,    0xA9B2, 0x5,    0xA9B3, 0x6,    0xA9B5, 0x8,    0xA9B9, 0x6,    0xA9BB, 0x8,
                  0xA9BC, 0x6,    0xA9C0, 0x8,    0xA9CD, 0x18,   0xA9CF, 0x4,    0xA9D9, 0x9,    0xA9DF, 0x18,
                  0xAA28, 0x5,    0xAA2E, 0x6,    0xAA30, 0x8,    0xAA32, 0x6,    0xAA34, 0x8,    0xAA36, 0x6,
                  0xAA42, 0x5,    0xAA43, 0x6,    0xAA4B, 0x5,    0xAA4D, 0x806,  0xAA59, 0x9,    0xAA5F, 0x18,
                  0xAA6F, 0x5,    0xAA70, 0x4,    0xAA76, 0x5,    0xAA79, 0x1C,   0xAA7B, 0x805,  0xAAAF, 0x5,
                  0xAAB1, 0x506,  0xAAB4, 0x6,    0xAAB6, 0x5,    0xAAB8, 0x6,    0xAABD, 0x5,    0xAABF, 0x6,
                  0xAAC2, 0x605,  0xAADC, 0x5,    0xAADD, 0x4,    0xAADF, 0x18,   0xAB06, 0x5,    0xAB0E, 0x5,
                  0xAB16, 0x5,    0xAB26, 0x5,    0xAB2E, 0x5,    0xABE2, 0x5,    0xABE4, 0x8,    0xABE5, 0x6,
                  0xABE7, 0x8,    0xABE8, 0x6,    0xABEA, 0x8,    0xABEB, 0x18,   0xABEC, 0x8,    0xABED, 0x6,
                  0xABF9, 0x9,    0xD7A3, 0x5,    0xD7C6, 0x5,    0xD7FB, 0x5,    0xDFFF, 0x13,   0xF8FF, 0x12,
                  0xFA2D, 0x5,    0xFA6D, 0x5,    0xFAD9, 0x5,    0xFB06, 0x2,    0xFB17, 0x2,    0xFB1E, 0x506,
                  0xFB28, 0x5,    0xFB29, 0x19,   0xFB36, 0x5,    0xFB3C, 0x5,    0xFB3E, 0x5,    0xFB41, 0x5,
                  0xFB44, 0x5,    0xFBB1, 0x5,    0xFBC1, 0x1B,   0xFD3D, 0x5,    0xFD3F, 0x1615, 0xFD8F, 0x5,
                  0xFDC7, 0x5,    0xFDFB, 0x5,    0xFDFD, 0x1C1A, 0xFE0F, 0x6,    0xFE16, 0x18,   0xFE17, 0x15,
                  0xFE18, 0x16,   0xFE19, 0x18,   0xFE26, 0x6,    0xFE30, 0x18,   0xFE32, 0x14,   0xFE34, 0x17,
                  0xFE44, 0x1516, 0xFE46, 0x18,   0xFE48, 0x1516, 0xFE4C, 0x18,   0xFE4F, 0x17,   0xFE52, 0x18,
                  0xFE57, 0x18,   0xFE58, 0x14,   0xFE59, 0x15,   0xFE5A, 0x16,   0xFE5B, 0x15,   0xFE5C, 0x16,
                  0xFE5D, 0x15,   0xFE5E, 0x16,   0xFE61, 0x18,   0xFE63, 0x1419, 0xFE66, 0x19,   0xFE69, 0x1A18,
                  0xFE6B, 0x18,   0xFE74, 0x5,    0xFEFC, 0x5,    0xFEFF, 0x10,   0xFF03, 0x18,   0xFF04, 0x1A,
                  0xFF07, 0x18,   0xFF08, 0x15,   0xFF09, 0x16,   0xFF0A, 0x18,   0xFF0B, 0x19,   0xFF0C, 0x18,
                  0xFF0D, 0x14,   0xFF0F, 0x18,   0xFF19, 0x9,    0xFF1B, 0x18,   0xFF1E, 0x19,   0xFF20, 0x18,
                  0xFF3A, 0x1,    0xFF3B, 0x15,   0xFF3C, 0x18,   0xFF3D, 0x16,   0xFF3E, 0x1B,   0xFF3F, 0x17,
                  0xFF40, 0x1B,   0xFF5A, 0x2,    0xFF5B, 0x15,   0xFF5C, 0x19,   0xFF5D, 0x16,   0xFF5E, 0x19,
                  0xFF5F, 0x15,   0xFF60, 0x16,   0xFF61, 0x18,   0xFF62, 0x15,   0xFF63, 0x16,   0xFF65, 0x18,
                  0xFF6F, 0x5,    0xFF70, 0x4,    0xFF9D, 0x5,    0xFF9F, 0x4,    0xFFBE, 0x5,    0xFFC7, 0x5,
                  0xFFCF, 0x5,    0xFFD7, 0x5,    0xFFDC, 0x5,    0xFFE1, 0x1A,   0xFFE2, 0x19,   0xFFE3, 0x1B,
                  0xFFE4, 0x1C,   0xFFE6, 0x1A,   0xFFE8, 0x1C,   0xFFEC, 0x19,   0xFFEE, 0x1C,   0xFFFB, 0x10,
                  0xFFFD, 0x1C } };

            static const std::array<char16_t, 1000 - 192> kLowerCaseCache
            = { { 224, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239, 240, 241, 242, 243,
                  244, 245, 246, 215, 248, 249, 250, 251, 252, 253, 254, 223, 224, 225, 226, 227, 228, 229, 230, 231,
                  232, 233, 234, 235, 236, 237, 238, 239, 240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251,
                  252, 253, 254, 255, 257, 257, 259, 259, 261, 261, 263, 263, 265, 265, 267, 267, 269, 269, 271, 271,
                  273, 273, 275, 275, 277, 277, 279, 279, 281, 281, 283, 283, 285, 285, 287, 287, 289, 289, 291, 291,
                  293, 293, 295, 295, 297, 297, 299, 299, 301, 301, 303, 303, 105, 305, 307, 307, 309, 309, 311, 311,
                  312, 314, 314, 316, 316, 318, 318, 320, 320, 322, 322, 324, 324, 326, 326, 328, 328, 329, 331, 331,
                  333, 333, 335, 335, 337, 337, 339, 339, 341, 341, 343, 343, 345, 345, 347, 347, 349, 349, 351, 351,
                  353, 353, 355, 355, 357, 357, 359, 359, 361, 361, 363, 363, 365, 365, 367, 367, 369, 369, 371, 371,
                  373, 373, 375, 375, 255, 378, 378, 380, 380, 382, 382, 383, 384, 595, 387, 387, 389, 389, 596, 392,
                  392, 598, 599, 396, 396, 397, 477, 601, 603, 402, 402, 608, 611, 405, 617, 616, 409, 409, 410, 411,
                  623, 626, 414, 629, 417, 417, 419, 419, 421, 421, 640, 424, 424, 643, 426, 427, 429, 429, 648, 432,
                  432, 650, 651, 436, 436, 438, 438, 658, 441, 441, 442, 443, 445, 445, 446, 447, 448, 449, 450, 451,
                  454, 454, 454, 457, 457, 457, 460, 460, 460, 462, 462, 464, 464, 466, 466, 468, 468, 470, 470, 472,
                  472, 474, 474, 476, 476, 477, 479, 479, 481, 481, 483, 483, 485, 485, 487, 487, 489, 489, 491, 491,
                  493, 493, 495, 495, 496, 499, 499, 499, 501, 501, 405, 447, 505, 505, 507, 507, 509, 509, 511, 511,
                  513, 513, 515, 515, 517, 517, 519, 519, 521, 521, 523, 523, 525, 525, 527, 527, 529, 529, 531, 531,
                  533, 533, 535, 535, 537, 537, 539, 539, 541, 541, 543, 543, 414, 545, 547, 547, 549, 549, 551, 551,
                  553, 553, 555, 555, 557, 557, 559, 559, 561, 561, 563, 563, 564, 565, 566, 567, 568, 569, 570, 571,
                  572, 573, 574, 575, 576, 577, 578, 579, 580, 581, 582, 583, 584, 585, 586, 587, 588, 589, 590, 591,
                  592, 593, 594, 595, 596, 597, 598, 599, 600, 601, 602, 603, 604, 605, 606, 607, 608, 609, 610, 611,
                  612, 613, 614, 615, 616, 617, 618, 619, 620, 621, 622, 623, 624, 625, 626, 627, 628, 629, 630, 631,
                  632, 633, 634, 635, 636, 637, 638, 639, 640, 641, 642, 643, 644, 645, 646, 647, 648, 649, 650, 651,
                  652, 653, 654, 655, 656, 657, 658, 659, 660, 661, 662, 663, 664, 665, 666, 667, 668, 669, 670, 671,
                  672, 673, 674, 675, 676, 677, 678, 679, 680, 681, 682, 683, 684, 685, 686, 687, 688, 689, 690, 691,
                  692, 693, 694, 695, 696, 697, 698, 699, 700, 701, 702, 703, 704, 705, 706, 707, 708, 709, 710, 711,
                  712, 713, 714, 715, 716, 717, 718, 719, 720, 721, 722, 723, 724, 725, 726, 727, 728, 729, 730, 731,
                  732, 733, 734, 735, 736, 737, 738, 739, 740, 741, 742, 743, 744, 745, 746, 747, 748, 749, 750, 751,
                  752, 753, 754, 755, 756, 757, 758, 759, 760, 761, 762, 763, 764, 765, 766, 767, 768, 769, 770, 771,
                  772, 773, 774, 775, 776, 777, 778, 779, 780, 781, 782, 783, 784, 785, 786, 787, 788, 789, 790, 791,
                  792, 793, 794, 795, 796, 797, 798, 799, 800, 801, 802, 803, 804, 805, 806, 807, 808, 809, 810, 811,
                  812, 813, 814, 815, 816, 817, 818, 819, 820, 821, 822, 823, 824, 825, 826, 827, 828, 829, 830, 831,
                  832, 833, 834, 835, 836, 837, 838, 839, 840, 841, 842, 843, 844, 845, 846, 847, 848, 849, 850, 851,
                  852, 853, 854, 855, 856, 857, 858, 859, 860, 861, 862, 863, 864, 865, 866, 867, 868, 869, 870, 871,
                  872, 873, 874, 875, 876, 877, 878, 879, 880, 881, 882, 883, 884, 885, 886, 887, 888, 889, 890, 891,
                  892, 893, 894, 895, 896, 897, 898, 899, 900, 901, 940, 903, 941, 942, 943, 907, 972, 909, 973, 974,
                  912, 945, 946, 947, 948, 949, 950, 951, 952, 953, 954, 955, 956, 957, 958, 959, 960, 961, 930, 963,
                  964, 965, 966, 967, 968, 969, 970, 971, 940, 941, 942, 943, 944, 945, 946, 947, 948, 949, 950, 951,
                  952, 953, 954, 955, 956, 957, 958, 959, 960, 961, 962, 963, 964, 965, 966, 967, 968, 969, 970, 971,
                  972, 973, 974, 975, 976, 977, 978, 979, 980, 981, 982, 983, 985, 985, 987, 987, 989, 989, 991, 991,
                  993, 993, 995, 995, 997, 997, 999, 999 } };

            static const std::array<char16_t, 101> kLowerCaseKeys = { {
            0x41,   0xC0,   0xD8,   0x100,  0x130,  0x132,  0x139,  0x14A,  0x178,  0x179,  0x181,  0x182,  0x186,
            0x187,  0x189,  0x18B,  0x18E,  0x18F,  0x190,  0x191,  0x193,  0x194,  0x196,  0x197,  0x198,  0x19C,
            0x19D,  0x19F,  0x1A0,  0x1A6,  0x1A7,  0x1A9,  0x1AC,  0x1AE,  0x1AF,  0x1B1,  0x1B3,  0x1B7,  0x1B8,
            0x1BC,  0x1C4,  0x1C5,  0x1C7,  0x1C8,  0x1CA,  0x1CB,  0x1DE,  0x1F1,  0x1F2,  0x1F6,  0x1F7,  0x1F8,
            0x222,  0x386,  0x388,  0x38C,  0x38E,  0x391,  0x3A3,  0x3DA,  0x400,  0x410,  0x460,  0x48C,  0x4C1,
            0x4C7,  0x4CB,  0x4D0,  0x4F8,  0x531,  0x1E00, 0x1EA0, 0x1F08, 0x1F18, 0x1F28, 0x1F38, 0x1F48, 0x1F59,
            0x1F68, 0x1F88, 0x1F98, 0x1FA8, 0x1FB8, 0x1FBA, 0x1FBC, 0x1FC8, 0x1FCC, 0x1FD8, 0x1FDA, 0x1FE8, 0x1FEA,
            0x1FEC, 0x1FF8, 0x1FFA, 0x1FFC, 0x2126, 0x212A, 0x212B, 0x2160, 0x24B6, 0xFF21// NOLINT
            } };

            static const std::array<char16_t, 202> kLowerCaseValues = { {
            0x5A,   0x20,   0xD6,   0x20,   0xDE,   0x20,   0x812E, 0x1,    0x130,  0xFF39, 0x8136, 0x1,    0x8147,
            0x1,    0x8176, 0x1,    0x178,  0xFF87, 0x817D, 0x1,    0x181,  0xD2,   0x8184, 0x1,    0x186,  0xCE,
            0x187,  0x1,    0x18A,  0xCD,   0x18B,  0x1,    0x18E,  0x4F,   0x18F,  0xCA,   0x190,  0xCB,   0x191,
            0x1,    0x193,  0xCD,   0x194,  0xCF,   0x196,  0xD3,   0x197,  0xD1,   0x198,  0x1,    0x19C,  0xD3,
            0x19D,  0xD5,   0x19F,  0xD6,   0x81A4, 0x1,    0x1A6,  0xDA,   0x1A7,  0x1,    0x1A9,  0xDA,   0x1AC,
            0x1,    0x1AE,  0xDA,   0x1AF,  0x1,    0x1B2,  0xD9,   0x81B5, 0x1,    0x1B7,  0xDB,   0x1B8,  0x1,
            0x1BC,  0x1,    0x1C4,  0x2,    0x1C5,  0x1,    0x1C7,  0x2,    0x1C8,  0x1,    0x1CA,  0x2,    0x81DB,
            0x1,    0x81EE, 0x1,    0x1F1,  0x2,    0x81F4, 0x1,    0x1F6,  0xFF9F, 0x1F7,  0xFFC8, 0x821E, 0x1,
            0x8232, 0x1,    0x386,  0x26,   0x38A,  0x25,   0x38C,  0x40,   0x38F,  0x3F,   0x3A1,  0x20,   0x3AB,
            0x20,   0x83EE, 0x1,    0x40F,  0x50,   0x42F,  0x20,   0x8480, 0x1,    0x84BE, 0x1,    0x84C3, 0x1,
            0x4C7,  0x1,    0x4CB,  0x1,    0x84F4, 0x1,    0x4F8,  0x1,    0x556,  0x30,   0x9E94, 0x1,    0x9EF8,
            0x1,    0x1F0F, 0xFFF8, 0x1F1D, 0xFFF8, 0x1F2F, 0xFFF8, 0x1F3F, 0xFFF8, 0x1F4D, 0xFFF8, 0x9F5F, 0xFFF8,
            0x1F6F, 0xFFF8, 0x1F8F, 0xFFF8, 0x1F9F, 0xFFF8, 0x1FAF, 0xFFF8, 0x1FB9, 0xFFF8, 0x1FBB, 0xFFB6, 0x1FBC,
            0xFFF7, 0x1FCB, 0xFFAA, 0x1FCC, 0xFFF7, 0x1FD9, 0xFFF8, 0x1FDB, 0xFF9C, 0x1FE9, 0xFFF8, 0x1FEB, 0xFF90,
            0x1FEC, 0xFFF9, 0x1FF9, 0xFF80, 0x1FFB, 0xFF82, 0x1FFC, 0xFFF7, 0x2126, 0xE2A3, 0x212A, 0xDF41, 0x212B,
            0xDFBA, 0x216F, 0x10,   0x24CF, 0x1A,   0xFF3A, 0x20// NOLINT
            } };

            static const std::array<char16_t, 1000 - 181> kUpperCaseCache
            = { { 924, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191, 192, 193, 194, 195, 196, 197, 198, 199, 200,
                  201, 202, 203, 204, 205, 206, 207, 208, 209, 210, 211, 212, 213, 214, 215, 216, 217, 218, 219, 220,
                  221, 222, 223, 192, 193, 194, 195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207, 208,
                  209, 210, 211, 212, 213, 214, 247, 216, 217, 218, 219, 220, 221, 222, 376, 256, 256, 258, 258, 260,
                  260, 262, 262, 264, 264, 266, 266, 268, 268, 270, 270, 272, 272, 274, 274, 276, 276, 278, 278, 280,
                  280, 282, 282, 284, 284, 286, 286, 288, 288, 290, 290, 292, 292, 294, 294, 296, 296, 298, 298, 300,
                  300, 302, 302, 304, 73,  306, 306, 308, 308, 310, 310, 312, 313, 313, 315, 315, 317, 317, 319, 319,
                  321, 321, 323, 323, 325, 325, 327, 327, 329, 330, 330, 332, 332, 334, 334, 336, 336, 338, 338, 340,
                  340, 342, 342, 344, 344, 346, 346, 348, 348, 350, 350, 352, 352, 354, 354, 356, 356, 358, 358, 360,
                  360, 362, 362, 364, 364, 366, 366, 368, 368, 370, 370, 372, 372, 374, 374, 376, 377, 377, 379, 379,
                  381, 381, 83,  384, 385, 386, 386, 388, 388, 390, 391, 391, 393, 394, 395, 395, 397, 398, 399, 400,
                  401, 401, 403, 404, 502, 406, 407, 408, 408, 410, 411, 412, 413, 544, 415, 416, 416, 418, 418, 420,
                  420, 422, 423, 423, 425, 426, 427, 428, 428, 430, 431, 431, 433, 434, 435, 435, 437, 437, 439, 440,
                  440, 442, 443, 444, 444, 446, 503, 448, 449, 450, 451, 452, 452, 452, 455, 455, 455, 458, 458, 458,
                  461, 461, 463, 463, 465, 465, 467, 467, 469, 469, 471, 471, 473, 473, 475, 475, 398, 478, 478, 480,
                  480, 482, 482, 484, 484, 486, 486, 488, 488, 490, 490, 492, 492, 494, 494, 496, 497, 497, 497, 500,
                  500, 502, 503, 504, 504, 506, 506, 508, 508, 510, 510, 512, 512, 514, 514, 516, 516, 518, 518, 520,
                  520, 522, 522, 524, 524, 526, 526, 528, 528, 530, 530, 532, 532, 534, 534, 536, 536, 538, 538, 540,
                  540, 542, 542, 544, 545, 546, 546, 548, 548, 550, 550, 552, 552, 554, 554, 556, 556, 558, 558, 560,
                  560, 562, 562, 564, 565, 566, 567, 568, 569, 570, 571, 572, 573, 574, 575, 576, 577, 578, 579, 580,
                  581, 582, 583, 584, 585, 586, 587, 588, 589, 590, 591, 592, 593, 594, 385, 390, 597, 393, 394, 600,
                  399, 602, 400, 604, 605, 606, 607, 403, 609, 610, 404, 612, 613, 614, 615, 407, 406, 618, 619, 620,
                  621, 622, 412, 624, 625, 413, 627, 628, 415, 630, 631, 632, 633, 634, 635, 636, 637, 638, 639, 422,
                  641, 642, 425, 644, 645, 646, 647, 430, 649, 433, 434, 652, 653, 654, 655, 656, 657, 439, 659, 660,
                  661, 662, 663, 664, 665, 666, 667, 668, 669, 670, 671, 672, 673, 674, 675, 676, 677, 678, 679, 680,
                  681, 682, 683, 684, 685, 686, 687, 688, 689, 690, 691, 692, 693, 694, 695, 696, 697, 698, 699, 700,
                  701, 702, 703, 704, 705, 706, 707, 708, 709, 710, 711, 712, 713, 714, 715, 716, 717, 718, 719, 720,
                  721, 722, 723, 724, 725, 726, 727, 728, 729, 730, 731, 732, 733, 734, 735, 736, 737, 738, 739, 740,
                  741, 742, 743, 744, 745, 746, 747, 748, 749, 750, 751, 752, 753, 754, 755, 756, 757, 758, 759, 760,
                  761, 762, 763, 764, 765, 766, 767, 768, 769, 770, 771, 772, 773, 774, 775, 776, 777, 778, 779, 780,
                  781, 782, 783, 784, 785, 786, 787, 788, 789, 790, 791, 792, 793, 794, 795, 796, 797, 798, 799, 800,
                  801, 802, 803, 804, 805, 806, 807, 808, 809, 810, 811, 812, 813, 814, 815, 816, 817, 818, 819, 820,
                  821, 822, 823, 824, 825, 826, 827, 828, 829, 830, 831, 832, 833, 834, 835, 836, 921, 838, 839, 840,
                  841, 842, 843, 844, 845, 846, 847, 848, 849, 850, 851, 852, 853, 854, 855, 856, 857, 858, 859, 860,
                  861, 862, 863, 864, 865, 866, 867, 868, 869, 870, 871, 872, 873, 874, 875, 876, 877, 878, 879, 880,
                  881, 882, 883, 884, 885, 886, 887, 888, 889, 890, 891, 892, 893, 894, 895, 896, 897, 898, 899, 900,
                  901, 902, 903, 904, 905, 906, 907, 908, 909, 910, 911, 912, 913, 914, 915, 916, 917, 918, 919, 920,
                  921, 922, 923, 924, 925, 926, 927, 928, 929, 930, 931, 932, 933, 934, 935, 936, 937, 938, 939, 902,
                  904, 905, 906, 944, 913, 914, 915, 916, 917, 918, 919, 920, 921, 922, 923, 924, 925, 926, 927, 928,
                  929, 931, 931, 932, 933, 934, 935, 936, 937, 938, 939, 908, 910, 911, 975, 914, 920, 978, 979, 980,
                  934, 928, 983, 984, 984, 986, 986, 988, 988, 990, 990, 992, 992, 994, 994, 996, 996, 998, 998 } };

            static const std::array<char16_t, 113> kUpperCaseKeys = { {
            0x61,   0xB5,   0xE0,   0xF8,   0xFF,   0x101,  0x131,  0x133,  0x13A,  0x14B,  0x17A,  0x17F,  0x183,
            0x188,  0x18C,  0x192,  0x195,  0x199,  0x1A1,  0x1A8,  0x1AD,  0x1B0,  0x1B4,  0x1B9,  0x1BD,  0x1BF,
            0x1C5,  0x1C6,  0x1C8,  0x1C9,  0x1CB,  0x1CC,  0x1CE,  0x1DD,  0x1DF,  0x1F2,  0x1F3,  0x1F5,  0x1F9,
            0x223,  0x253,  0x254,  0x256,  0x259,  0x25B,  0x260,  0x263,  0x268,  0x269,  0x26F,  0x272,  0x275,
            0x280,  0x283,  0x288,  0x28A,  0x292,  0x345,  0x3AC,  0x3AD,  0x3B1,  0x3C2,  0x3C3,  0x3CC,  0x3CD,
            0x3D0,  0x3D1,  0x3D5,  0x3D6,  0x3DB,  0x3F0,  0x3F1,  0x3F2,  0x430,  0x450,  0x461,  0x48D,  0x4C2,
            0x4C8,  0x4CC,  0x4D1,  0x4F9,  0x561,  0x1E01, 0x1E9B, 0x1EA1, 0x1F00, 0x1F10, 0x1F20, 0x1F30, 0x1F40,
            0x1F51, 0x1F60, 0x1F70, 0x1F72, 0x1F76, 0x1F78, 0x1F7A, 0x1F7C, 0x1F80, 0x1F90, 0x1FA0, 0x1FB0, 0x1FB3,
            0x1FBE, 0x1FC3, 0x1FD0, 0x1FE0, 0x1FE5, 0x1FF3, 0x2170, 0x24D0, 0xFF41// NOLINT
            } };

            static const std::array<char16_t, 226> kUpperCaseValues = { {
            0x7A,   0xFFE0, 0xB5,   0x2E7,  0xF6,   0xFFE0, 0xFE,   0xFFE0, 0xFF,   0x79,   0x812F, 0xFFFF, 0x131,
            0xFF18, 0x8137, 0xFFFF, 0x8148, 0xFFFF, 0x8177, 0xFFFF, 0x817E, 0xFFFF, 0x17F,  0xFED4, 0x8185, 0xFFFF,
            0x188,  0xFFFF, 0x18C,  0xFFFF, 0x192,  0xFFFF, 0x195,  0x61,   0x199,  0xFFFF, 0x81A5, 0xFFFF, 0x1A8,
            0xFFFF, 0x1AD,  0xFFFF, 0x1B0,  0xFFFF, 0x81B6, 0xFFFF, 0x1B9,  0xFFFF, 0x1BD,  0xFFFF, 0x1BF,  0x38,
            0x1C5,  0xFFFF, 0x1C6,  0xFFFE, 0x1C8,  0xFFFF, 0x1C9,  0xFFFE, 0x1CB,  0xFFFF, 0x1CC,  0xFFFE, 0x81DC,
            0xFFFF, 0x1DD,  0xFFB1, 0x81EF, 0xFFFF, 0x1F2,  0xFFFF, 0x1F3,  0xFFFE, 0x1F5,  0xFFFF, 0x821F, 0xFFFF,
            0x8233, 0xFFFF, 0x253,  0xFF2E, 0x254,  0xFF32, 0x257,  0xFF33, 0x259,  0xFF36, 0x25B,  0xFF35, 0x260,
            0xFF33, 0x263,  0xFF31, 0x268,  0xFF2F, 0x269,  0xFF2D, 0x26F,  0xFF2D, 0x272,  0xFF2B, 0x275,  0xFF2A,
            0x280,  0xFF26, 0x283,  0xFF26, 0x288,  0xFF26, 0x28B,  0xFF27, 0x292,  0xFF25, 0x345,  0x54,   0x3AC,
            0xFFDA, 0x3AF,  0xFFDB, 0x3C1,  0xFFE0, 0x3C2,  0xFFE1, 0x3CB,  0xFFE0, 0x3CC,  0xFFC0, 0x3CE,  0xFFC1,
            0x3D0,  0xFFC2, 0x3D1,  0xFFC7, 0x3D5,  0xFFD1, 0x3D6,  0xFFCA, 0x83EF, 0xFFFF, 0x3F0,  0xFFAA, 0x3F1,
            0xFFB0, 0x3F2,  0xFFB1, 0x44F,  0xFFE0, 0x45F,  0xFFB0, 0x8481, 0xFFFF, 0x84BF, 0xFFFF, 0x84C4, 0xFFFF,
            0x4C8,  0xFFFF, 0x4CC,  0xFFFF, 0x84F5, 0xFFFF, 0x4F9,  0xFFFF, 0x586,  0xFFD0, 0x9E95, 0xFFFF, 0x1E9B,
            0xFFC5, 0x9EF9, 0xFFFF, 0x1F07, 0x8,    0x1F15, 0x8,    0x1F27, 0x8,    0x1F37, 0x8,    0x1F45, 0x8,
            0x9F57, 0x8,    0x1F67, 0x8,    0x1F71, 0x4A,   0x1F75, 0x56,   0x1F77, 0x64,   0x1F79, 0x80,   0x1F7B,
            0x70,   0x1F7D, 0x7E,   0x1F87, 0x8,    0x1F97, 0x8,    0x1FA7, 0x8,    0x1FB1, 0x8,    0x1FB3, 0x9,
            0x1FBE, 0xE3DB, 0x1FC3, 0x9,    0x1FD1, 0x8,    0x1FE1, 0x8,    0x1FE5, 0x7,    0x1FF3, 0x9,    0x217F,
            0xFFF0, 0x24E9, 0xFFE6, 0xFF5A, 0xFFE0// NOLINT
            } };

            inline Category GetCategory(char16_t c)
            {
                if(c < 1000)
                {
                    return static_cast<Category>(kCategoryCache[c]);
                }
                const int result
                = static_cast<int>(std::upper_bound(kCategoryKeys.begin(), kCategoryKeys.end(), c) - kCategoryKeys.begin() - 1);
                assert(result < static_cast<int>(kCategoryKeys.size()));
                const int high = kCategoryValues[result * 2];
                if(c <= high)
                {
                    const int code = kCategoryValues[result * 2 + 1];
                    if(code < 0x100)
                    {
                        return static_cast<Category>(code);
                    }
                    return static_cast<Category>((c & 1) == 1 ? code >> 8 : code & 0xff);
                }
                return UNASSIGNED;
            }

        }// namespace unicode
    }// namespace character

    namespace log
    {
        void PrintSource(const std::string& comment, const std::u16string& str = u"", const std::string& postfix = "")
        {
#ifdef TEST
            std::cout << comment;
            for(const auto& c : str)
                std::cout << static_cast<char>(c);
            std::cout << postfix << std::endl;
#endif
        }

        std::string ToString(const std::u16string& str)
        {
            std::string result(str.size(), ' ');
            for(size_t i = 0; i < str.size(); i++)
                result[i] = str[i];
            return result;
        }

        std::string ToString(bool b)
        {
            return b ? "true" : "false";
        }

        std::string ToString(const void* ptr)
        {
            std::stringstream ss;
            ss << ptr;
            return ss.str();
        }

    }// namespace log

    std::u16string StrCat(const std::vector<std::u16string>& vals)
    {
        size_t size = 0;
        for(auto val : vals)
        {
            size += val.size();
        }
        std::u16string res(size, 0);
        size_t offset = 0;
        for(auto val : vals)
        {
            memcpy((void*)(res.c_str() + offset), (void*)(val.data()), val.size() * 2);
            offset += val.size();
        }
        return res;
    }

    bool HaveDuplicate(const std::vector<std::u16string>& vals)
    {
        for(size_t i = 0; i < vals.size(); i++)
        {
            for(size_t j = 0; j < vals.size(); j++)
            {
                if(i != j && vals[i] == vals[j])
                    return true;
            }
        }
        return false;
    }

    namespace character
    {
        using namespace unicode;

        // End of Line
        static const char16_t EOS = 0x0000;

        // Format-Control Character
        static const char16_t ZWNJ = 0x200C;// Zero width non-joiner
        static const char16_t ZWJ = 0x200D;// Zero width joiner
        static const char16_t BOM = 0xFEFF;// Byte Order Mark

        // White Space
        static const char16_t TAB = 0x0009;// Tab
        static const char16_t VT = 0x000B;// Vertical Tab
        static const char16_t FF = 0x0020;// Form Feed
        static const char16_t SP = 0x0020;// Space
        static const char16_t hashx0a = 0x00A0;// No-break space
        // static const char16_t BOM  = 0xFEFF;  // Byte Order Mark
        // USP includes lots of characters, therefore only included in the function.

        // Line Terminators
        static const char16_t LF = 0x000A;
        static const char16_t CR = 0x000D;
        static const char16_t LS = 0x2028;
        static const char16_t PS = 0x2029;

        inline bool IsUSP(char16_t c)
        {
            return c == 0x1680 || (c >= 0x2000 && c <= 0x200A) || c == 0x202F || c == 0x205F || c == 0x3000;
        }

        inline bool IsWhiteSpace(char16_t c)
        {
            return c == TAB || c == VT || c == FF || c == FF || c == SP || c == hashx0a || IsUSP(c);
        }

        inline bool IsLineTerminator(char16_t c)
        {
            return c == LF || c == CR || c == LS || c == PS;
        }

        inline bool IsDecimalDigit(char16_t c)
        {
            return c >= u'0' && c <= u'9';
        }

        inline bool IsUnicodeLetter(char16_t c)
        {
            return ((1 << GetCategory(c)) & (Lu | Ll | Lt | Lm | Lo | Nl));
        }

        inline bool IsUnicodeCombiningMark(char16_t c)
        {
            return ((1 << GetCategory(c)) & (Mn | Mc));
        }

        inline bool IsUnicodeDigit(char16_t c)
        {
            return GetCategory(c) == DECIMAL_DIGIT_NUMBER;
        }

        inline bool IsUnicodeConnectorPunctuation(char16_t c)
        {
            return GetCategory(c) == CONNECTOR_PUNCTUATION;
        }

        inline bool IsHexDigit(char16_t c)
        {
            return IsDecimalDigit(c) || u'A' <= c && c <= u'F' || u'a' <= c && c <= u'f';
        }

        inline bool IsSingleEscapeCharacter(char16_t c)
        {
            return c == u'\'' || c == u'"' || c == u'\\' || c == u'b' || c == u'f' || c == u'f' || c == u'n'
                   || c == u'r' || c == u't' || c == u'v';
        }

        inline bool IsEscapeCharacter(char16_t c)
        {
            return IsSingleEscapeCharacter(c) || IsDecimalDigit(c) || c == u'x' || c == u'u';
        }

        inline bool IsNonEscapeCharacter(char16_t c)
        {
            return !IsEscapeCharacter(c) && !IsLineTerminator(c);
        }

        inline bool IsCharacterEscapeSequence(char16_t c)
        {
            // NODE(zhuzilin) The chars that are not in { LineTerminator, DecimalDigit, u'x', u'u' }.
            return IsSingleEscapeCharacter(c) || IsNonEscapeCharacter(c);
        }

        inline bool IsIdentifierStart(char16_t c)
        {
            return IsUnicodeLetter(c) || c == u'$' || c == u'_' || c == u'\\';
        }

        inline bool IsIdentifierPart(char16_t c)
        {
            return IsIdentifierStart(c) || IsUnicodeCombiningMark(c) || IsUnicodeDigit(c)
                   || IsUnicodeConnectorPunctuation(c) || c == ZWNJ || c == ZWJ;
        }

        inline bool IsRegularExpressionChar(char16_t c)
        {
            return !IsLineTerminator(c) && c != u'\\' && c != u'/' && c != u'[';
        }

        inline bool IsRegularExpressionFirstChar(char16_t c)
        {
            return !IsLineTerminator(c) && c != u'*' && c != u'/';
        }

        inline bool IsRegularExpressionClassChar(char16_t c)
        {
            return !IsLineTerminator(c) && c != u']';
        }

        inline double Digit(char16_t c)
        {
            switch(c)
            {
                case u'0':
                case u'1':
                case u'2':
                case u'3':
                case u'4':
                case u'5':
                case u'6':
                case u'7':
                case u'8':
                case u'9':
                    return c - u'0';
                case u'A':
                case u'a':
                    return 10;
                case u'B':
                case u'b':
                    return 11;
                case u'C':
                case u'c':
                    return 12;
                case u'D':
                case u'd':
                    return 13;
                case u'E':
                case u'e':
                    return 14;
                case u'F':
                case u'f':
                    return 15;
                default:
                    assert(false);
            }
        }
    }

    class Token
    {
        public:
            // Order by section 7.5, 7.6, 7.7, 7.8
            enum Type
            {
                // Identifier
                TK_IDENT = 0,

                // Keywords
                TK_KEYWORD,

                // Future Reserved Words
                TK_FUTURE,

                // Punctuator
                TK_LBRACE,// {
                TK_RBRACE,// }
                TK_LPAREN,// (
                TK_RPAREN,// )
                TK_LBRACK,// [
                TK_RBRACK,// ]

                TK_DOT,// .
                TK_SEMICOLON,// ;
                TK_COMMA,// ,
                TK_QUESTION,// ?
                TK_COLON,// :

                TK_LT,// <
                TK_GT,// >
                TK_LE,// <=
                TK_GE,// >=
                TK_EQ,// ==
                TK_NE,// !=
                TK_EQ3,// ===
                TK_NE3,// !==

                TK_ADD,// +
                TK_SUB,// -
                TK_MUL,// *
                TK_MOD,// %
                TK_INC,// ++
                TK_DEC,// --

                TK_BIT_LSH,// <<
                TK_BIT_RSH,// >>
                TK_BIT_URSH,// >>>, unsigned right shift

                TK_BIT_AND,// &
                TK_BIT_OR,// |
                TK_BIT_XOR,// ^
                TK_BIT_NOT,// ~

                TK_LOGICAL_AND,// &&
                TK_LOGICAL_OR,// ||
                TK_LOGICAL_NOT,// !

                TK_ASSIGN,// =
                TK_ADD_ASSIGN,// +=
                TK_SUB_ASSIGN,// -=
                TK_MUL_ASSIGN,// *=
                TK_MOD_ASSIGN,// %=

                TK_BIT_LSH_ASSIGN,// <<=
                TK_BIT_RSH_ASSIGN,// >>=
                TK_BIT_URSH_ASSIGN,// >>>=
                TK_BIT_AND_ASSIGN,// &=
                TK_BIT_OR_ASSIGN,// |=
                TK_BIT_XOR_ASSIGN,// ^=

                // DivPunctuator
                TK_DIV,// /
                TK_DIV_ASSIGN,// /=

                // Null Literal
                TK_NULL,// null

                // Bool Literal
                TK_BOOL,// true & false

                // Number Literal
                TK_NUMBER,

                // String Literal
                TK_STRING,

                // Regular Expression Literal
                TK_REGEX,

                TK_LINE_TERM,

                TK_EOS,
                TK_NOT_FOUND,
                TK_ILLEGAL,
            };

        private:
            Type type_;
            std::u16string source_;

        public:
            Token(Type type, const std::u16string& source) : type_(type), source_(source)
            {
            }

            inline bool IsAssignmentOperator()
            {
                switch(type_)
                {
                    case TK_ASSIGN:// =
                    case TK_ADD_ASSIGN:// +=
                    case TK_SUB_ASSIGN:// -=
                    case TK_MUL_ASSIGN:// *=
                    case TK_MOD_ASSIGN:// %=
                    case TK_DIV_ASSIGN:// /=

                    case TK_BIT_LSH_ASSIGN:// <<=
                    case TK_BIT_RSH_ASSIGN:// >>=
                    case TK_BIT_URSH_ASSIGN:// >>>=
                    case TK_BIT_AND_ASSIGN:// &=
                    case TK_BIT_OR_ASSIGN:// |=
                    case TK_BIT_XOR_ASSIGN:// ^=
                        return true;
                    default:
                        return false;
                }
            }

            inline bool IsLineTerminator()
            {
                return type_ == TK_LINE_TERM;
            }

            inline bool IsIdentifierName()
            {
                return type_ == TK_IDENT || type_ == TK_KEYWORD || type_ == TK_FUTURE;
            }

            inline bool IsPropertyName()
            {
                return IsIdentifierName() || type_ == TK_STRING || type_ == TK_NUMBER;
            }

            inline bool IsSemiColon()
            {
                return type_ == TK_SEMICOLON;
            }

            inline bool IsIdentifier()
            {
                return type_ == TK_IDENT;
            }

            inline int BinaryPriority(bool no_in)
            {
                switch(type_)
                {
                    // Binary
                    case TK_LOGICAL_OR:// ||
                        return 2;
                    case TK_LOGICAL_AND:// &&
                        return 3;
                    case TK_BIT_OR:// |
                        return 4;
                    case TK_BIT_XOR:// ^
                        return 5;
                    case TK_BIT_AND:// &
                        return 6;
                    case TK_EQ:// ==
                    case TK_NE:// !=
                    case TK_EQ3:// ===
                    case TK_NE3:// !==
                        return 7;
                    case TK_LT:// <
                    case TK_GT:// >
                    case TK_LE:// <=
                    case TK_GE:// >=
                        return 8;
                    case TK_BIT_LSH:// <<
                    case TK_BIT_RSH:// >>
                    case TK_BIT_URSH:// >>>
                        return 9;
                    case TK_ADD:
                    case TK_SUB:
                        return 10;
                    case TK_MUL:
                    case TK_DIV:
                    case TK_MOD:
                        return 11;

                    case TK_KEYWORD:
                        if(source_ == u"instanceof")
                        {
                            return 8;
                            // To prevent parsing for(a in b).
                        }
                        else if(!no_in && source_ == u"in")
                        {
                            return 8;
                        }
                    default:
                        return -1;
                }
            }

            inline int UnaryPrefixPriority()
            {
                switch(type_)
                {
                    // Prefix
                    case TK_INC:
                    case TK_DEC:
                    case TK_ADD:
                    case TK_SUB:
                    case TK_BIT_NOT:
                    case TK_LOGICAL_NOT:
                        return 100;// UnaryExpresion always have higher priority.

                    case TK_KEYWORD:
                        if(source_ == u"delete" || source_ == u"void" || source_ == u"typeof")
                        {
                            return 100;
                        }
                    default:
                        return -1;
                }
            }

            inline int UnaryPostfixPriority()
            {
                switch(type_)
                {
                    // Prefix
                    case TK_INC:
                    case TK_DEC:
                        return 200;// UnaryExpresion always have higher priority.
                    default:
                        return -1;
                }
            }

            Type type()
            {
                return type_;
            }
            std::u16string source()
            {
                return source_;
            }
    };

    const std::array<std::u16string, 26> kKeywords = {
        u"break",   u"do",     u"instanceof", u"typeof",   u"case",  u"else",   u"new",   u"var",      u"catch",
        u"finally", u"return", u"void",       u"continue", u"for",   u"switch", u"while", u"debugger", u"function",
        u"this",    u"with",   u"default",    u"if",       u"throw", u"delete", u"in",    u"try",
    };

    const std::array<std::u16string, 7> kFutureReservedWords = {
        u"class", u"enum", u"extends", u"super", u"const", u"export", u"import",
    };

    const std::array<std::u16string, 9> kStrictModeFutureReservedWords = { u"implements",
                                                                           u"let",
                                                                           u"private",
                                                                           u"public",
                                                                           u"yield"
                                                                           u"interface",
                                                                           u"package",
                                                                           u"protected",
                                                                           u"static" };

    class Lexer
    {
        private:
            char16_t c_;
            size_t pos_;
            size_t end_;
            Token token_;
            std::u16string source_;

        private:
            inline char16_t LookAhead()
            {
                if(pos_ + 1 >= end_)
                {
                    return character::EOS;
                }
                return source_[pos_ + 1];
            }

            inline void Advance()
            {
                if(pos_ < end_)
                {
                    pos_++;
                }
                UpdateC();
            }

            inline void UpdateC()
            {
                if(pos_ < end_)
                {
                    c_ = source_[pos_];
                }
                else
                {
                    c_ = character::EOS;
                }
            }

            bool SkipRegularExpressionBackslashSequence()
            {
                assert(c_ == u'\\');
                Advance();
                if(character::IsLineTerminator(c_))
                {
                    return false;
                }
                Advance();
                return true;
            }

            void SkipRegularExpressionChars()
            {
                while(c_ != character::EOS && character::IsRegularExpressionChar(c_))
                {
                    Advance();
                }
            }

            bool SkipRegularExpressionClass()
            {
                assert(c_ == u'[');
                Advance();
                while(c_ != character::EOS && character::IsRegularExpressionClassChar(c_))
                {
                    switch(c_)
                    {
                        case u'\\':
                        {
                            if(!SkipRegularExpressionBackslashSequence())
                            {
                                return false;
                            }
                            break;
                        }
                        default:
                            Advance();
                    }
                }
                if(c_ == u']')
                {
                    return true;
                }
                return false;
            }

            void SkipMultiLineComment()
            {
                while(c_ != character::EOS)
                {
                    if(c_ == u'*')
                    {
                        Advance();
                        if(c_ == u'/')
                        {
                            Advance();
                            break;
                        }
                    }
                    else
                    {
                        Advance();
                    }
                }
            }

            void SkipSingleLineComment()
            {
                // This will not skip line terminators.
                while(c_ != character::EOS && !character::IsLineTerminator(c_))
                {
                    Advance();
                }
            }

            void SkipWhiteSpace()
            {
                while(character::IsWhiteSpace(c_))
                {
                    Advance();
                }
            }

            Token ScanLineTerminatorSequence()
            {
                assert(character::IsLineTerminator(c_));
                size_t start = pos_;
                if(c_ == character::CR && LookAhead() == character::LF)
                {
                    Advance();
                    Advance();
                }
                else
                {
                    Advance();
                }
                return Token(Token::TK_LINE_TERM, source_.substr(start, pos_ - start));
            }

            void SkipLineTerminatorSequence()
            {
                assert(character::IsLineTerminator(c_));
                if(c_ == character::CR && LookAhead() == character::LF)
                {
                    Advance();
                    Advance();
                }
                else
                {
                    Advance();
                }
            }

            Token ScanStringLiteral()
            {
                char16_t quote = c_;
                size_t start = pos_;
                Advance();
                while(c_ != character::EOS && c_ != quote && !character::IsLineTerminator(c_))
                {
                    switch(c_)
                    {
                        case u'\\':
                        {
                            Advance();
                            // TODO(zhuzilin) Find out if "\1" will trigger error.
                            switch(c_)
                            {
                                case u'0':
                                {
                                    Advance();
                                    if(character::IsDecimalDigit(LookAhead()))
                                    {
                                        Advance();
                                        goto error;
                                    }
                                    break;
                                }
                                case u'x':
                                {// HexEscapeSequence
                                    Advance();
                                    for(size_t i = 0; i < 2; i++)
                                    {
                                        if(!character::IsHexDigit(c_))
                                        {
                                            Advance();
                                            goto error;
                                        }
                                        Advance();
                                    }
                                    break;
                                }
                                case u'u':
                                {// UnicodeEscapeSequence
                                    if(!SkipUnicodeEscapeSequence())
                                    {
                                        Advance();
                                        goto error;
                                    }
                                    break;
                                }
                                default:
                                    if(character::IsLineTerminator(c_))
                                    {
                                        SkipLineTerminatorSequence();
                                    }
                                    else if(character::IsCharacterEscapeSequence(c_))
                                    {
                                        Advance();
                                    }
                                    else
                                    {
                                        Advance();
                                        goto error;
                                    }
                            }
                            break;
                        }
                        default:
                            Advance();
                    }
                }

                if(c_ == quote)
                {
                    Advance();
                    return Token(Token::Type::TK_STRING, source_.substr(start, pos_ - start));
                }
            error:
                return Token(Token::Type::TK_ILLEGAL, source_.substr(start, pos_ - start));
            }

            bool SkipAtLeastOneDecimalDigit()
            {
                if(!character::IsDecimalDigit(c_))
                {
                    return false;
                }
                while(character::IsDecimalDigit(c_))
                {
                    Advance();
                }
                return true;
            }

            bool SkipAtLeastOneHexDigit()
            {
                if(!character::IsHexDigit(c_))
                {
                    return false;
                }
                while(character::IsHexDigit(c_))
                {
                    Advance();
                }
                return true;
            }

            Token ScanNumericLiteral()
            {
                assert(c_ == u'.' || character::IsDecimalDigit(c_));
                size_t start = pos_;

                bool is_hex = false;
                switch(c_)
                {
                    case u'0':
                    {
                        Advance();
                        switch(c_)
                        {
                            case u'x':
                            case u'X':
                            {// HexIntegerLiteral
                                Advance();
                                if(!SkipAtLeastOneHexDigit())
                                {
                                    Advance();
                                    goto error;
                                }
                                is_hex = true;
                                break;
                            }
                            case u'.':
                            {
                                Advance();
                                if(!SkipAtLeastOneDecimalDigit())
                                {
                                    Advance();
                                    goto error;
                                }
                                break;
                            }
                        }
                        break;
                    }
                    case u'.':
                    {
                        Advance();
                        if(!SkipAtLeastOneDecimalDigit())
                        {
                            Advance();
                            goto error;
                        }
                        break;
                    }
                    default:// NonZeroDigit
                        SkipAtLeastOneDecimalDigit();
                        if(c_ == u'.')
                        {
                            Advance();
                            if(!SkipAtLeastOneDecimalDigit())
                            {
                                Advance();
                                goto error;
                            }
                        }
                }

                if(!is_hex)
                {// ExponentPart
                    if(c_ == u'e' || c_ == u'E')
                    {
                        Advance();
                        if(c_ == u'+' || c_ == u'-')
                        {
                            Advance();
                        }
                        if(!SkipAtLeastOneDecimalDigit())
                        {
                            Advance();
                            goto error;
                        }
                    }
                }

                // The source character immediately following a NumericLiteral must not
                // be an IdentifierStart or DecimalDigit.
                if(character::IsIdentifierStart(c_) || character::IsDecimalDigit(c_))
                {
                    Advance();
                    goto error;
                }
                return Token(Token::Type::TK_NUMBER, source_.substr(start, pos_ - start));
            error:
                return Token(Token::Type::TK_ILLEGAL, source_.substr(start, pos_ - start));
            }

            bool SkipUnicodeEscapeSequence()
            {
                if(c_ != u'u')
                {
                    return false;
                }
                Advance();
                for(size_t i = 0; i < 4; i++)
                {
                    if(!character::IsHexDigit(c_))
                    {
                        return false;
                    }
                    Advance();
                }
                return true;
            }

            Token ScanIdentifier()
            {
                assert(character::IsIdentifierStart(c_));
                size_t start = pos_;
                std::u16string source;
                if(c_ == u'\\')
                {
                    Advance();
                    if(!SkipUnicodeEscapeSequence())
                    {
                        Advance();
                        goto error;
                    }
                }
                else
                {
                    Advance();
                }

                while(character::IsIdentifierPart(c_))
                {
                    if(c_ == u'\\')
                    {
                        Advance();
                        if(!SkipUnicodeEscapeSequence())
                        {
                            Advance();
                            goto error;
                        }
                    }
                    else
                    {
                        Advance();
                    }
                }

                source = source_.substr(start, pos_ - start);
                if(source == u"null")
                {
                    return Token(Token::Type::TK_NULL, source);
                }
                if(source == u"true" || source == u"false")
                {
                    return Token(Token::Type::TK_BOOL, source);
                }
                for(auto keyword : kKeywords)
                {
                    if(source == keyword)
                    {
                        return Token(Token::Type::TK_KEYWORD, source);
                    }
                }
                for(auto future : kFutureReservedWords)
                {
                    if(source == future)
                    {
                        return Token(Token::Type::TK_FUTURE, source);
                    }
                    // TODO(zhuzilin) Check if source in kStrictModeFutureReservedWords
                    // when stric mode code is supported.
                }
                return Token(Token::Type::TK_IDENT, source);
            error:
                return Token(Token::Type::TK_ILLEGAL, source_.substr(start, pos_ - start));
            }

        public:
            Lexer(const std::u16string& source)
            : source_(source), pos_(0), end_(source.size()), token_(Token::Type::TK_NOT_FOUND, u"")
            {
                UpdateC();
            }

            Token Next(bool line_terminator = false)
            {
                Token token = Token(Token::Type::TK_NOT_FOUND, u"");
                do
                {
                    size_t start = pos_;
                    switch(c_)
                    {
                        case character::EOS:
                        {
                            token = Token(Token::Type::TK_EOS, source_.substr(pos_, 0));
                            break;
                        }

                        case u'{':
                        {
                            Advance();
                            token = Token(Token::Type::TK_LBRACE, source_.substr(start, 1));
                            break;
                        }
                        case u'}':
                        {
                            Advance();
                            token = Token(Token::Type::TK_RBRACE, source_.substr(start, 1));
                            break;
                        }
                        case u'(':
                        {
                            Advance();
                            token = Token(Token::Type::TK_LPAREN, source_.substr(start, 1));
                            break;
                        }
                        case u')':
                        {
                            Advance();
                            token = Token(Token::Type::TK_RPAREN, source_.substr(start, 1));
                            break;
                        }
                        case u'[':
                        {
                            Advance();
                            token = Token(Token::Type::TK_LBRACK, source_.substr(start, 1));
                            break;
                        }
                        case u']':
                        {
                            Advance();
                            token = Token(Token::Type::TK_RBRACK, source_.substr(start, 1));
                            break;
                        }
                        case u'.':
                        {
                            if(character::IsDecimalDigit(LookAhead()))
                            {
                                token = ScanNumericLiteral();
                            }
                            else
                            {
                                token = Token(Token::Type::TK_DOT, source_.substr(start, 1));
                                Advance();
                            }
                            break;
                        }
                        case u';':
                        {
                            Advance();
                            token = Token(Token::Type::TK_SEMICOLON, source_.substr(start, 1));
                            break;
                        }
                        case u',':
                        {
                            Advance();
                            token = Token(Token::Type::TK_COMMA, source_.substr(start, 1));
                            break;
                        }
                        case u'?':
                        {
                            Advance();
                            token = Token(Token::Type::TK_QUESTION, source_.substr(start, 1));
                            break;
                        }
                        case u':':
                        {
                            Advance();
                            token = Token(Token::Type::TK_COLON, source_.substr(start, 1));
                            break;
                        }

                        case u'<':
                        {
                            Advance();
                            switch(c_)
                            {
                                case u'<':
                                    Advance();
                                    switch(c_)
                                    {
                                        case u'=':// <<=
                                            Advance();
                                            token = Token(Token::Type::TK_BIT_LSH_ASSIGN, source_.substr(start, 3));
                                            break;
                                        default:// <<
                                            token = Token(Token::Type::TK_BIT_LSH, source_.substr(start, 2));
                                    }
                                    break;
                                case u'=':// <=
                                    Advance();
                                    token = Token(Token::Type::TK_LE, source_.substr(start, 2));
                                    break;
                                default:// <
                                    token = Token(Token::Type::TK_LT, source_.substr(start, 1));
                            }
                            break;
                        }
                        case u'>':
                        {
                            Advance();
                            switch(c_)
                            {
                                case u'>':
                                    Advance();
                                    switch(c_)
                                    {
                                        case u'>':
                                            Advance();
                                            switch(c_)
                                            {
                                                case u'=':// >>>=
                                                    Advance();
                                                    token = Token(Token::Type::TK_BIT_URSH_ASSIGN, source_.substr(start, 4));
                                                    break;
                                                default:// >>>
                                                    token = Token(Token::Type::TK_BIT_URSH, source_.substr(start, 3));
                                            }
                                            break;
                                        case u'=':// >>=
                                            token = Token(Token::Type::TK_BIT_RSH_ASSIGN, source_.substr(start, 3));
                                            Advance();
                                            break;
                                        default:// >>
                                            token = Token(Token::Type::TK_BIT_RSH, source_.substr(start, 2));
                                    }
                                    break;
                                case u'=':// >=
                                    Advance();
                                    token = Token(Token::Type::TK_GE, source_.substr(start, 2));
                                    break;
                                default:// >
                                    token = Token(Token::Type::TK_GT, source_.substr(start, 1));
                            }
                            break;
                        }
                        case u'=':
                        {
                            Advance();
                            switch(c_)
                            {
                                case u'=':
                                    Advance();
                                    switch(c_)
                                    {
                                        case u'=':// ===
                                            Advance();
                                            token = Token(Token::Type::TK_EQ3, source_.substr(start, 3));
                                            break;
                                        default:// ==
                                            token = Token(Token::Type::TK_EQ, source_.substr(start, 2));
                                            break;
                                    }
                                    break;
                                default:// =
                                    token = Token(Token::Type::TK_ASSIGN, source_.substr(start, 1));
                            }
                            break;
                        }
                        case u'!':
                        {
                            Advance();
                            switch(c_)
                            {
                                case u'=':
                                    Advance();
                                    switch(c_)
                                    {
                                        case u'=':// !==
                                            Advance();
                                            token = Token(Token::Type::TK_NE3, source_.substr(start, 3));
                                            break;
                                        default:// !=
                                            token = Token(Token::Type::TK_NE, source_.substr(start, 2));
                                            break;
                                    }
                                    break;
                                default:// !
                                    token = Token(Token::Type::TK_LOGICAL_NOT, source_.substr(start, 1));
                            }
                            break;
                        }

                        case u'+':
                        {
                            Advance();
                            switch(c_)
                            {
                                case u'+':// ++
                                    Advance();
                                    token = Token(Token::Type::TK_INC, source_.substr(start, 2));
                                    break;
                                case u'=':// +=
                                    Advance();
                                    token = Token(Token::Type::TK_ADD_ASSIGN, source_.substr(start, 2));
                                    break;
                                default:// +
                                    token = Token(Token::Type::TK_ADD, source_.substr(start, 1));
                            }
                            break;
                        }
                        case u'-':
                        {
                            Advance();
                            switch(c_)
                            {
                                case u'-':// --
                                    Advance();
                                    token = Token(Token::Type::TK_DEC, source_.substr(start, 2));
                                    break;
                                case u'=':// -=
                                    Advance();
                                    token = Token(Token::Type::TK_SUB_ASSIGN, source_.substr(start, 2));
                                default:// -
                                    token = Token(Token::Type::TK_SUB, source_.substr(start, 1));
                            }
                            break;
                        }
                        case u'*':
                        {
                            Advance();
                            if(c_ == u'=')
                            {// *=
                                Advance();
                                token = Token(Token::Type::TK_MUL_ASSIGN, source_.substr(start, 2));
                            }
                            else
                            {// +
                                token = Token(Token::Type::TK_MUL, source_.substr(start, 1));
                            }
                            break;
                        }
                        case u'%':
                        {
                            Advance();
                            if(c_ == u'=')
                            {// %=
                                Advance();
                                token = Token(Token::Type::TK_MOD_ASSIGN, source_.substr(start, 2));
                            }
                            else
                            {// %
                                token = Token(Token::Type::TK_MOD, source_.substr(start, 1));
                            }
                            break;
                        }

                        case u'&':
                        {
                            Advance();
                            switch(c_)
                            {
                                case u'&':// &&
                                    Advance();
                                    token = Token(Token::Type::TK_LOGICAL_AND, source_.substr(start, 2));
                                    break;
                                case u'=':// &=
                                    Advance();
                                    token = Token(Token::Type::TK_BIT_AND_ASSIGN, source_.substr(start, 2));
                                    break;
                                default:// &
                                    token = Token(Token::Type::TK_BIT_AND, source_.substr(start, 1));
                            }
                            break;
                        }
                        case u'|':
                        {
                            Advance();
                            switch(c_)
                            {
                                case u'|':// ||
                                    Advance();
                                    token = Token(Token::Type::TK_LOGICAL_OR, source_.substr(start, 2));
                                    break;
                                case u'=':// |=
                                    Advance();
                                    token = Token(Token::Type::TK_BIT_OR_ASSIGN, source_.substr(start, 2));
                                    break;
                                default:// |
                                    token = Token(Token::Type::TK_BIT_OR, source_.substr(start, 1));
                            }
                            break;
                        }
                        case u'^':
                        {
                            Advance();
                            if(c_ == u'=')
                            {// ^=
                                Advance();
                                token = Token(Token::Type::TK_BIT_XOR_ASSIGN, source_.substr(start, 2));
                            }
                            else
                            {
                                token = Token(Token::Type::TK_BIT_XOR, source_.substr(start, 1));
                            }
                            break;
                        }
                        case u'~':
                        {
                            Advance();
                            token = Token(Token::Type::TK_BIT_NOT, source_.substr(start, 1));
                            break;
                        }

                        case u'/':
                        {
                            Advance();
                            switch(c_)
                            {
                                case u'*':// /*
                                    Advance();
                                    SkipMultiLineComment();
                                    break;
                                case u'/':// //
                                    Advance();
                                    SkipSingleLineComment();
                                    break;
                                case u'=':// /=
                                    Advance();
                                    token = Token(Token::Type::TK_DIV_ASSIGN, source_.substr(start, 2));
                                    break;
                                default:// /
                                    // We cannot distinguish DIV and regex in lexer level and therefore,
                                    // we need to check if the symbol of div operator or start of regex
                                    // in parser.
                                    token = Token(Token::Type::TK_DIV, source_.substr(start, 1));
                            }
                            break;
                        }

                        case u'\'':
                        case u'"':
                        {
                            token = ScanStringLiteral();
                            break;
                        }

                        default:
                            if(character::IsWhiteSpace(c_))
                            {
                                SkipWhiteSpace();
                            }
                            else if(character::IsLineTerminator(c_))
                            {
                                if(line_terminator)
                                {
                                    token = ScanLineTerminatorSequence();
                                }
                                else
                                {
                                    SkipLineTerminatorSequence();
                                }
                            }
                            else if(character::IsDecimalDigit(c_))
                            {
                                token = ScanNumericLiteral();
                            }
                            else if(character::IsIdentifierStart(c_))
                            {
                                token = ScanIdentifier();
                            }
                            else
                            {
                                Advance();
                                token = Token(Token::TK_ILLEGAL, source_.substr(start, 1));
                            }
                    }
                } while(token.type() == Token::Type::TK_NOT_FOUND);
                token_ = token;
                return token_;
            }

            inline Token Last()
            {
                return token_;
            }
            inline size_t Pos()
            {
                return pos_;
            }

            void Rewind(size_t pos, Token token)
            {
                pos_ = pos;
                token_ = token;
                UpdateC();
            }

            Token NextAndRewind(bool line_terminator = false)
            {
                size_t old_pos = Pos();
                Token old_token = Last();
                Token token = Next(line_terminator);
                Rewind(old_pos, old_token);
                return token;
            }

            bool LineTermAhead()
            {
                return NextAndRewind(true).IsLineTerminator();
            }

            bool TrySkipSemiColon()
            {
                Token token = NextAndRewind();
                if(token.IsSemiColon())
                {
                    Next();
                    return true;
                }
                // 7.9 Automatic Semicolon Insertion
                if(token.type() == Token::TK_EOS || token.type() == Token::TK_RBRACE || LineTermAhead())
                    return true;
                return false;
            }

            Token ScanRegexLiteral()
            {
                assert(c_ == u'/');
                size_t start = pos_;
                Advance();
                if(!character::IsRegularExpressionFirstChar(c_))
                {
                    Advance();
                    goto error;
                }
                while(c_ != character::EOS && c_ != u'/' && !character::IsLineTerminator(c_))
                {
                    switch(c_)
                    {
                        case u'\\':
                        {// Regular Expression
                            if(!SkipRegularExpressionBackslashSequence())
                            {
                                Advance();
                                goto error;
                            }
                            break;
                        }
                        case u'[':
                        {
                            if(!SkipRegularExpressionClass())
                            {
                                Advance();
                                goto error;
                            }
                            break;
                        }
                        default:
                            SkipRegularExpressionChars();
                    }
                }

                if(c_ == u'/')
                {
                    Advance();
                    // RegularExpressionFlags
                    while(character::IsIdentifierPart(c_))
                    {
                        if(c_ == u'\\')
                        {
                            Advance();
                            if(!SkipUnicodeEscapeSequence())
                            {
                                Advance();
                                goto error;
                            }
                        }
                        else
                        {
                            Advance();
                        }
                    }
                    token_ = Token(Token::Type::TK_REGEX, source_.substr(start, pos_ - start));
                    return token_;
                }
            error:
                token_ = Token(Token::Type::TK_ILLEGAL, source_.substr(start, pos_ - start));
                return token_;
            }

            // For regex
            inline void Back()
            {
                if(pos_ == 0)
                    return;
                pos_--;
                UpdateC();
            }
    };

    class AST
    {
        public:
            enum Type
            {
                AST_EXPR_THIS,
                AST_EXPR_IDENT,

                AST_EXPR_NULL,
                AST_EXPR_BOOL,
                AST_EXPR_NUMBER,
                AST_EXPR_STRING,
                AST_EXPR_REGEX,

                AST_EXPR_ARRAY,
                AST_EXPR_OBJ,

                AST_EXPR_PAREN,// ( Expression )

                AST_EXPR_BINARY,
                AST_EXPR_UNARY,
                AST_EXPR_TRIPLE,

                AST_EXPR_ARGS,
                AST_EXPR_LHS,

                AST_EXPR,

                AST_FUNC,

                AST_STMT_EMPTY,
                AST_STMT_BLOCK,
                AST_STMT_IF,
                AST_STMT_WHILE,
                AST_STMT_FOR,
                AST_STMT_FOR_IN,
                AST_STMT_WITH,
                AST_STMT_DO_WHILE,
                AST_STMT_TRY,

                AST_STMT_VAR,
                AST_STMT_VAR_DECL,

                AST_STMT_CONTINUE,
                AST_STMT_BREAK,
                AST_STMT_RETURN,
                AST_STMT_THROW,

                AST_STMT_SWITCH,

                AST_STMT_LABEL,
                AST_STMT_DEBUG,

                AST_PROGRAM,
                AST_FUNC_BODY,

                AST_ILLEGAL,
            };

        private:
            Type type_;
            std::u16string source_;
            std::u16string label_;

        public:

            AST(Type type, const std::u16string& source = u"") : type_(type), source_(source)
            {
            }
            virtual ~AST(){};

            Type type()
            {
                return type_;
            }
            std::u16string source()
            {
                return source_;
            }

            void SetSource(const std::u16string& source)
            {
                source_ = source;
            }

            bool IsIllegal()
            {
                return type_ == AST_ILLEGAL;
            }

            std::u16string label()
            {
                return label_;
            }
            void SetLabel(const std::u16string& label)
            {
                label_ = label;
            }
    };

    class ArrayLiteral : public AST
    {
    public:
        ArrayLiteral() : AST(AST_EXPR_ARRAY), len_(0)
        {
        }

        ~ArrayLiteral() override
        {
            for(auto pair : elements_)
            {
                delete pair.second;
            }
        }

        size_t length()
        {
            return len_;
        }
        std::vector<std::pair<size_t, AST*>> elements()
        {
            return elements_;
        }

        void AddElement(AST* element)
        {
            if(element != nullptr)
            {
                elements_.emplace_back(len_, element);
            }
            len_++;
        }

    private:
        std::vector<std::pair<size_t, AST*>> elements_;
        size_t len_;
    };

    class ObjectLiteral : public AST
    {
    public:
        ObjectLiteral() : AST(AST_EXPR_OBJ)
        {
        }

        ~ObjectLiteral() override
        {
            for(auto property : properties_)
            {
                delete property.value;
            }
        }

        struct Property
        {
            enum Type
            {
                NORMAL = 0,
                GET,
                SET,
            };

            Property(Token k, AST* v, Type t) : key(k), type(t), value(v)
            {
            }

            Token key;
            AST* value;
            Type type;
        };

        void AddProperty(Property p)
        {
            properties_.emplace_back(p);
        }

        std::vector<Property> properties()
        {
            return properties_;
        }

        size_t length()
        {
            return properties_.size();
        }

    private:
        std::vector<Property> properties_;
    };

    class Paren : public AST
    {
    public:
        Paren(AST* expr, const std::u16string& source) : AST(AST_EXPR_PAREN, source), expr_(expr)
        {
        }

        AST* expr()
        {
            return expr_;
        }

    private:
        AST* expr_;
    };

    class Binary : public AST
    {
    public:
        Binary(AST* lhs, AST* rhs, Token op, const std::u16string& source = u"")
        : AST(AST_EXPR_BINARY, source), lhs_(lhs), rhs_(rhs), op_(op)
        {
        }

        ~Binary() override
        {
            delete lhs_;
            delete rhs_;
        }

        AST* lhs()
        {
            return lhs_;
        }
        AST* rhs()
        {
            return rhs_;
        }
        std::u16string op()
        {
            return op_.source();
        }

    private:
        AST* lhs_;
        AST* rhs_;
        Token op_;
    };

    class Unary : public AST
    {
    public:
        Unary(AST* node, Token op, bool prefix) : AST(AST_EXPR_UNARY), node_(node), op_(op), prefix_(prefix)
        {
        }

        ~Unary() override
        {
            delete node_;
        }

        AST* node()
        {
            return node_;
        }
        Token op()
        {
            return op_;
        }
        bool prefix()
        {
            return prefix_;
        }

    private:
        AST* node_;
        Token op_;
        bool prefix_;
    };

    class TripleCondition : public AST
    {
    public:
        TripleCondition(AST* cond, AST* true_expr, AST* false_expr)
        : AST(AST_EXPR_TRIPLE), cond_(cond), true_expr_(true_expr), false_expr_(false_expr)
        {
        }

        ~TripleCondition() override
        {
            delete cond_;
            delete true_expr_;
            delete false_expr_;
        }

        AST* cond()
        {
            return cond_;
        }
        AST* true_expr()
        {
            return true_expr_;
        }
        AST* false_expr()
        {
            return false_expr_;
        }

    private:
        AST* cond_;
        AST* true_expr_;
        AST* false_expr_;
    };

    class Expression : public AST
    {
    public:
        Expression() : AST(AST_EXPR)
        {
        }
        ~Expression() override
        {
            for(auto element : elements_)
            {
                delete element;
            }
        }

        void AddElement(AST* element)
        {
            elements_.push_back(element);
        }

        std::vector<AST*> elements()
        {
            return elements_;
        }

    private:
        std::vector<AST*> elements_;
    };

    class Arguments : public AST
    {
    public:
        Arguments(const std::vector<AST*>& args) : AST(AST_EXPR_ARGS), args_(args)
        {
        }

        ~Arguments() override
        {
            for(auto arg : args_)
                delete arg;
        }

        std::vector<AST*> args()
        {
            return args_;
        }

    private:
        std::vector<AST*> args_;
    };

    class LHS : public AST
    {
    public:
        LHS(AST* base, size_t new_count) : AST(AST_EXPR_LHS), base_(base), new_count_(new_count)
        {
        }

        ~LHS() override
        {
            for(auto args : args_list_)
                delete args;
            for(auto index : index_list_)
                delete index;
        }

        enum PostfixType
        {
            CALL,
            INDEX,
            PROP,
        };

        void AddArguments(Arguments* args)
        {
            order_.emplace_back(std::make_pair(args_list_.size(), CALL));
            args_list_.emplace_back(args);
        }

        void AddIndex(AST* index)
        {
            order_.emplace_back(std::make_pair(index_list_.size(), INDEX));
            index_list_.emplace_back(index);
        }

        void AddProp(Token prop_name)
        {
            order_.emplace_back(std::make_pair(prop_name_list_.size(), PROP));
            prop_name_list_.emplace_back(prop_name.source());
        }

        AST* base()
        {
            return base_;
        }
        size_t new_count()
        {
            return new_count_;
        }
        std::vector<std::pair<size_t, PostfixType>> order()
        {
            return order_;
        }
        std::vector<Arguments*> args_list()
        {
            return args_list_;
        }
        std::vector<AST*> index_list()
        {
            return index_list_;
        }
        std::vector<std::u16string> prop_name_list()
        {
            return prop_name_list_;
        }

    private:
        AST* base_;
        size_t new_count_;

        std::vector<std::pair<size_t, PostfixType>> order_;
        std::vector<Arguments*> args_list_;
        std::vector<AST*> index_list_;
        std::vector<std::u16string> prop_name_list_;
    };

    class Function : public AST
    {
    public:
        Function(const std::vector<std::u16string>& params, AST* body, const std::u16string& source)
        : Function(Token(Token::TK_NOT_FOUND, u""), params, body, source)
        {
        }

        Function(Token name, const std::vector<std::u16string>& params, AST* body, const std::u16string& source)
        : AST(AST_FUNC, source), name_(name), params_(params)
        {
            assert(body->type() == AST::AST_FUNC_BODY);
            body_ = body;
        }

        ~Function() override
        {
            delete body_;
        }

        bool is_named()
        {
            return name_.type() != Token::TK_NOT_FOUND;
        }
        std::u16string name()
        {
            return name_.source();
        }
        std::vector<std::u16string> params()
        {
            return params_;
        }
        AST* body()
        {
            return body_;
        }

    private:
        Token name_;
        std::vector<std::u16string> params_;
        AST* body_;
    };

    class ProgramOrFunctionBody : public AST
    {
    public:
        ProgramOrFunctionBody(Type type, bool strict) : AST(type), strict_(strict)
        {
        }
        ~ProgramOrFunctionBody() override
        {
            for(auto func_decl : func_decls_)
                delete func_decl;
            for(auto stmt : stmts_)
                delete stmt;
        }

        void AddFunctionDecl(AST* func)
        {
            assert(func->type() == AST_FUNC);
            func_decls_.emplace_back(static_cast<Function*>(func));
        }
        void AddStatement(AST* stmt)
        {
            stmts_.emplace_back(stmt);
        }

        bool strict()
        {
            return strict_;
        }
        std::vector<Function*> func_decls()
        {
            return func_decls_;
        }
        std::vector<AST*> statements()
        {
            return stmts_;
        }

    private:
        bool strict_;
        std::vector<Function*> func_decls_;
        std::vector<AST*> stmts_;
    };

    class LabelledStmt : public AST
    {
    public:
        LabelledStmt(Token label, AST* stmt, const std::u16string& source)
        : AST(AST_STMT_LABEL, source), label_(label), stmt_(stmt)
        {
        }
        ~LabelledStmt()
        {
            delete stmt_;
        }

        std::u16string label()
        {
            return label_.source();
        }
        AST* statement()
        {
            return stmt_;
        }

    private:
        Token label_;
        AST* stmt_;
    };

    class ContinueOrBreak : public AST
    {
    public:
        ContinueOrBreak(Type type, const std::u16string& source)
        : ContinueOrBreak(type, Token(Token::TK_NOT_FOUND, u""), source)
        {
        }

        ContinueOrBreak(Type type, Token ident, const std::u16string& source) : AST(type, source), ident_(ident)
        {
        }

        std::u16string ident()
        {
            return ident_.source();
        }

    private:
        Token ident_;
    };

    class Return : public AST
    {
    public:
        Return(AST* expr, const std::u16string& source) : AST(AST_STMT_RETURN, source), expr_(expr)
        {
        }
        ~Return()
        {
            if(expr_ != nullptr)
                delete expr_;
        }

        AST* expr()
        {
            return expr_;
        }

    private:
        AST* expr_;
    };

    class Throw : public AST
    {
    public:
        Throw(AST* expr, const std::u16string& source) : AST(AST_STMT_THROW, source), expr_(expr)
        {
        }
        ~Throw()
        {
            if(expr_ != nullptr)
                delete expr_;
        }

        AST* expr()
        {
            return expr_;
        }

    private:
        AST* expr_;
    };

    class VarDecl : public AST
    {
    public:
        VarDecl(Token ident, const std::u16string& source) : VarDecl(ident, nullptr, source)
        {
        }

        VarDecl(Token ident, AST* init, const std::u16string& source)
        : AST(AST_STMT_VAR_DECL, source), ident_(ident), init_(init)
        {
        }
        ~VarDecl()
        {
            delete init_;
        }

        std::u16string ident()
        {
            return ident_.source();
        }
        AST* init()
        {
            return init_;
        }

    private:
        Token ident_;
        AST* init_;
    };

    class VarStmt : public AST
    {
    public:
        VarStmt() : AST(AST_STMT_VAR)
        {
        }
        ~VarStmt()
        {
            for(auto decl : decls_)
                delete decl;
        }

        void AddDecl(AST* decl)
        {
            assert(decl->type() == AST_STMT_VAR_DECL);
            decls_.emplace_back(static_cast<VarDecl*>(decl));
        }

        std::vector<VarDecl*> decls()
        {
            return decls_;
        }

    public:
        std::vector<VarDecl*> decls_;
    };

    class Block : public AST
    {
    public:
        Block() : AST(AST_STMT_BLOCK)
        {
        }
        ~Block()
        {
            for(auto stmt : stmts_)
                delete stmt;
        }

        void AddStatement(AST* stmt)
        {
            stmts_.emplace_back(stmt);
        }

        std::vector<AST*> statements()
        {
            return stmts_;
        }

    public:
        std::vector<AST*> stmts_;
    };

    class Try : public AST
    {
    public:
        Try(AST* try_block, Token catch_ident, AST* catch_block, const std::u16string& source)
        : Try(try_block, catch_ident, catch_block, nullptr, source)
        {
        }

        Try(AST* try_block, AST* finally_block, const std::u16string& source)
        : Try(try_block, Token(Token::TK_NOT_FOUND, u""), nullptr, finally_block, source)
        {
        }

        Try(AST* try_block, Token catch_ident, AST* catch_block, AST* finally_block, const std::u16string& source)
        : AST(AST_STMT_TRY, source), try_block_(try_block), catch_ident_(catch_ident), catch_block_(catch_block),
          finally_block_(finally_block)
        {
        }

        ~Try()
        {
            delete try_block_;
            if(catch_block_ != nullptr)
                delete catch_block_;
            if(finally_block_ != nullptr)
                delete finally_block_;
        }

        AST* try_block()
        {
            return try_block_;
        }
        std::u16string catch_ident()
        {
            return catch_ident_.source();
        };
        AST* catch_block()
        {
            return catch_block_;
        }
        AST* finally_block()
        {
            return finally_block_;
        }

    public:
        AST* try_block_;
        Token catch_ident_;
        AST* catch_block_;
        AST* finally_block_;
    };

    class If : public AST
    {
    public:
        If(AST* cond, AST* if_block, const std::u16string& source) : If(cond, if_block, nullptr, source)
        {
        }

        If(AST* cond, AST* if_block, AST* else_block, const std::u16string& source)
        : AST(AST_STMT_IF, source), cond_(cond), if_block_(if_block), else_block_(else_block)
        {
        }
        ~If()
        {
            delete cond_;
            delete if_block_;
            if(else_block_ != nullptr)
                delete else_block_;
        }

        AST* cond()
        {
            return cond_;
        }
        AST* if_block()
        {
            return if_block_;
        }
        AST* else_block()
        {
            return else_block_;
        }

    public:
        AST* cond_;
        AST* if_block_;
        AST* else_block_;
    };

    class WhileOrWith : public AST
    {
    public:
        WhileOrWith(Type type, AST* expr, AST* stmt, const std::u16string& source)
        : AST(type, source), expr_(expr), stmt_(stmt)
        {
        }
        ~WhileOrWith()
        {
            delete expr_;
            delete stmt_;
        }

        AST* expr()
        {
            return expr_;
        }
        AST* stmt()
        {
            return stmt_;
        }

    public:
        AST* expr_;
        AST* stmt_;
    };

    class DoWhile : public AST
    {
    public:
        DoWhile(AST* expr, AST* stmt, const std::u16string& source) : AST(AST_STMT_DO_WHILE, source), expr_(expr), stmt_(stmt)
        {
        }
        ~DoWhile()
        {
            delete expr_;
            delete stmt_;
        }

        AST* expr()
        {
            return expr_;
        }
        AST* stmt()
        {
            return stmt_;
        }

    public:
        AST* expr_;
        AST* stmt_;
    };

    class Switch : public AST
    {
    public:
        Switch() : AST(AST_STMT_SWITCH)
        {
        }

        ~Switch() override
        {
            for(CaseClause clause : before_default_case_clauses_)
            {
                delete clause.expr;
                for(auto stmt : clause.stmts)
                {
                    delete stmt;
                }
            }
            for(CaseClause clause : after_default_case_clauses_)
            {
                delete clause.expr;
                for(auto stmt : clause.stmts)
                {
                    delete stmt;
                }
            }
            for(auto stmt : default_clause_.stmts)
            {
                delete stmt;
            }
        }

        void SetExpr(AST* expr)
        {
            expr_ = expr;
        }

        struct DefaultClause
        {
            std::vector<AST*> stmts;
        };

        struct CaseClause
        {
            CaseClause(AST* expr, const std::vector<AST*>& stmts) : expr(expr), stmts(stmts)
            {
            }
            AST* expr;
            std::vector<AST*> stmts;
        };

        void SetDefaultClause(const std::vector<AST*>& stmts)
        {
            assert(!has_default_clause());
            has_default_clause_ = true;
            default_clause_.stmts = stmts;
        }

        void AddBeforeDefaultCaseClause(CaseClause c)
        {
            before_default_case_clauses_.emplace_back(c);
        }

        void AddAfterDefaultCaseClause(CaseClause c)
        {
            after_default_case_clauses_.emplace_back(c);
        }

        AST* expr()
        {
            return expr_;
        }
        std::vector<CaseClause> before_default_case_clauses()
        {
            return before_default_case_clauses_;
        }
        bool has_default_clause()
        {
            return has_default_clause_;
        }
        DefaultClause default_clause()
        {
            assert(has_default_clause());
            return default_clause_;
        }
        std::vector<CaseClause> after_default_case_clauses()
        {
            return after_default_case_clauses_;
        }

    private:
        AST* expr_;
        bool has_default_clause_ = false;
        DefaultClause default_clause_;
        std::vector<CaseClause> before_default_case_clauses_;
        std::vector<CaseClause> after_default_case_clauses_;
    };

    class For : public AST
    {
    public:
        For(const std::vector<AST*>& expr0s, AST* expr1, AST* expr2, AST* stmt, const std::u16string& source)
        : AST(AST_STMT_FOR, source), expr0s_(expr0s), expr1_(expr1), expr2_(expr2), stmt_(stmt)
        {
        }

        std::vector<AST*> expr0s()
        {
            return expr0s_;
        }
        AST* expr1()
        {
            return expr1_;
        }
        AST* expr2()
        {
            return expr2_;
        }
        AST* statement()
        {
            return stmt_;
        }

    private:
        std::vector<AST*> expr0s_;
        AST* expr1_;
        AST* expr2_;

        AST* stmt_;
    };

    class ForIn : public AST
    {
    public:
        ForIn(AST* expr0, AST* expr1, AST* stmt, const std::u16string& source)
        : AST(AST_STMT_FOR_IN, source), expr0_(expr0), expr1_(expr1), stmt_(stmt)
        {
        }

        AST* expr0()
        {
            return expr0_;
        }
        AST* expr1()
        {
            return expr1_;
        }
        AST* statement()
        {
            return stmt_;
        }

    private:
        AST* expr0_;
        AST* expr1_;

        AST* stmt_;
    };

    class Parser
    {
    public:
        Parser(const std::u16string& source) : source_(source), lexer_(source)
        {
        }

        AST* ParsePrimaryExpression()
        {
            Token token = lexer_.NextAndRewind();
            switch(token.type())
            {
                case Token::TK_KEYWORD:
                    if(token.source() == u"this")
                    {
                        lexer_.Next();
                        return new AST(AST::AST_EXPR_THIS, token.source());
                    }
                    goto error;
                case Token::TK_IDENT:
                    lexer_.Next();
                    return new AST(AST::AST_EXPR_IDENT, token.source());
                case Token::TK_NULL:
                    lexer_.Next();
                    return new AST(AST::AST_EXPR_NULL, token.source());
                case Token::TK_BOOL:
                    lexer_.Next();
                    return new AST(AST::AST_EXPR_BOOL, token.source());
                case Token::TK_NUMBER:
                    lexer_.Next();
                    return new AST(AST::AST_EXPR_NUMBER, token.source());
                case Token::TK_STRING:
                    lexer_.Next();
                    return new AST(AST::AST_EXPR_STRING, token.source());
                case Token::TK_LBRACK:// [
                    return ParseArrayLiteral();
                case Token::TK_LBRACE:// {
                    return ParseObjectLiteral();
                case Token::TK_LPAREN:
                {// (
                    lexer_.Next();// skip (
                    AST* value = ParseExpression(false);
                    if(value->IsIllegal())
                        return value;
                    if(lexer_.Next().type() != Token::TK_RPAREN)
                    {
                        delete value;
                        goto error;
                    }
                    return new Paren(value, value->source());
                }
                case Token::TK_DIV:
                {// /
                    lexer_.Next();// skip /
                    lexer_.Back();// back to /
                    token = lexer_.ScanRegexLiteral();
                    if(token.type() == Token::TK_REGEX)
                    {
                        return new AST(AST::AST_EXPR_REGEX, token.source());
                    }
                    else
                    {
                        goto error;
                    }
                    break;
                }
                default:
                    goto error;
            }

        error:
            return new AST(AST::AST_ILLEGAL, token.source());
        }

        std::vector<std::u16string> ParseFormalParameterList()
        {
            assert(lexer_.NextAndRewind().IsIdentifier());
            std::vector<std::u16string> params;
            params.emplace_back(lexer_.Next().source());
            Token token = lexer_.NextAndRewind();
            // NOTE(zhuzilin) the EOS is for new Function("a,b,c", "")
            while(token.type() != Token::TK_RPAREN && token.type() != Token::TK_EOS)
            {
                if(token.type() != Token::TK_COMMA)
                {
                    return {};
                }
                lexer_.Next();// skip ,
                token = lexer_.Next();
                if(token.type() != Token::TK_IDENT)
                {
                    return {};
                }
                params.emplace_back(token.source());
                token = lexer_.NextAndRewind();
            }
            return params;
        }

        AST* ParseFunction(bool must_be_named)
        {
            START_POS;
            assert(lexer_.Next().source() == u"function");

            Token name(Token::TK_NOT_FOUND, u"");
            std::vector<std::u16string> params;
            AST* tmp;
            AST* body;
            Function* func;

            // Identifier_opt
            Token token = lexer_.Next();
            if(token.type() == Token::TK_IDENT)
            {
                name = token;
                token = lexer_.Next();// skip "("
            }
            else if(must_be_named)
            {
                goto error;
            }
            if(token.type() != Token::TK_LPAREN)
            {
                goto error;
            }
            token = lexer_.NextAndRewind();
            if(token.type() == Token::TK_IDENT)
            {
                params = ParseFormalParameterList();
            }
            if(lexer_.Next().type() != Token::TK_RPAREN)
            {// skip )
                goto error;
            }
            token = lexer_.Next();// skip {
            if(token.type() != Token::TK_LBRACE)
            {
                goto error;
            }
            body = ParseFunctionBody();
            if(body->IsIllegal())
                return body;

            token = lexer_.Next();// skip }
            if(token.type() != Token::TK_RBRACE)
            {
                goto error;
            }

            if(name.type() == Token::TK_NOT_FOUND)
            {
                func = new Function(params, body, SOURCE_PARSED);
            }
            else
            {
                func = new Function(name, params, body, SOURCE_PARSED);
            }

            return func;
        error:
            return new AST(AST::AST_ILLEGAL, SOURCE_PARSED);
        }

        AST* ParseArrayLiteral()
        {
            START_POS;
            assert(lexer_.Next().type() == Token::TK_LBRACK);

            ArrayLiteral* array = new ArrayLiteral();
            AST* element = nullptr;

            Token token = lexer_.NextAndRewind();
            while(token.type() != Token::TK_RBRACK)
            {
                switch(token.type())
                {
                    case Token::TK_COMMA:
                        lexer_.Next();
                        array->AddElement(element);
                        element = nullptr;
                        break;
                    default:
                        element = ParseAssignmentExpression(false);
                        if(element->type() == AST::AST_ILLEGAL)
                        {
                            return element;
                        }
                }
                token = lexer_.NextAndRewind();
            }
            if(element != nullptr)
            {
                array->AddElement(element);
            }
            assert(token.type() == Token::TK_RBRACK);
            assert(lexer_.Next().type() == Token::TK_RBRACK);
            array->SetSource(SOURCE_PARSED);
            return array;
        error:
            delete array;
            return new AST(AST::AST_ILLEGAL, SOURCE_PARSED);
        }

        AST* ParseObjectLiteral()
        {
            START_POS;
            assert(lexer_.Next().type() == Token::TK_LBRACE);

            ObjectLiteral* obj = new ObjectLiteral();
            Token token = lexer_.NextAndRewind();
            while(token.type() != Token::TK_RBRACE)
            {
                if(token.IsPropertyName())
                {
                    lexer_.Next();
                    if((token.source() == u"get" || token.source() == u"set") && lexer_.NextAndRewind().IsPropertyName())
                    {
                        START_POS;
                        ObjectLiteral::Property::Type type;
                        if(token.source() == u"get")
                            type = ObjectLiteral::Property::GET;
                        else
                            type = ObjectLiteral::Property::SET;
                        Token key = lexer_.Next();// skip property name
                        if(!key.IsPropertyName())
                        {
                            goto error;
                        }
                        if(lexer_.Next().type() != Token::TK_LPAREN)
                        {
                            goto error;
                        }
                        std::vector<std::u16string> params;
                        if(type == ObjectLiteral::Property::SET)
                        {
                            Token param = lexer_.Next();
                            if(!param.IsIdentifier())
                            {
                                goto error;
                            }
                            params.emplace_back(param.source());
                        }
                        if(lexer_.Next().type() != Token::TK_RPAREN)
                        {// Skip )
                            goto error;
                        }
                        if(lexer_.Next().type() != Token::TK_LBRACE)
                        {// Skip {
                            goto error;
                        }
                        AST* body = ParseFunctionBody();
                        if(body->IsIllegal())
                        {
                            delete obj;
                            return body;
                        }
                        if(lexer_.Next().type() != Token::TK_RBRACE)
                        {// Skip }
                            delete body;
                            goto error;
                        }
                        Function* value = new Function(params, body, SOURCE_PARSED);
                        obj->AddProperty(ObjectLiteral::Property(key, value, type));
                    }
                    else
                    {
                        if(lexer_.Next().type() != Token::TK_COLON)
                            goto error;
                        AST* value = ParseAssignmentExpression(false);
                        if(value->type() == AST::AST_ILLEGAL)
                            goto error;
                        obj->AddProperty(ObjectLiteral::Property(token, value, ObjectLiteral::Property::NORMAL));
                    }
                }
                else
                {
                    lexer_.Next();
                    goto error;
                }
                token = lexer_.NextAndRewind();
                if(token.type() == Token::TK_COMMA)
                {
                    lexer_.Next();// Skip ,
                    token = lexer_.NextAndRewind();
                }
            }
            assert(token.type() == Token::TK_RBRACE);
            assert(lexer_.Next().type() == Token::TK_RBRACE);
            obj->SetSource(SOURCE_PARSED);
            return obj;
        error:
            delete obj;
            return new AST(AST::AST_ILLEGAL, SOURCE_PARSED);
        }

        AST* ParseExpression(bool no_in)
        {
            START_POS;

            AST* element = ParseAssignmentExpression(no_in);
            if(element->IsIllegal())
            {
                return element;
            }
            // NOTE(zhuzilin) If expr has only one element, then just return the element.
            Token token = lexer_.NextAndRewind();
            if(token.type() != Token::TK_COMMA)
            {
                return element;
            }

            Expression* expr = new Expression();
            expr->AddElement(element);
            while(token.type() == Token::TK_COMMA)
            {
                lexer_.Next();// skip ,
                element = ParseAssignmentExpression(no_in);
                if(element->IsIllegal())
                {
                    delete expr;
                    return element;
                }
                expr->AddElement(element);
                token = lexer_.NextAndRewind();
            }
            expr->SetSource(SOURCE_PARSED);
            return expr;
        }

        AST* ParseAssignmentExpression(bool no_in)
        {
            START_POS;

            AST* lhs = ParseConditionalExpression(no_in);
            if(lhs->IsIllegal())
                return lhs;

            // Not LeftHandSideExpression
            if(lhs->type() != AST::AST_EXPR_LHS)
            {
                return lhs;
            }
            Token op = lexer_.NextAndRewind();
            if(!op.IsAssignmentOperator())
                return lhs;

            lexer_.Next();
            AST* rhs = ParseAssignmentExpression(no_in);
            if(rhs->IsIllegal())
            {
                delete lhs;
                return rhs;
            }

            return new Binary(lhs, rhs, op, SOURCE_PARSED);
        }

        AST* ParseConditionalExpression(bool no_in)
        {
            START_POS;
            AST* cond = ParseBinaryAndUnaryExpression(no_in, 0);
            if(cond->IsIllegal())
                return cond;
            Token token = lexer_.NextAndRewind();
            if(token.type() != Token::TK_QUESTION)
                return cond;
            lexer_.Next();
            AST* lhs = ParseAssignmentExpression(no_in);
            if(lhs->IsIllegal())
            {
                delete cond;
                return lhs;
            }
            token = lexer_.NextAndRewind();
            if(token.type() != Token::TK_COLON)
            {
                delete cond;
                delete lhs;
                return new AST(AST::AST_ILLEGAL, SOURCE_PARSED);
            }
            lexer_.Next();
            AST* rhs = ParseAssignmentExpression(no_in);
            if(lhs->IsIllegal())
            {
                delete cond;
                delete lhs;
                return rhs;
            }
            AST* triple = new TripleCondition(cond, lhs, rhs);
            triple->SetSource(SOURCE_PARSED);
            return triple;
        }

        AST* ParseBinaryAndUnaryExpression(bool no_in, int priority)
        {
            START_POS;
            AST* lhs = nullptr;
            AST* rhs = nullptr;
            // Prefix Operators.
            Token prefix_op = lexer_.NextAndRewind();
            if(prefix_op.UnaryPrefixPriority() > priority)
            {
                lexer_.Next();
                lhs = ParseBinaryAndUnaryExpression(no_in, prefix_op.UnaryPrefixPriority());
                if(lhs->IsIllegal())
                    return lhs;
                lhs = new Unary(lhs, prefix_op, true);
            }
            else
            {
                lhs = ParseLeftHandSideExpression();
                if(lhs->IsIllegal())
                    return lhs;
                // Postfix Operators.
                //
                // Because the priority of postfix operators are higher than prefix ones,
                // they won't be parsed at the same time.
                Token postfix_op = lexer_.NextAndRewind();
                if(!lexer_.LineTermAhead() && postfix_op.UnaryPostfixPriority() > priority)
                {
                    if(lhs->type() != AST::AST_EXPR_BINARY && lhs->type() != AST::AST_EXPR_UNARY)
                    {
                        lexer_.Next();
                        lhs = new Unary(lhs, postfix_op, false);
                        lhs->SetSource(SOURCE_PARSED);
                    }
                    else
                    {
                        delete lhs;
                        return new AST(AST::AST_ILLEGAL, SOURCE_PARSED);
                    }
                }
            }
            while(true)
            {
                Token binary_op = lexer_.NextAndRewind();
                if(binary_op.BinaryPriority(no_in) > priority)
                {
                    lexer_.Next();
                    rhs = ParseBinaryAndUnaryExpression(no_in, binary_op.BinaryPriority(no_in));
                    if(rhs->IsIllegal())
                        return rhs;
                    lhs = new Binary(lhs, rhs, binary_op);
                    lhs->SetSource(SOURCE_PARSED);
                }
                else
                {
                    break;
                }
            }
            lhs->SetSource(SOURCE_PARSED);
            return lhs;
        }

        AST* ParseLeftHandSideExpression()
        {
            START_POS;
            Token token = lexer_.NextAndRewind();
            AST* base;
            size_t new_count = 0;
            while(token.source() == u"new")
            {
                lexer_.Next();
                new_count++;
                token = lexer_.NextAndRewind();
            }
            if(token.source() == u"function")
            {
                base = ParseFunction(false);
            }
            else
            {
                base = ParsePrimaryExpression();
            }
            if(base->IsIllegal())
            {
                return base;
            }
            LHS* lhs = new LHS(base, new_count);

            while(true)
            {
                token = lexer_.NextAndRewind();
                switch(token.type())
                {
                    case Token::TK_LPAREN:
                    {// (
                        AST* ast = ParseArguments();
                        if(ast->IsIllegal())
                        {
                            delete lhs;
                            return ast;
                        }
                        assert(ast->type() == AST::AST_EXPR_ARGS);
                        Arguments* args = static_cast<Arguments*>(ast);
                        lhs->AddArguments(args);
                        break;
                    }
                    case Token::TK_LBRACK:
                    {// [
                        lexer_.Next();// skip [
                        AST* index = ParseExpression(false);
                        if(index->IsIllegal())
                        {
                            delete lhs;
                            return index;
                        }
                        token = lexer_.Next();// skip ]
                        if(token.type() != Token::TK_RBRACK)
                        {
                            delete lhs;
                            delete index;
                            goto error;
                        }
                        lhs->AddIndex(index);
                        break;
                    }
                    case Token::TK_DOT:
                    {// .
                        lexer_.Next();// skip .
                        token = lexer_.Next();// skip IdentifierName
                        if(!token.IsIdentifierName())
                        {
                            delete lhs;
                            goto error;
                        }
                        lhs->AddProp(token);
                        break;
                    }
                    default:
                        lhs->SetSource(SOURCE_PARSED);
                        return lhs;
                }
            }
        error:
            return new AST(AST::AST_ILLEGAL, SOURCE_PARSED);
        }

        AST* ParseArguments()
        {
            START_POS;
            assert(lexer_.Next().type() == Token::TK_LPAREN);
            std::vector<AST*> args;
            AST* arg;
            Arguments* arg_ast;
            Token token = lexer_.NextAndRewind();
            if(token.type() != Token::TK_RPAREN)
            {
                arg = ParseAssignmentExpression(false);
                if(arg->IsIllegal())
                    return arg;
                args.emplace_back(arg);
                token = lexer_.NextAndRewind();
            }
            while(token.type() != Token::TK_RPAREN)
            {
                if(token.type() != Token::TK_COMMA)
                {
                    goto error;
                }
                lexer_.Next();// skip ,
                arg = ParseAssignmentExpression(false);
                if(arg->IsIllegal())
                {
                    for(auto arg : args)
                        delete arg;
                    return arg;
                }
                args.emplace_back(arg);
                token = lexer_.NextAndRewind();
            }
            assert(lexer_.Next().type() == Token::TK_RPAREN);// skip )
            arg_ast = new Arguments(args);
            arg_ast->SetSource(SOURCE_PARSED);
            return arg_ast;
        error:
            for(auto arg : args)
                delete arg;
            return new AST(AST::AST_ILLEGAL, SOURCE_PARSED);
        }

        AST* ParseFunctionBody(Token::Type ending_token_type = Token::TK_RBRACE)
        {
            return ParseProgramOrFunctionBody(ending_token_type, AST::AST_FUNC_BODY);
        }

        AST* ParseProgram()
        {
            return ParseProgramOrFunctionBody(Token::TK_EOS, AST::AST_PROGRAM);
        }

        AST* ParseProgramOrFunctionBody(Token::Type ending_token_type, AST::Type program_or_function)
        {
            START_POS;
            // 14.1
            bool strict = false;
            size_t old_pos = lexer_.Pos();
            Token old_token = lexer_.Last();
            Token token = lexer_.NextAndRewind();
            if(token.source() == u"\"use strict\"" || token.source() == u"'use strict'")
            {
                lexer_.Next();
                if(lexer_.Next().IsSemiColon())
                {
                    strict = true;
                }
                else
                {
                    lexer_.Rewind(old_pos, old_token);
                }
            }

            ProgramOrFunctionBody* prog = new ProgramOrFunctionBody(program_or_function, strict);
            AST* element;

            token = lexer_.NextAndRewind();
            while(token.type() != ending_token_type)
            {
                if(token.source() == u"function")
                {
                    element = ParseFunction(true);
                    if(element->IsIllegal())
                    {
                        delete prog;
                        return element;
                    }
                    prog->AddFunctionDecl(element);
                }
                else
                {
                    element = ParseStatement();
                    if(element->IsIllegal())
                    {
                        delete prog;
                        return element;
                    }
                    prog->AddStatement(element);
                }
                token = lexer_.NextAndRewind();
            }
            assert(token.type() == ending_token_type);
            prog->SetSource(SOURCE_PARSED);
            return prog;
        }

        AST* ParseStatement()
        {
            START_POS;
            Token token = lexer_.NextAndRewind();

            switch(token.type())
            {
                case Token::TK_LBRACE:// {
                    return ParseBlockStatement();
                case Token::TK_SEMICOLON:// ;
                    lexer_.Next();
                    return new AST(AST::AST_STMT_EMPTY, u";");
                case Token::TK_KEYWORD:
                {
                    if(token.source() == u"var")
                        return ParseVariableStatement(false);
                    else if(token.source() == u"if")
                        return ParseIfStatement();
                    else if(token.source() == u"do")
                        return ParseDoWhileStatement();
                    else if(token.source() == u"while")
                        return ParseWhileStatement();
                    else if(token.source() == u"for")
                        return ParseForStatement();
                    else if(token.source() == u"continue")
                        return ParseContinueStatement();
                    else if(token.source() == u"break")
                        return ParseBreakStatement();
                    else if(token.source() == u"return")
                        return ParseReturnStatement();
                    else if(token.source() == u"with")
                        return ParseWithStatement();
                    else if(token.source() == u"switch")
                        return ParseSwitchStatement();
                    else if(token.source() == u"throw")
                        return ParseThrowStatement();
                    else if(token.source() == u"try")
                        return ParseTryStatement();
                    else if(token.source() == u"debugger")
                    {
                        lexer_.Next();
                        if(!lexer_.TrySkipSemiColon())
                        {
                            lexer_.Next();
                            goto error;
                        }
                        return new AST(AST::AST_STMT_DEBUG, SOURCE_PARSED);
                    }
                    break;
                }
                case Token::TK_IDENT:
                {
                    size_t old_pos = lexer_.Pos();
                    Token old_token = lexer_.Last();
                    lexer_.Next();
                    Token colon = lexer_.Next();
                    lexer_.Rewind(old_pos, old_token);
                    if(colon.type() == Token::TK_COLON)
                        return ParseLabelledStatement();
                }
                default:
                    break;
            }
            return ParseExpressionStatement();
        error:
            return new AST(AST::AST_ILLEGAL, SOURCE_PARSED);
        }

        AST* ParseBlockStatement()
        {
            START_POS;
            assert(lexer_.Next().type() == Token::TK_LBRACE);
            Block* block = new Block();
            Token token = lexer_.NextAndRewind();
            while(token.type() != Token::TK_RBRACE)
            {
                AST* stmt = ParseStatement();
                if(stmt->IsIllegal())
                {
                    delete block;
                    return stmt;
                }
                block->AddStatement(stmt);
                token = lexer_.NextAndRewind();
            }
            assert(token.type() == Token::TK_RBRACE);
            lexer_.Next();
            block->SetSource(SOURCE_PARSED);
            return block;
        }

        AST* ParseVariableDeclaration(bool no_in)
        {
            START_POS;
            Token ident = lexer_.Next();
            AST* init;
            assert(ident.IsIdentifier());
            if(lexer_.NextAndRewind().type() != Token::TK_ASSIGN)
            {
                return new VarDecl(ident, SOURCE_PARSED);
            }
            lexer_.Next();// skip =
            init = ParseAssignmentExpression(no_in);
            if(init->IsIllegal())
                return init;
            return new VarDecl(ident, init, SOURCE_PARSED);
        error:
            return new AST(AST::AST_ILLEGAL, SOURCE_PARSED);
        }

        AST* ParseVariableStatement(bool no_in)
        {
            START_POS;
            assert(lexer_.Next().source() == u"var");
            VarStmt* var_stmt = new VarStmt();
            AST* decl;
            Token token = lexer_.NextAndRewind();
            if(!token.IsIdentifier())
            {
                lexer_.Next();
                goto error;
            }
            // Similar to ParseExpression
            decl = ParseVariableDeclaration(no_in);
            if(decl->IsIllegal())
            {
                delete var_stmt;
                return decl;
            }
            var_stmt->AddDecl(decl);
            token = lexer_.NextAndRewind();
            while(token.type() == Token::TK_COMMA)
            {
                lexer_.Next();// skip ,
                decl = ParseVariableDeclaration(no_in);
                if(decl->IsIllegal())
                {
                    delete var_stmt;
                    return decl;
                }
                var_stmt->AddDecl(decl);
                token = lexer_.NextAndRewind();
            }
            if(!lexer_.TrySkipSemiColon())
            {
                lexer_.Next();
                goto error;
            }

            var_stmt->SetSource(SOURCE_PARSED);
            return var_stmt;
        error:
            delete var_stmt;
            return new AST(AST::AST_ILLEGAL, SOURCE_PARSED);
        }

        AST* ParseExpressionStatement()
        {
            START_POS;
            Token token = lexer_.NextAndRewind();
            assert(token.type() != Token::TK_LBRACE && token.source() != u"function");
            AST* exp = ParseExpression(false);
            if(exp->IsIllegal())
                return exp;
            if(!lexer_.TrySkipSemiColon())
            {
                lexer_.Next();
                delete exp;
                return new AST(AST::AST_ILLEGAL, SOURCE_PARSED);
            }
            return exp;
        }

        AST* ParseIfStatement()
        {
            START_POS;
            AST* cond;
            AST* if_block;

            assert(lexer_.Next().source() == u"if");
            lexer_.Next();// skip (
            cond = ParseExpression(false);
            if(cond->IsIllegal())
                return cond;
            if(lexer_.Next().type() != Token::TK_RPAREN)
            {// skip )
                delete cond;
                goto error;
            }
            if_block = ParseStatement();
            if(if_block->IsIllegal())
            {
                delete cond;
                return if_block;
            }
            if(lexer_.NextAndRewind().source() == u"else")
            {
                lexer_.Next();// skip else
                AST* else_block = ParseStatement();
                if(else_block->IsIllegal())
                {
                    delete cond;
                    delete if_block;
                    return else_block;
                }
                return new If(cond, if_block, else_block, SOURCE_PARSED);
            }
            return new If(cond, if_block, SOURCE_PARSED);

        error:
            return new AST(AST::AST_ILLEGAL, SOURCE_PARSED);
        }

        AST* ParseDoWhileStatement()
        {
            START_POS;
            assert(lexer_.Next().source() == u"do");
            AST* cond;
            AST* loop_block;
            loop_block = ParseStatement();
            if(loop_block->IsIllegal())
            {
                return loop_block;
            }
            if(lexer_.Next().source() != u"while")
            {// skip while
                delete loop_block;
                goto error;
            }
            if(lexer_.Next().type() != Token::TK_LPAREN)
            {// skip (
                delete loop_block;
                goto error;
            }
            cond = ParseExpression(false);
            if(cond->IsIllegal())
            {
                delete loop_block;
                return cond;
            }
            if(lexer_.Next().type() != Token::TK_RPAREN)
            {// skip )
                delete cond;
                goto error;
            }
            if(!lexer_.TrySkipSemiColon())
            {
                lexer_.Next();
                delete cond;
                delete loop_block;
                goto error;
            }
            return new DoWhile(cond, loop_block, SOURCE_PARSED);
        error:
            return new AST(AST::AST_ILLEGAL, SOURCE_PARSED);
        }

        AST* ParseWhileStatement()
        {
            return ParseWhileOrWithStatement(u"while", AST::AST_STMT_WHILE);
        }

        AST* ParseWithStatement()
        {
            return ParseWhileOrWithStatement(u"with", AST::AST_STMT_WITH);
        }

        AST* ParseWhileOrWithStatement(const std::u16string& keyword, AST::Type type)
        {
            START_POS;
            assert(lexer_.Next().source() == keyword);
            AST* expr;
            AST* stmt;
            if(lexer_.Next().type() != Token::TK_LPAREN)
            {// skip (
                goto error;
            }
            expr = ParseExpression(false);
            if(expr->IsIllegal())
                return expr;
            if(lexer_.Next().type() != Token::TK_RPAREN)
            {// skip )
                delete expr;
                goto error;
            }
            stmt = ParseStatement();
            if(stmt->IsIllegal())
            {
                delete expr;
                return stmt;
            }
            return new WhileOrWith(type, expr, stmt, SOURCE_PARSED);
        error:
            return new AST(AST::AST_ILLEGAL, SOURCE_PARSED);
        }

        AST* ParseForStatement()
        {
            START_POS;
            assert(lexer_.Next().source() == u"for");
            Token token = lexer_.Next();// skip (
            AST* expr0;
            if(token.type() != Token::TK_LPAREN)
                goto error;
            token = lexer_.NextAndRewind();
            if(token.IsSemiColon())
            {
                return ParseForStatement({}, start);// for (;
            }
            else if(token.source() == u"var")
            {
                lexer_.Next();// skip var
                std::vector<AST*> expr0s;

                // NOTE(zhuzilin) the starting token for ParseVariableDeclaration
                // must be identifier. This is for better error code.
                if(!lexer_.NextAndRewind().IsIdentifier())
                {
                    goto error;
                }
                expr0 = ParseVariableDeclaration(true);
                if(expr0->IsIllegal())
                    return expr0;

                token = lexer_.NextAndRewind();
                if(token.source() == u"in")// var VariableDeclarationNoIn in
                    return ParseForInStatement(expr0, start);

                expr0s.emplace_back(expr0);
                while(!token.IsSemiColon())
                {
                    // NOTE(zhuzilin) the starting token for ParseVariableDeclaration
                    // must be identifier. This is for better error code.
                    if(lexer_.Next().type() != Token::TK_COMMA ||// skip ,
                       !lexer_.NextAndRewind().IsIdentifier())
                    {
                        for(auto expr : expr0s)
                            delete expr;
                        goto error;
                    }

                    expr0 = ParseVariableDeclaration(true);
                    if(expr0->IsIllegal())
                    {
                        for(auto expr : expr0s)
                            delete expr;
                        return expr0;
                    }
                    expr0s.emplace_back(expr0);
                    token = lexer_.NextAndRewind();
                }
                return ParseForStatement(expr0s, start);// var VariableDeclarationListNoIn;
            }
            else
            {
                expr0 = ParseExpression(true);
                if(expr0->IsIllegal())
                {
                    return expr0;
                }
                token = lexer_.NextAndRewind();
                if(token.IsSemiColon())
                {
                    return ParseForStatement({ expr0 }, start);// for ( ExpressionNoIn;
                }
                else if(token.source() == u"in" && expr0->type() == AST::AST_EXPR_LHS)
                {// for ( LeftHandSideExpression in
                    return ParseForInStatement(expr0, start);
                }
                else
                {
                    delete expr0;
                    goto error;
                }
            }
        error:
            return new AST(AST::AST_ILLEGAL, SOURCE_PARSED);
        }

        AST* ParseForStatement(const std::vector<AST*>& expr0s, size_t start)
        {
            assert(lexer_.Next().IsSemiColon());
            AST* expr1 = nullptr;
            AST* expr2 = nullptr;
            AST* stmt;
            Token token = lexer_.NextAndRewind();
            if(!token.IsSemiColon())
            {
                expr1 = ParseExpression(false);// for (xxx; Expression
                if(expr1->IsIllegal())
                {
                    for(auto expr : expr0s)
                    {
                        delete expr;
                    }
                    return expr1;
                }
            }

            if(!lexer_.Next().IsSemiColon())
            {// skip ;
                lexer_.Next();
                goto error;
            }

            token = lexer_.NextAndRewind();
            if(token.type() != Token::TK_RPAREN)
            {
                expr2 = ParseExpression(false);// for (xxx; xxx; Expression
                if(expr2->IsIllegal())
                {
                    for(auto expr : expr0s)
                    {
                        delete expr;
                    }
                    if(expr1 != nullptr)
                        delete expr1;
                    return expr2;
                }
            }

            if(lexer_.Next().type() != Token::TK_RPAREN)
            {// skip )
                lexer_.Next();
                goto error;
            }

            stmt = ParseStatement();
            if(stmt->IsIllegal())
            {
                for(auto expr : expr0s)
                {
                    delete expr;
                }
                if(expr1 != nullptr)
                    delete expr1;
                if(expr2 != nullptr)
                    delete expr2;
                return stmt;
            }

            return new For(expr0s, expr1, expr2, stmt, SOURCE_PARSED);
        error:
            for(auto expr : expr0s)
            {
                delete expr;
            }
            if(expr1 != nullptr)
                delete expr1;
            if(expr2 != nullptr)
                delete expr2;
            return new AST(AST::AST_ILLEGAL, SOURCE_PARSED);
        }

        AST* ParseForInStatement(AST* expr0, size_t start)
        {
            assert(lexer_.Next().source() == u"in");
            AST* expr1 = ParseExpression(false);// for ( xxx in Expression
            AST* stmt;
            if(expr1->IsIllegal())
            {
                delete expr0;
                return expr1;
            }

            if(lexer_.Next().type() != Token::TK_RPAREN)
            {// skip )
                lexer_.Next();
                goto error;
            }

            stmt = ParseStatement();
            if(stmt->IsIllegal())
            {
                delete expr0;
                delete expr1;
                return stmt;
            }
            return new ForIn(expr0, expr1, stmt, SOURCE_PARSED);
        error:
            delete expr0;
            delete expr1;
            return new AST(AST::AST_ILLEGAL, SOURCE_PARSED);
        }

        AST* ParseContinueStatement()
        {
            return ParseContinueOrBreakStatement(u"continue", AST::AST_STMT_CONTINUE);
        }

        AST* ParseBreakStatement()
        {
            return ParseContinueOrBreakStatement(u"break", AST::AST_STMT_BREAK);
        }

        AST* ParseContinueOrBreakStatement(const std::u16string& keyword, AST::Type type)
        {
            START_POS;
            assert(lexer_.Next().source() == keyword);
            if(!lexer_.TrySkipSemiColon())
            {
                Token ident = lexer_.NextAndRewind();
                if(ident.IsIdentifier())
                {
                    lexer_.Next();// Skip Identifier
                }
                if(!lexer_.TrySkipSemiColon())
                {
                    lexer_.Next();
                    return new AST(AST::AST_ILLEGAL, SOURCE_PARSED);
                }
                return new ContinueOrBreak(type, ident, SOURCE_PARSED);
            }
            return new ContinueOrBreak(type, SOURCE_PARSED);
        }

        AST* ParseReturnStatement()
        {
            START_POS;
            assert(lexer_.Next().source() == u"return");
            AST* expr = nullptr;
            if(!lexer_.TrySkipSemiColon())
            {
                expr = ParseExpression(false);
                if(expr->IsIllegal())
                {
                    return expr;
                }
                if(!lexer_.TrySkipSemiColon())
                {
                    lexer_.Next();
                    delete expr;
                    return new AST(AST::AST_ILLEGAL, SOURCE_PARSED);
                }
            }
            return new Return(expr, SOURCE_PARSED);
        }

        AST* ParseThrowStatement()
        {
            START_POS;
            assert(lexer_.Next().source() == u"throw");
            AST* expr = nullptr;
            if(!lexer_.TrySkipSemiColon())
            {
                expr = ParseExpression(false);
                if(expr->IsIllegal())
                {
                    return expr;
                }
                if(!lexer_.TrySkipSemiColon())
                {
                    lexer_.Next();
                    delete expr;
                    return new AST(AST::AST_ILLEGAL, SOURCE_PARSED);
                }
            }
            return new Throw(expr, SOURCE_PARSED);
        }

        AST* ParseSwitchStatement()
        {
            START_POS;
            Switch* switch_stmt = new Switch();
            AST* expr;
            Token token = lexer_.Last();
            assert(lexer_.Next().source() == u"switch");
            if(lexer_.Next().type() != Token::TK_LPAREN)
            {// skip (
                goto error;
            }
            expr = ParseExpression(false);
            if(expr->IsIllegal())
            {
                delete switch_stmt;
                return expr;
            }
            if(lexer_.Next().type() != Token::TK_RPAREN)
            {// skip )
                delete expr;
                goto error;
            }
            switch_stmt->SetExpr(expr);
            if(lexer_.Next().type() != Token::TK_LBRACE)
            {// skip {
                goto error;
            }
            // Loop for parsing CaseClause
            token = lexer_.NextAndRewind();
            while(token.type() != Token::TK_RBRACE)
            {
                AST* case_expr = nullptr;
                std::vector<AST*> stmts;
                std::u16string type = token.source();
                if(type == u"case")
                {
                    lexer_.Next();// skip case
                    case_expr = ParseExpression(false);
                    if(case_expr->IsIllegal())
                    {
                        delete switch_stmt;
                        return case_expr;
                    }
                }
                else if(type == u"default")
                {
                    lexer_.Next();// skip default
                    // can only have one default.
                    if(switch_stmt->has_default_clause())
                        goto error;
                }
                else
                {
                    lexer_.Next();
                    goto error;
                }
                if(lexer_.Next().type() != Token::TK_COLON)
                {// skip :
                    delete case_expr;
                    goto error;
                }
                // parse StatementList
                token = lexer_.NextAndRewind();
                while(token.source() != u"case" && token.source() != u"default" && token.type() != Token::TK_RBRACE)
                {
                    AST* stmt = ParseStatement();
                    if(stmt->IsIllegal())
                    {
                        for(auto s : stmts)
                        {
                            delete s;
                        }
                        delete switch_stmt;
                        return stmt;
                    }
                    stmts.emplace_back(stmt);
                    token = lexer_.NextAndRewind();
                }
                if(type == u"case")
                {
                    if(switch_stmt->has_default_clause())
                    {
                        switch_stmt->AddAfterDefaultCaseClause(Switch::CaseClause(case_expr, stmts));
                    }
                    else
                    {
                        switch_stmt->AddBeforeDefaultCaseClause(Switch::CaseClause(case_expr, stmts));
                    }
                }
                else
                {
                    switch_stmt->SetDefaultClause(stmts);
                }
                token = lexer_.NextAndRewind();
            }
            assert(token.type() == Token::TK_RBRACE);
            assert(lexer_.Next().type() == Token::TK_RBRACE);
            switch_stmt->SetSource(SOURCE_PARSED);
            return switch_stmt;
        error:
            delete switch_stmt;
            return new AST(AST::AST_ILLEGAL, SOURCE_PARSED);
        }

        AST* ParseTryStatement()
        {
            START_POS;
            assert(lexer_.Next().source() == u"try");

            AST* try_block;
            Token catch_ident(Token::TK_NOT_FOUND, u"");
            AST* catch_block = nullptr;
            AST* finally_block = nullptr;

            try_block = ParseBlockStatement();
            if(try_block->IsIllegal())
                return try_block;
            if(lexer_.NextAndRewind().source() == u"catch")
            {
                lexer_.Next();// skip catch
                if(lexer_.Next().type() != Token::TK_LPAREN)
                {// skip (
                    delete try_block;
                    goto error;
                }
                catch_ident = lexer_.Next();// skip identifier
                if(!catch_ident.IsIdentifier())
                {
                    goto error;
                }
                if(lexer_.Next().type() != Token::TK_RPAREN)
                {// skip )
                    delete try_block;
                    goto error;
                }
                catch_block = ParseBlockStatement();
                if(catch_block->IsIllegal())
                {
                    delete try_block;
                    return catch_block;
                }
            }
            if(lexer_.NextAndRewind().source() == u"finally")
            {
                lexer_.Next();// skip finally
                finally_block = ParseBlockStatement();
                if(finally_block->IsIllegal())
                {
                    delete try_block;
                    if(catch_block != nullptr)
                        delete catch_block;
                    return finally_block;
                }
            }
            if(catch_block == nullptr && finally_block == nullptr)
            {
                goto error;
            }
            else if(finally_block == nullptr)
            {
                assert(catch_block != nullptr && catch_ident.type() == Token::TK_IDENT);
                return new Try(try_block, catch_ident, catch_block, SOURCE_PARSED);
            }
            else if(catch_block == nullptr)
            {
                assert(finally_block != nullptr);
                return new Try(try_block, finally_block, SOURCE_PARSED);
            }
            assert(catch_block != nullptr && catch_ident.type() == Token::TK_IDENT);
            assert(finally_block != nullptr);
            return new Try(try_block, catch_ident, catch_block, finally_block, SOURCE_PARSED);
        error:
            delete try_block;
            if(catch_block != nullptr)
                delete try_block;
            if(finally_block != nullptr)
                delete finally_block;
            return new AST(AST::AST_ILLEGAL, SOURCE_PARSED);
        }

        AST* ParseLabelledStatement()
        {
            START_POS;
            Token ident = lexer_.Next();// skip identifier
            assert(lexer_.Next().type() == Token::TK_COLON);// skip colon
            AST* stmt = ParseStatement();
            if(stmt->IsIllegal())
            {
                return stmt;
            }
            return new LabelledStmt(ident, stmt, SOURCE_PARSED);
        }

    private:
        std::u16string source_;
        Lexer lexer_;
    };

    class JSValue;

    class Error
    {
    public:
        enum Type
        {
            E_OK = 0,
            E_EVAL,
            E_RANGE,
            E_REFERENCE,
            E_SYNTAX,
            E_TYPE,
            E_URI,
            E_NATIVE,
        };

        // TODO(zhuzilin) Fix memory leakage here.
        static Error* Ok()
        {
            return new Error(E_OK);
        }

        static Error* EvalError()
        {
            return new Error(E_EVAL);
        }

        static Error* RangeError(const std::u16string& message = u"")
        {
            return new Error(E_RANGE, message);
        }

        static Error* ReferenceError(const std::u16string& message = u"")
        {
            return new Error(E_REFERENCE, message);
        }

        static Error* SyntaxError(const std::u16string& message = u"")
        {
            return new Error(E_SYNTAX, message);
        }

        static Error* TypeError(const std::u16string& message = u"")
        {
            return new Error(E_TYPE, message);
        }

        static Error* UriError()
        {
            return new Error(E_URI);
        }

        static Error* NativeError(const std::u16string& message)
        {
            return new Error(E_NATIVE, message);
        }

        Type type()
        {
            return type_;
        }

        bool IsOk()
        {
            return type_ == E_OK;
        }

        std::u16string message()
        {
            return message_;
        }

        std::string ToString()
        {
            return Ok() ? "ok" : "error";
        }

    private:
        Error(Type t, const std::u16string& message = u"") : type_(t), message_(message)
        {
        }

        Type type_;
        std::u16string message_;
    };

    class JSValue
    {
    public:
        enum Type
        {
            JS_UNDEFINED = 0,
            JS_NULL,
            JS_BOOL,
            JS_STRING,
            JS_NUMBER,
            JS_OBJECT,

            LANG_TO_SPEC,

            JS_REF,
            JS_LIST,
            JS_PROP_DESC,
            JS_PROP_IDEN,
            JS_LEX_ENV,
            JS_ENV_REC,
        };

        JSValue(Type type) : type_(type)
        {
        }

        inline Type type()
        {
            return type_;
        }
        inline bool IsLanguageType()
        {
            return type_ < LANG_TO_SPEC;
        }
        inline bool IsSpecificationType()
        {
            return type_ > LANG_TO_SPEC;
        }
        inline bool IsPrimitive()
        {
            return type_ < JS_OBJECT;
        }

        inline bool IsUndefined()
        {
            return type_ == JS_UNDEFINED;
        }
        inline bool IsNull()
        {
            return type_ == JS_NULL;
        }
        inline bool IsBool()
        {
            return type_ == JS_BOOL;
        }
        inline bool IsString()
        {
            return type_ == JS_STRING;
        }
        inline bool IsNumber()
        {
            return type_ == JS_NUMBER;
        }
        inline bool IsObject()
        {
            return type_ == JS_OBJECT;
        }

        inline bool IsReference()
        {
            return type_ == JS_REF;
        }
        inline bool IsPropertyDescriptor()
        {
            return type_ == JS_PROP_DESC;
        }
        inline bool IsPropertyIdentifier()
        {
            return type_ == JS_PROP_IDEN;
        }
        inline bool IsLexicalEnvironment()
        {
            return type_ == JS_LEX_ENV;
        }
        inline bool IsEnvironmentRecord()
        {
            return type_ == JS_ENV_REC;
        }

        virtual std::string ToString() = 0;

        void CheckObjectCoercible(Error* e)
        {
            if(IsUndefined() || IsNull())
            {
                *e = *Error::TypeError(u"undefined or null is not coercible");
            }
        }
        virtual bool IsCallable()
        {
            // JSObject need to implement its own IsCallable
            assert(!IsObject());
            return false;
        }

        virtual bool IsConstructor()
        {
            // JSObject need to implement its own IsConstructor
            assert(!IsObject());
            return false;
        }

    private:
        Type type_;
    };

    class Undefined : public JSValue
    {
    public:
        static Undefined* Instance()
        {
            static Undefined singleton;
            return &singleton;
        }

        inline std::string ToString() override
        {
            return "Undefined";
        }

    private:
        Undefined() : JSValue(JS_UNDEFINED)
        {
        }
    };

    class Null : public JSValue
    {
    public:
        static Null* Instance()
        {
            static Null singleton;
            return &singleton;
        }
        std::string ToString() override
        {
            return "Null";
        }

    private:
        Null() : JSValue(JS_NULL)
        {
        }
    };

    class Bool : public JSValue
    {
    public:
        static Bool* True()
        {
            static Bool singleton(true);
            return &singleton;
        }
        static Bool* False()
        {
            static Bool singleton(false);
            return &singleton;
        }

        static Bool* Wrap(bool val)
        {
            return val ? True() : False();
        }

        inline bool data()
        {
            return data_;
        }

        inline std::string ToString() override
        {
            return data_ ? "true" : "false";
        }

    private:
        Bool(bool data) : JSValue(JS_BOOL), data_(data)
        {
        }

        bool data_;
    };

    class String : public JSValue
    {
    public:
        String(const std::u16string& data) : JSValue(JS_STRING), data_(data)
        {
        }
        std::u16string data()
        {
            return data_;
        }

        static String* Empty()
        {
            static String singleton(u"");
            return &singleton;
        }

        static String* Undefined()
        {
            static String singleton(u"undefined");
            return &singleton;
        }

        static String* Null()
        {
            static String singleton(u"null");
            return &singleton;
        }

        static String* True()
        {
            static String singleton(u"true");
            return &singleton;
        }

        static String* False()
        {
            static String singleton(u"false");
            return &singleton;
        }

        static String* NaN()
        {
            static String singleton(u"NaN");
            return &singleton;
        }

        static String* Zero()
        {
            static String singleton(u"0");
            return &singleton;
        }

        static String* Infinity()
        {
            static String singleton(u"Infinity");
            return &singleton;
        }

        inline std::string ToString() override
        {
            return log::ToString(data_);
        }

    private:
        std::u16string data_;
    };

    class Number : public JSValue
    {
    public:
        Number(double data) : JSValue(JS_NUMBER), data_(data)
        {
        }

        static Number* NaN()
        {
            static Number singleton(nan(""));
            return &singleton;
        }

        static Number* PositiveInfinity()
        {
            static Number singleton(std::numeric_limits<double>::infinity());
            return &singleton;
        }

        static Number* NegativeInfinity()
        {
            static Number singleton(-std::numeric_limits<double>::infinity());
            return &singleton;
        }

        static Number* Zero()
        {
            static Number singleton(0.0);
            return &singleton;
        }

        static Number* NegativeZero()
        {
            static Number singleton(-0.0);
            return &singleton;
        }

        static Number* One()
        {
            static Number singleton(1.0);
            return &singleton;
        }

        inline bool IsInfinity()
        {
            return isinf(data_);
        }
        inline bool IsPositiveInfinity()
        {
            return data_ == std::numeric_limits<double>::infinity();
        }
        inline bool IsNegativeInfinity()
        {
            return data_ == -std::numeric_limits<double>::infinity();
        }
        inline bool IsNaN()
        {
            return isnan(data_);
        }

        inline double data()
        {
            return data_;
        }

        inline std::string ToString() override
        {
            return std::to_string(data_);
        }

    private:
        double data_;
    };

    class PropertyDescriptor : public JSValue
    {
    public:
        PropertyDescriptor()
        : JSValue(JS_PROP_DESC), bitmask_(0), value_(Undefined::Instance()), getter_(Undefined::Instance()),
          setter_(Undefined::Instance()), writable_(false), enumerable_(false), configurable_(false)
        {
        }

        inline bool IsAccessorDescriptor()
        {
            return (bitmask_ & GET) && (bitmask_ & SET);
        }

        inline bool IsDataDescriptor()
        {
            return (bitmask_ & VALUE) && (bitmask_ & WRITABLE);
        }

        inline bool IsGenericDescriptor()
        {
            return !IsAccessorDescriptor() && !IsDataDescriptor();
        }

        // TODO(zhuzilin) May be check the member variable is initialized?
        inline bool HasValue()
        {
            return bitmask_ & VALUE;
        }
        inline JSValue* Value()
        {
            return value_;
        }
        inline void SetValue(JSValue* value)
        {
            bitmask_ |= VALUE;
            value_ = value;
        }

        inline bool HasWritable()
        {
            return bitmask_ & WRITABLE;
        }
        inline bool Writable()
        {
            return writable_;
        }
        inline void SetWritable(bool writable)
        {
            bitmask_ |= WRITABLE;
            writable_ = writable;
        }

        inline bool HasGet()
        {
            return bitmask_ & GET;
        }
        inline JSValue* Get()
        {
            return getter_;
        }
        inline void SetGet(JSValue* getter)
        {
            bitmask_ |= GET;
            getter_ = getter;
        }

        inline bool HasSet()
        {
            return bitmask_ & SET;
        }
        inline JSValue* Set()
        {
            return setter_;
        }
        inline void SetSet(JSValue* setter)
        {
            bitmask_ |= SET;
            setter_ = setter;
        }

        inline bool HasEnumerable()
        {
            return bitmask_ & ENUMERABLE;
        }
        inline bool Enumerable()
        {
            return enumerable_;
        }
        inline void SetEnumerable(bool enumerable)
        {
            bitmask_ |= ENUMERABLE;
            enumerable_ = enumerable;
        }

        inline bool HasConfigurable()
        {
            return bitmask_ & CONFIGURABLE;
        }
        inline bool Configurable()
        {
            return configurable_;
        }
        inline void SetConfigurable(bool configurable)
        {
            bitmask_ |= CONFIGURABLE;
            configurable_ = configurable;
        }

        inline void SetDataDescriptor(JSValue* value, bool writable, bool enumerable, bool configurable)
        {
            SetValue(value);
            SetWritable(writable);
            SetEnumerable(enumerable);
            SetConfigurable(configurable);
        }

        inline void SetAccessorDescriptor(JSValue* getter, JSValue* setter, bool enumerable, bool configurable)
        {
            SetGet(getter);
            SetSet(setter);
            SetEnumerable(enumerable);
            SetConfigurable(configurable);
        }

        // Set the value to `this` if `other` has.
        inline void Set(PropertyDescriptor* other)
        {
            if(other->HasValue())
                SetValue(other->Value());
            if(other->HasWritable())
                SetWritable(other->Writable());
            if(other->HasGet())
                SetGet(other->Get());
            if(other->HasSet())
                SetSet(other->Set());
            if(other->HasConfigurable())
                SetConfigurable(other->Configurable());
            if(other->HasEnumerable())
                SetEnumerable(other->Enumerable());
        }

        char bitmask()
        {
            return bitmask_;
        }
        void SetBitMask(char bitmask)
        {
            bitmask_ = bitmask;
        }

        std::string ToString() override
        {
            std::string res = "PropertyDescriptor{";
            if(HasValue())
                res += "v: " + value_->ToString() + ", ";
            if(HasWritable())
                res += "w: " + log::ToString(writable_) + ", ";
            if(HasEnumerable())
                res += "e: " + log::ToString(enumerable_) + ", ";
            if(HasConfigurable())
                res += "c: " + log::ToString(configurable_);

            res += '}';
            return res;
        }

    private:
        enum Field
        {
            VALUE = 1 << 0,
            WRITABLE = 1 << 1,
            GET = 1 << 2,
            SET = 1 << 3,
            ENUMERABLE = 1 << 4,
            CONFIGURABLE = 1 << 5,
        };

        char bitmask_;

        JSValue* value_;
        bool writable_;
        JSValue* getter_;
        JSValue* setter_;
        bool enumerable_;
        bool configurable_;
    };

    class PropertyIdentifier : public JSValue
    {
    public:
        PropertyIdentifier(const std::u16string& name, PropertyDescriptor* desciptor)
        : JSValue(JS_PROP_IDEN), name_(name), desciptor_(desciptor)
        {
        }

    private:
        std::u16string name_;
        PropertyDescriptor* desciptor_;
    };

    bool SameValue(JSValue* x, JSValue* y)
    {
        if(x->type() != y->type())
            return false;
        switch(x->type())
        {
            case JSValue::JS_UNDEFINED:
                return true;
            case JSValue::JS_NULL:
                return true;
            case JSValue::JS_NUMBER:
            {
                Number* num_x = static_cast<Number*>(x);
                Number* num_y = static_cast<Number*>(y);
                if(num_x->IsNaN() && num_y->IsNaN())
                    return true;
                double dx = num_x->data();
                double dy = num_y->data();
                if(dx == dy && dx == 0.0)
                {
                    return signbit(dx) == signbit(dy);
                }
                return dx == dy;
            }
            case JSValue::JS_STRING:
            {
                String* str_x = static_cast<String*>(x);
                String* str_y = static_cast<String*>(y);
                return str_x->data() == str_y->data();
            }
            case JSValue::JS_BOOL:
            {
                Bool* b_x = static_cast<Bool*>(x);
                Bool* b_y = static_cast<Bool*>(y);
                return b_x->data() == b_y->data();
            }
            default:
                return x == y;
        }
    }

    typedef std::function<JSValue*(Error*, JSValue*, const std::vector<JSValue*>&)> inner_func;

    class JSObject : public JSValue
    {
    public:
        enum ObjType
        {
            OBJ_GLOBAL,
            OBJ_OBJECT,
            OBJ_FUNC,
            OBJ_ARRAY,
            OBJ_STRING,
            OBJ_BOOL,
            OBJ_NUMBER,
            OBJ_MATH,
            OBJ_DATE,
            OBJ_REGEX,
            OBJ_JSON,
            OBJ_ERROR,

            OBJ_INNER_FUNC,
            OBJ_OTHER,
        };

        JSObject(ObjType obj_type, const std::u16string& klass, bool extensible, JSValue* primitive_value, bool is_constructor, bool is_callable, inner_func callable = nullptr)
        : JSValue(JS_OBJECT), obj_type_(obj_type), prototype_(Null::Instance()), class_(klass), extensible_(extensible),
          primitive_value_(primitive_value), is_constructor_(is_constructor), is_callable_(is_callable), callable_(callable)
        {
        }

        ObjType obj_type()
        {
            return obj_type_;
        }

        bool IsFunction()
        {
            return obj_type_ == OBJ_FUNC;
        }

        // Internal Preperties Common to All Objects
        JSValue* Prototype()
        {
            return prototype_;
        }
        void SetPrototype(JSValue* proto)
        {
            assert(proto->type() == JS_NULL || proto->type() == JS_OBJECT);
            prototype_ = proto;
        }
        std::u16string Class()
        {
            return class_;
        };
        bool Extensible()
        {
            return extensible_;
        };
        void SetExtensible(bool extensible)
        {
            extensible_ = extensible;
        }

        virtual JSValue* Get(Error* e, const std::u16string& P);
        virtual JSValue* GetOwnProperty(const std::u16string& P);
        JSValue* GetProperty(const std::u16string& P);
        void Put(Error* e, const std::u16string& P, JSValue* V, bool throw_flag);
        bool CanPut(const std::u16string& P);
        bool HasProperty(const std::u16string& P);
        virtual bool Delete(Error* e, const std::u16string& P, bool throw_flag);
        JSValue* DefaultValue(Error* e, const std::u16string& hint);
        virtual bool DefineOwnProperty(Error* e, const std::u16string& P, PropertyDescriptor* desc, bool throw_flag);

        // Internal Properties Only Defined for Some Objects
        // [[PrimitiveValue]]
        JSValue* PrimitiveValue()
        {
            assert(primitive_value_ != nullptr);
            return primitive_value_;
        };
        bool HasPrimitiveValue()
        {
            return obj_type_ == OBJ_BOOL || obj_type_ == OBJ_DATE || obj_type_ == OBJ_NUMBER || obj_type_ == OBJ_STRING;
        }
        // [[Construct]]
        virtual JSObject* Construct(Error* e, const std::vector<JSValue*>& arguments)
        {
            assert(false);
        }
        bool IsConstructor() override
        {
            return is_constructor_;
        }
        // [[Call]]
        virtual JSValue* Call(Error* e, JSValue* this_arg, const std::vector<JSValue*>& arguments = {})
        {
            assert(is_callable_ && callable_ != nullptr);
            return callable_(e, this, arguments);
        }
        bool IsCallable() override
        {
            return is_callable_;
        }
        // [[HasInstance]]
        // NOTE(zhuzilin) Here we use the implementation in 15.3.5.3 [[HasInstance]] (V)
        // to make sure all callables have HasInstance.
        virtual bool HasInstance(Error* e, JSValue* V)
        {
            assert(IsCallable());
            if(!V->IsObject())
                return false;
            JSValue* O = Get(e, u"prototype");
            if(!e->IsOk())
                return false;
            if(!O->IsObject())
            {
                *e = *Error::TypeError();
                return false;
            }
            while(!V->IsNull())
            {
                if(V == O)
                    return true;
                assert(V->IsObject());
                V = static_cast<JSObject*>(V)->Prototype();
                if(!e->IsOk())
                    return false;
            }
            return false;
        }

        void AddValueProperty(const std::u16string& name, JSValue* value, bool writable, bool enumerable, bool configurable)
        {
            PropertyDescriptor* desc = new PropertyDescriptor();
            desc->SetDataDescriptor(value, writable, enumerable, configurable);
            // This should just like named_properties_[name] = desc
            DefineOwnProperty(nullptr, name, desc, false);
        }

        void AddFuncProperty(const std::u16string& name, inner_func callable, bool writable, bool enumerable, bool configurable);

        // This for for-in statement.
        virtual std::vector<std::pair<std::u16string, PropertyDescriptor*>> AllEnumerableProperties()
        {
            std::vector<std::pair<std::u16string, PropertyDescriptor*>> result;
            for(auto pair : named_properties_)
            {
                if(!pair.second->HasEnumerable() || !pair.second->Enumerable())
                    continue;
                result.emplace_back(pair);
            }
            if(!prototype_->IsNull())
            {
                JSObject* proto = static_cast<JSObject*>(prototype_);
                for(auto pair : proto->AllEnumerableProperties())
                {
                    if(!pair.second->HasEnumerable() || !pair.second->Enumerable())
                        continue;
                    if(named_properties_.find(pair.first) == named_properties_.end())
                    {
                        result.emplace_back(pair);
                    }
                }
            }
            return result;
        }

        virtual std::string ToString() override
        {
            return log::ToString(class_);
        }

    private:
        ObjType obj_type_;
        std::map<std::u16string, PropertyDescriptor*> named_properties_;

        JSValue* prototype_;
        std::u16string class_;
        bool extensible_;

        JSValue* primitive_value_;

        bool is_constructor_;
        bool is_callable_;
        inner_func callable_;
    };

    // 8.12.1 [[GetOwnProperty]] (P)
    JSValue* JSObject::GetOwnProperty(const std::u16string& P)
    {
        // TODO(zhuzilin) String Object has a more elaborate impl 15.5.5.2.
        auto iter = named_properties_.find(P);
        if(iter == named_properties_.end())
        {
            return Undefined::Instance();
        }
        // NOTE(zhuzilin) In the spec, we need to create a new property descriptor D,
        // And assign the property to it. However, if we init D->value = a, and
        // set D->value = b in DefineOwnProperty, the value saved in the named_properties_ will
        // remain b and that is not what we want.
        return iter->second;
    }

    JSValue* JSObject::GetProperty(const std::u16string& P)
    {
        JSValue* own_property = GetOwnProperty(P);
        if(!own_property->IsUndefined())
        {
            return own_property;
        }
        JSValue* proto = Prototype();
        if(proto->IsNull())
        {
            return Undefined::Instance();
        }
        assert(proto->IsObject());
        JSObject* proto_obj = static_cast<JSObject*>(proto);
        return proto_obj->GetProperty(P);
    }

    JSValue* JSObject::Get(Error* e, const std::u16string& P)
    {
        JSValue* value = GetProperty(P);
        if(value->IsUndefined())
        {
            return Undefined::Instance();
        }
        PropertyDescriptor* desc = static_cast<PropertyDescriptor*>(value);
        if(desc->IsDataDescriptor())
        {
            return desc->Value();
        }
        else
        {
            assert(desc->IsAccessorDescriptor());
            JSValue* getter = desc->Get();
            if(getter->IsUndefined())
            {
                return Undefined::Instance();
            }
            JSObject* getter_obj = static_cast<JSObject*>(getter);
            return getter_obj->Call(e, this);
        }
    }

    bool JSObject::CanPut(const std::u16string& P)
    {
        JSValue* value = GetOwnProperty(P);
        if(!value->IsUndefined())
        {
            PropertyDescriptor* desc = static_cast<PropertyDescriptor*>(value);
            if(desc->IsAccessorDescriptor())
            {
                return !desc->Set()->IsUndefined();
            }
            else
            {
                return desc->Writable();
            }
        }

        JSValue* proto = Prototype();
        if(proto->IsNull())
        {
            return Extensible();
        }
        JSObject* proto_obj = static_cast<JSObject*>(proto);
        JSValue* inherit = proto_obj->GetProperty(P);
        if(inherit->IsUndefined())
        {
            return Extensible();
        }
        PropertyDescriptor* desc = static_cast<PropertyDescriptor*>(inherit);
        if(desc->IsAccessorDescriptor())
        {
            return !desc->Set()->IsUndefined();
        }
        else
        {
            return Extensible() ? desc->Writable() : false;
        }
    }

    // 8.12.5 [[Put]] ( P, V, Throw )
    void JSObject::Put(Error* e, const std::u16string& P, JSValue* V, bool throw_flag)
    {
        log::PrintSource("Put ", P, " " + V->ToString());
        if(!CanPut(P))
        {// 1
            if(throw_flag)
            {// 1.a
                *e = *Error::TypeError();
            }
            return;// 1.b
        }
        JSValue* value = GetOwnProperty(P);
        if(!value->IsUndefined())
        {
            PropertyDescriptor* own_desc = static_cast<PropertyDescriptor*>(value);// 2
            if(own_desc->IsDataDescriptor())
            {// 3
                PropertyDescriptor* value_desc = new PropertyDescriptor();
                value_desc->SetValue(V);
                log::PrintSource("Overwrite the old desc with " + value_desc->ToString());
                DefineOwnProperty(e, P, value_desc, throw_flag);
                return;
            }
        }
        value = GetProperty(P);
        if(!value->IsUndefined())
        {
            PropertyDescriptor* desc = static_cast<PropertyDescriptor*>(value);
            if(desc->IsAccessorDescriptor())
            {
                log::PrintSource("Use parent prototype's setter");
                JSValue* setter = desc->Set();
                assert(!setter->IsUndefined());
                JSObject* setter_obj = static_cast<JSObject*>(setter);
                setter_obj->Call(e, this, { V });
                return;
            }
        }
        PropertyDescriptor* new_desc = new PropertyDescriptor();
        new_desc->SetDataDescriptor(V, true, true, true);// 6.a
        DefineOwnProperty(e, P, new_desc, throw_flag);
    }

    bool JSObject::HasProperty(const std::u16string& P)
    {
        JSValue* desc = GetOwnProperty(P);
        return !desc->IsUndefined();
    }

    bool JSObject::Delete(Error* e, const std::u16string& P, bool throw_flag)
    {
        JSValue* value = GetOwnProperty(P);
        if(value->IsUndefined())
        {
            return true;
        }
        PropertyDescriptor* desc = static_cast<PropertyDescriptor*>(value);
        if(desc->Configurable())
        {
            named_properties_.erase(P);
            return true;
        }
        else
        {
            if(throw_flag)
            {
                *e = *Error::TypeError();
            }
            return false;
        }
    }

    // 8.12.9 [[DefineOwnProperty]] (P, Desc, Throw)
    bool JSObject::DefineOwnProperty(Error* e, const std::u16string& P, PropertyDescriptor* desc, bool throw_flag)
    {
        JSValue* current = GetOwnProperty(P);
        PropertyDescriptor* current_desc;
        if(current->IsUndefined())
        {
            if(!extensible_)// 3
                goto reject;
            // 4.
            named_properties_[P] = desc;
            return true;
        }
        if(desc->bitmask() == 0)
        {// 5
            return true;
        }
        current_desc = static_cast<PropertyDescriptor*>(current);
        if((desc->bitmask() & current_desc->bitmask()) == desc->bitmask())
        {
            bool same = true;
            if(desc->HasValue())
                same = same && SameValue(desc->Value(), current_desc->Value());
            if(desc->HasWritable())
                same = same && (desc->Writable() == current_desc->Writable());
            if(desc->HasGet())
                same = same && SameValue(desc->Get(), current_desc->Get());
            if(desc->HasSet())
                same = same && SameValue(desc->Set(), current_desc->Set());
            if(desc->HasConfigurable())
                same = same && (desc->Configurable() == current_desc->Configurable());
            if(desc->HasEnumerable())
                same = same && (desc->Enumerable() == current_desc->Enumerable());
            if(same)
                return true;// 6
        }
        log::PrintSource("desc: " + desc->ToString() + ", current: " + current_desc->ToString());
        if(!current_desc->Configurable())
        {// 7
            if(desc->Configurable())
            {// 7.a
                log::PrintSource("DefineOwnProperty: ", P, " not configurable, while new value configurable");
                goto reject;
            }
            if(desc->HasEnumerable() && (desc->Enumerable() != current_desc->Enumerable()))
            {// 7.b
                log::PrintSource("DefineOwnProperty: ", P, " enumerable value differ");
                goto reject;
            }
        }
        // 8.
        if(!desc->IsGenericDescriptor())
        {
            if(current_desc->IsDataDescriptor() != desc->IsDataDescriptor())
            {// 9.
                // 9.a
                if(!current_desc->Configurable())
                    goto reject;
                // 9.b.i & 9.c.i
                PropertyDescriptor* old_property = named_properties_[P];
                PropertyDescriptor* new_property = new PropertyDescriptor();
                new_property->SetConfigurable(old_property->Configurable());
                new_property->SetEnumerable(old_property->Enumerable());
                new_property->SetBitMask(old_property->bitmask());
                named_properties_[P] = new_property;
            }
            else if(current_desc->IsDataDescriptor() && desc->IsDataDescriptor())
            {// 10.
                if(!current_desc->Configurable())
                {// 10.a
                    if(!current_desc->Writable())
                    {
                        if(desc->Writable())
                            goto reject;// 10.a.i
                        // 10.a.ii.1
                        if(desc->HasValue() && !SameValue(desc->Value(), current_desc->Value()))
                            goto reject;
                    }
                }
                else
                {// 10.b
                    assert(current_desc->Configurable());
                }
            }
            else
            {// 11.
                assert(current_desc->IsAccessorDescriptor() && desc->IsAccessorDescriptor());
                if(!current_desc->Configurable())
                {// 11.a
                    if(!SameValue(desc->Set(), current_desc->Set()) ||// 11.a.i
                       !SameValue(desc->Get(), current_desc->Get()))// 11.a.ii
                        goto reject;
                }
            }
        }
        log::PrintSource("DefineOwnProperty: ", P, " is set" + (desc->HasValue() ? " to " + desc->Value()->ToString() : ""));
        // 12.
        current_desc->Set(desc);
        // 13.
        return true;
    reject:
        log::PrintSource("DefineOwnProperty reject");
        if(throw_flag)
        {
            *e = *Error::TypeError();
        }
        return false;
    }

    // EnvironmentRecord is also of type JSValue
    class EnvironmentRecord : public JSValue
    {
    public:
        EnvironmentRecord() : JSValue(JS_ENV_REC)
        {
        }

        virtual bool HasBinding(const std::u16string& N) = 0;
        virtual void CreateMutableBinding(Error* e, const std::u16string& N, bool D) = 0;
        virtual void SetMutableBinding(Error* e, const std::u16string& N, JSValue* V, bool S) = 0;
        virtual JSValue* GetBindingValue(Error* e, const std::u16string& N, bool S) = 0;
        virtual bool DeleteBinding(Error* e, const std::u16string& N) = 0;
        virtual JSValue* ImplicitThisValue() = 0;
    };

    class DeclarativeEnvironmentRecord : public EnvironmentRecord
    {
    public:
        struct Binding
        {
            JSValue* value;
            bool can_delete;
            bool is_mutable;
        };

        bool HasBinding(const std::u16string& N) override
        {
            return bindings_.find(N) != bindings_.end();
        }

        void CreateMutableBinding(Error* e, const std::u16string& N, bool D) override
        {
            assert(!HasBinding(N));
            Binding b;
            b.value = Undefined::Instance();
            b.can_delete = D;
            b.is_mutable = true;
            bindings_[N] = b;
        }

        void SetMutableBinding(Error* e, const std::u16string& N, JSValue* V, bool S) override
        {
            log::PrintSource("enter SetMutableBinding ", N, " to " + V->ToString());
            assert(V->IsLanguageType());
            assert(HasBinding(N));
            // NOTE(zhuzilin) If we do note b = bindings_[N] and change b.value,
            // the value stored in bindings_ won't change.
            if(bindings_[N].is_mutable)
            {
                bindings_[N].value = V;
            }
            else if(S)
            {
                *e = *Error::TypeError();
            }
        }

        JSValue* GetBindingValue(Error* e, const std::u16string& N, bool S) override
        {
            assert(HasBinding(N));
            Binding b = bindings_[N];
            if(b.value->IsUndefined())
            {
                if(S)
                {
                    *e = *Error::ReferenceError(N + u" is not defined");
                    return nullptr;
                }
                else
                {
                    log::PrintSource("GetBindingValue ", N, " undefined");
                    return Undefined::Instance();
                }
            }
            log::PrintSource("GetBindingValue ", N, " " + b.value->ToString());
            return b.value;
        }

        bool DeleteBinding(Error* e, const std::u16string& N) override
        {
            if(!HasBinding(N))
                return true;
            if(!bindings_[N].can_delete)
            {
                return false;
            }
            bindings_.erase(N);
            return true;
        }

        JSValue* ImplicitThisValue() override
        {
            return Undefined::Instance();
        }

        void CreateImmutableBinding(const std::u16string& N)
        {
            assert(!HasBinding(N));
            Binding b;
            b.value = Undefined::Instance();
            b.can_delete = false;
            b.is_mutable = false;
            bindings_[N] = b;
        }

        void InitializeImmutableBinding(const std::u16string& N, JSValue* V)
        {
            assert(HasBinding(N));
            assert(!bindings_[N].is_mutable && bindings_[N].value->IsUndefined());
            bindings_[N].value = V;
        }

        virtual std::string ToString() override
        {
            return "DeclarativeEnvRec(" + log::ToString(this) + ")";
        }

    private:
        std::unordered_map<std::u16string, Binding> bindings_;
    };

    class ObjectEnvironmentRecord : public EnvironmentRecord
    {
    public:
        ObjectEnvironmentRecord(JSObject* obj, bool provide_this = false) : bindings_(obj), provide_this_(provide_this)
        {
        }

        bool HasBinding(const std::u16string& N) override
        {
            return bindings_->HasProperty(N);
        }

        // 10.2.1.2.2 CreateMutableBinding (N, D)
        void CreateMutableBinding(Error* e, const std::u16string& N, bool D) override
        {
            assert(!HasBinding(N));
            PropertyDescriptor* desc = new PropertyDescriptor();
            desc->SetDataDescriptor(Undefined::Instance(), true, true, D);
            bindings_->DefineOwnProperty(e, N, desc, true);
        }

        void SetMutableBinding(Error* e, const std::u16string& N, JSValue* V, bool S) override
        {
            log::PrintSource("enter SetMutableBinding ", N, " to " + V->ToString());
            assert(V->IsLanguageType());
            bindings_->Put(e, N, V, S);
        }

        JSValue* GetBindingValue(Error* e, const std::u16string& N, bool S) override
        {
            bool value = HasBinding(N);
            if(!value)
            {
                if(S)
                {
                    *e = *Error::ReferenceError(N + u" is not defined");
                    return nullptr;
                }
                else
                {
                    return Undefined::Instance();
                }
            }
            return bindings_->Get(e, N);
        }

        bool DeleteBinding(Error* e, const std::u16string& N) override
        {
            return bindings_->Delete(e, N, false);
        }

        JSValue* ImplicitThisValue() override
        {
            if(provide_this_)
            {
                return bindings_;
            }
            return Undefined::Instance();
        }

        virtual std::string ToString() override
        {
            return "ObjectEnvRec(" + log::ToString(this) + ")";
        }

    private:
        JSObject* bindings_;
        bool provide_this_;
    };

    // 15.1 The Global Object
    class GlobalObject : public JSObject
    {
    public:
        static GlobalObject* Instance()
        {
            static GlobalObject singleton;
            return &singleton;
        }

        bool direct_eval()
        {
            return direct_eval_;
        }
        void SetDirectEval(bool direct_eval)
        {
            direct_eval_ = direct_eval;
        }

        // 15.1.2.1 eval(X)
        static JSValue* eval(Error* e, JSValue* this_arg, const std::vector<JSValue*>& vals);

        // 15.1.2.2 parseInt (string , radix)
        static JSValue* parseInt(Error* e, JSValue* this_arg, const std::vector<JSValue*>& vals)
        {
            assert(false);
        }

        // 15.1.2.3 parseFloat (string)
        static JSValue* parseFloat(Error* e, JSValue* this_arg, const std::vector<JSValue*>& vals)
        {
            assert(false);
        }

        // 15.1.2.4 isNaN (number)
        static JSValue* isNaN(Error* e, JSValue* this_arg, const std::vector<JSValue*>& vals)
        {
            assert(false);
        }

        // 15.1.2.5 isFinite (number)
        static JSValue* isFinite(Error* e, JSValue* this_arg, const std::vector<JSValue*>& vals)
        {
            assert(false);
        }

        inline std::string ToString() override
        {
            return "GlobalObject";
        }

    private:
        GlobalObject()
        : JSObject(OBJ_GLOBAL,
                   // 15.1 The values of the [[Prototype]] and [[Class]]
                   // of the global object are implementation-dependent.
                   u"Global",
                   // NOTE(zhuzilin) global object need to have [[Extensible]] as true,
                   // otherwise we cannot define variable in global code, as global varaibles
                   // are the property of global object.
                   true,
                   nullptr,
                   false,
                   false),
          direct_eval_(false)
        {
        }

        bool direct_eval_;
    };

    class DirectEvalGuard
    {
    public:
        DirectEvalGuard()
        {
            GlobalObject::Instance()->SetDirectEval(true);
        }

        ~DirectEvalGuard()
        {
            GlobalObject::Instance()->SetDirectEval(false);
        }
    };

    JSObject* ToObject(Error* e, JSValue* input);

    class Reference : public JSValue
    {
    public:
        Reference(JSValue* base, const std::u16string& reference_name, bool strict_reference)
        : JSValue(JS_REF), base_(base), reference_name_(reference_name), strict_reference_(strict_reference)
        {
        }

        JSValue* GetBase()
        {
            return base_;
        }
        std::u16string GetReferencedName()
        {
            return reference_name_;
        }
        bool IsStrictReference()
        {
            return strict_reference_;
        }
        bool HasPrimitiveBase()
        {
            return base_->IsBool() || base_->IsString() || base_->IsNumber();
        }
        bool IsPropertyReference()
        {
            return base_->IsObject() || HasPrimitiveBase();
        }
        bool IsUnresolvableReference()
        {
            return base_->IsUndefined();
        }

        std::string ToString() override
        {
            return "ref(" + log::ToString(reference_name_) + ")";
        }

    private:
        JSValue* base_;
        std::u16string reference_name_;
        bool strict_reference_;
    };

    JSValue* GetValue(Error* e, JSValue* V)
    {
        if(!V->IsReference())
        {
            return V;
        }
        Reference* ref = static_cast<Reference*>(V);
        if(ref->IsUnresolvableReference())
        {
            *e = *Error::ReferenceError(ref->GetReferencedName() + u" is not defined");
            return nullptr;
        }
        JSValue* base = ref->GetBase();
        if(ref->IsPropertyReference())
        {// 4
            // 4.a & 4.b
            if(!ref->HasPrimitiveBase())
            {
                assert(base->IsObject());
                JSObject* obj = static_cast<JSObject*>(base);
                return obj->Get(e, ref->GetReferencedName());
            }
            else
            {// special [[Get]]
                JSObject* O = ToObject(e, base);
                if(!e->IsOk())
                    return nullptr;
                JSValue* tmp = O->GetProperty(ref->GetReferencedName());
                if(tmp->IsUndefined())
                    return Undefined::Instance();
                PropertyDescriptor* desc = static_cast<PropertyDescriptor*>(tmp);
                if(desc->IsDataDescriptor())
                {
                    return desc->Value();
                }
                else
                {
                    assert(desc->IsAccessorDescriptor());
                    JSValue* getter = desc->Get();
                    if(getter->IsUndefined())
                    {
                        return Undefined::Instance();
                    }
                    JSObject* getter_obj = static_cast<JSObject*>(getter);
                    return getter_obj->Call(e, base, {});
                }
            }
        }
        else
        {
            assert(base->IsEnvironmentRecord());
            EnvironmentRecord* er = static_cast<EnvironmentRecord*>(base);
            return er->GetBindingValue(e, ref->GetReferencedName(), ref->IsStrictReference());
        }
    }

    void PutValue(Error* e, JSValue* V, JSValue* W)
    {
        log::PrintSource("PutValue V: " + V->ToString() + ", W: " + W->ToString());
        if(!V->IsReference())
        {
            *e = *Error::ReferenceError();
            return;
        }
        Reference* ref = static_cast<Reference*>(V);
        JSValue* base = ref->GetBase();
        if(ref->IsUnresolvableReference())
        {// 3
            if(ref->IsStrictReference())
            {// 3.a
                *e = *Error::ReferenceError();
                return;
            }
            GlobalObject::Instance()->Put(e, ref->GetReferencedName(), W, false);// 3.b
        }
        else if(ref->IsPropertyReference())
        {
            bool throw_flag = ref->IsStrictReference();
            std::u16string P = ref->GetReferencedName();
            if(!ref->HasPrimitiveBase())
            {
                assert(base->IsObject());
                JSObject* base_obj = static_cast<JSObject*>(base);
                base_obj->Put(e, P, W, throw_flag);
            }
            else
            {// special [[Put]]
                JSObject* O = ToObject(e, base);
                if(!O->CanPut(P))
                {// 2
                    if(throw_flag)
                        *e = *Error::TypeError();
                    return;
                }
                JSValue* tmp = O->GetOwnProperty(P);// 3
                if(!tmp->IsUndefined())
                {
                    PropertyDescriptor* own_desc = static_cast<PropertyDescriptor*>(tmp);
                    if(own_desc->IsDataDescriptor())
                    {// 4
                        if(throw_flag)
                            *e = *Error::TypeError();
                        return;
                    }
                }
                tmp = O->GetProperty(P);
                if(!tmp->IsUndefined())
                {
                    PropertyDescriptor* desc = static_cast<PropertyDescriptor*>(tmp);
                    if(desc->IsAccessorDescriptor())
                    {// 4
                        JSValue* setter = desc->Set();
                        assert(!setter->IsUndefined());
                        JSObject* setter_obj = static_cast<JSObject*>(setter);
                        setter_obj->Call(e, base, { W });
                    }
                    else
                    {// 7
                        if(throw_flag)
                            *e = *Error::TypeError();
                        return;
                    }
                }
            }
        }
        else
        {
            assert(base->IsEnvironmentRecord());
            EnvironmentRecord* er = static_cast<EnvironmentRecord*>(base);
            er->SetMutableBinding(e, ref->GetReferencedName(), W, ref->IsStrictReference());
        }
    }

    class LexicalEnvironment : public JSValue
    {
    public:
        LexicalEnvironment(JSValue* outer, EnvironmentRecord* env_rec) : JSValue(JS_LEX_ENV), env_rec_(env_rec)
        {
            assert(outer->IsNull() || outer->IsLexicalEnvironment());
            outer_ = outer;
        }

        static LexicalEnvironment* Global()
        {
            static LexicalEnvironment singleton(Null::Instance(), new ObjectEnvironmentRecord(GlobalObject::Instance()));
            return &singleton;
        }

        Reference* GetIdentifierReference(const std::u16string& name, bool strict)
        {
            bool exists = env_rec_->HasBinding(name);
            if(exists)
            {
                return new Reference(env_rec_, name, strict);
            }
            if(outer_->IsNull())
            {
                return new Reference(Undefined::Instance(), name, strict);
            }
            LexicalEnvironment* outer = static_cast<LexicalEnvironment*>(outer_);
            return outer->GetIdentifierReference(name, strict);
        }

        static LexicalEnvironment* NewDeclarativeEnvironment(JSValue* lex)
        {
            DeclarativeEnvironmentRecord* env_rec = new DeclarativeEnvironmentRecord();
            return new LexicalEnvironment(lex, env_rec);
        }

        static LexicalEnvironment* NewObjectEnvironment(JSObject* obj, JSValue* lex, bool provide_this = false)
        {
            ObjectEnvironmentRecord* env_rec = new ObjectEnvironmentRecord(obj, provide_this);
            return new LexicalEnvironment(lex, env_rec);
        }

        JSValue* outer()
        {
            return outer_;
        }
        EnvironmentRecord* env_rec()
        {
            return env_rec_;
        }

        std::string ToString() override
        {
            return "LexicalEnvironment";
        }

    private:
        JSValue* outer_;// not owned
        EnvironmentRecord* env_rec_;
    };

    class ExecutionContext
    {
    public:
        ExecutionContext(LexicalEnvironment* variable_env, LexicalEnvironment* lexical_env, JSValue* this_binding, bool strict)
        : variable_env_(variable_env), lexical_env_(lexical_env), this_binding_(this_binding), strict_(strict),
          iteration_layers_(0)
        {
        }

        LexicalEnvironment* variable_env()
        {
            return variable_env_;
        }
        LexicalEnvironment* lexical_env()
        {
            return lexical_env_;
        }
        JSValue* this_binding()
        {
            return this_binding_;
        }
        bool strict()
        {
            return strict_;
        }

        void SetLexicalEnv(LexicalEnvironment* lexical_env)
        {
            lexical_env_ = lexical_env;
        }

        bool HasLabel(const std::u16string& label)
        {
            if(label == u"")
                return true;
            return label_stack_.size() && label_stack_.top() == label;
        }

        void AddLabel(const std::u16string& label)
        {
            assert(!HasLabel(label));
            label_stack_.push(label);
        }

        void RemoveLabel(const std::u16string& label)
        {
            if(label == u"")
                return;
            assert(HasLabel(label));
            label_stack_.pop();
        }

        void EnterIteration()
        {
            iteration_layers_++;
        }
        void ExitIteration()
        {
            if(iteration_layers_ != 0)
                iteration_layers_--;
        }
        bool InIteration()
        {
            return iteration_layers_ != 0;
        }

    private:
        LexicalEnvironment* variable_env_;
        LexicalEnvironment* lexical_env_;
        JSValue* this_binding_;
        bool strict_;
        std::stack<std::u16string> label_stack_;
        size_t iteration_layers_;
    };

    class RuntimeContext
    {
    public:
        static RuntimeContext* Global()
        {
            static RuntimeContext singleton;
            return &singleton;
        }

        void AddContext(ExecutionContext* context)
        {
            context_stack_.push(context);
            if(context_stack_.size() == 1)
                global_env_ = context;
        }

        static ExecutionContext* TopContext()
        {
            return RuntimeContext::Global()->context_stack_.top();
        }

        static LexicalEnvironment* TopLexicalEnv()
        {
            return RuntimeContext::TopContext()->lexical_env();
        }

        void PopContext()
        {
            ExecutionContext* top = context_stack_.top();
            context_stack_.pop();
            delete top;
        }

        static JSValue* TopValue()
        {
            return RuntimeContext::Global()->value_stack_.top();
        }

        void AddValue(JSValue* val)
        {
            value_stack_.push(val);
        }

        void PopValue()
        {
            value_stack_.pop();
        }

        ExecutionContext* global_env()
        {
            return global_env_;
        }

    private:
        RuntimeContext()
        {
            value_stack_.push(Null::Instance());
        }

        std::stack<ExecutionContext*> context_stack_;
        ExecutionContext* global_env_;
        // This is to make sure builtin function like `array.push()`
        // can visit `array`.
        std::stack<JSValue*> value_stack_;
    };

    class ValueGuard
    {
    public:
        ValueGuard() : count_(0)
        {
        }
        ~ValueGuard()
        {
            while(count_ > 0)
            {
                RuntimeContext::Global()->PopValue();
                count_--;
            }
        }

        void AddValue(JSValue* val)
        {
            RuntimeContext::Global()->AddValue(val);
            count_++;
        }

    private:
        size_t count_;
    };

    // TODO(zhuzilin) move this method to a better place
    JSValue* JSObject::DefaultValue(Error* e, const std::u16string& hint)
    {
        std::u16string first, second;
        if(hint == u"String" || hint == u"" && obj_type() == OBJ_DATE)
        {
            first = u"toString";
            second = u"valueOf";
        }
        else if(hint == u"Number" || hint == u"" && obj_type() != OBJ_DATE)
        {
            first = u"valueOf";
            second = u"toString";
        }
        else
        {
            assert(false);
        }

        ValueGuard guard;
        guard.AddValue(this);

        JSValue* to_string = Get(e, first);
        if(!e->IsOk())
            return nullptr;
        if(to_string->IsCallable())
        {
            JSObject* to_string_obj = static_cast<JSObject*>(to_string);
            JSValue* str = to_string_obj->Call(e, this);
            if(!e->IsOk())
                return nullptr;
            if(str->IsPrimitive())
            {
                return str;
            }
        }
        JSValue* value_of = Get(e, second);
        if(!e->IsOk())
            return nullptr;
        if(value_of->IsCallable())
        {
            JSObject* value_of_obj = static_cast<JSObject*>(value_of);
            JSValue* val = value_of_obj->Call(e, this);
            if(!e->IsOk())
                return nullptr;
            if(val->IsPrimitive())
            {
                return val;
            }
        }
        *e = *Error::TypeError(u"failed to get [[DefaultValue]]");
        return nullptr;
    }

    std::u16string ToString(Error* e, JSValue* input);
    PropertyDescriptor* ToPropertyDescriptor(Error* e, JSValue* obj);

    class ObjectProto : public JSObject
    {
    public:
        static ObjectProto* Instance()
        {
            static ObjectProto singleton;
            return &singleton;
        }

        static JSValue* toString(Error* e, JSValue* this_arg, const std::vector<JSValue*>& vals)
        {
            JSValue* val = RuntimeContext::TopValue();
            if(val->IsUndefined())
                return new String(u"[object Undefined]");
            if(val->IsNull())
                return new String(u"[object Null]");
            JSObject* obj = ToObject(e, val);
            return new String(u"[object " + obj->Class() + u"]");
        }

        static JSValue* toLocaleString(Error* e, JSValue* this_arg, const std::vector<JSValue*>& vals)
        {
            assert(false);
        }

        static JSValue* valueOf(Error* e, JSValue* this_arg, const std::vector<JSValue*>& vals)
        {
            JSValue* val = RuntimeContext::TopValue();
            JSObject* O = ToObject(e, val);
            if(!e->IsOk())
                return nullptr;
            // TODO(zhuzilin) Host object
            return O;
        }

        static JSValue* hasOwnProperty(Error* e, JSValue* this_arg, const std::vector<JSValue*>& vals)
        {
            assert(false);
        }

        static JSValue* isPrototypeOf(Error* e, JSValue* this_arg, const std::vector<JSValue*>& vals)
        {
            assert(false);
        }

        static JSValue* propertyIsEnumerable(Error* e, JSValue* this_arg, const std::vector<JSValue*>& vals)
        {
            assert(false);
        }

    private:
        ObjectProto() : JSObject(OBJ_OTHER, u"Object", true, nullptr, false, false)
        {
        }
    };

    class Object : public JSObject
    {
    public:
        Object() : JSObject(OBJ_OTHER, u"Object", true, nullptr, false, false)
        {
            SetPrototype(ObjectProto::Instance());
        }
    };

    class ObjectConstructor : public JSObject
    {
    public:
        static ObjectConstructor* Instance()
        {
            static ObjectConstructor singleton;
            return &singleton;
        }

        // 15.2.1 The Object Constructor Called as a Function
        JSValue* Call(Error* e, JSValue* this_arg, const std::vector<JSValue*>& arguments = {}) override
        {
            if(arguments.size() == 0 || arguments[0]->IsNull() || arguments[0]->IsUndefined())
                return Construct(e, arguments);
            return ToObject(e, arguments[0]);
        }

        // 15.2.2 The Object Constructor
        JSObject* Construct(Error* e, const std::vector<JSValue*>& arguments) override
        {
            if(arguments.size() > 0)
            {// 1
                JSValue* value = arguments[0];
                switch(value->type())
                {
                    case JSValue::JS_OBJECT:
                        // TODO(zhuzilin) deal with host object.
                        return static_cast<JSObject*>(value);
                    case JSValue::JS_STRING:
                    case JSValue::JS_BOOL:
                    case JSValue::JS_NUMBER:
                        return ToObject(e, value);
                    default:
                        break;
                }
            }
            assert(arguments.size() == 0 || arguments[0]->IsNull() || arguments[0]->IsUndefined());
            JSObject* obj = new Object();
            return obj;
        }

        // 15.2.3.2 Object.getPrototypeOf ( O )
        static JSValue* getPrototypeOf(Error* e, JSValue* this_arg, const std::vector<JSValue*>& vals)
        {
            if(vals.size() < 1 || !vals[0]->IsObject())
            {
                *e = *Error::TypeError();
                return nullptr;
            }
            return static_cast<JSObject*>(vals[0])->Prototype();
        }

        // 15.2.3.3 Object.getOwnPropertyDescriptor ( O, P )
        static JSValue* getOwnPropertyDescriptor(Error* e, JSValue* this_arg, const std::vector<JSValue*>& vals)
        {
            assert(false);
        }

        static JSValue* getOwnPropertyNames(Error* e, JSValue* this_arg, const std::vector<JSValue*>& vals)
        {
            assert(false);
        }

        static JSValue* create(Error* e, JSValue* this_arg, const std::vector<JSValue*>& vals)
        {
            if(vals.size() < 1 || (!vals[0]->IsObject() && !vals[0]->IsNull()))
            {
                *e = *Error::TypeError(u"Object.create called on non-object");
                return nullptr;
            }
            Object* obj = new Object();
            obj->SetPrototype(vals[0]);
            if(vals.size() > 1 && !vals[1]->IsUndefined())
            {
                ObjectConstructor::defineProperties(e, this_arg, vals);
                if(!e->IsOk())
                    return nullptr;
            }
            return obj;
        }

        static JSValue* defineProperty(Error* e, JSValue* this_arg, const std::vector<JSValue*>& vals)
        {
            if(vals.size() < 1 || !vals[0]->IsObject())
            {
                *e = *Error::TypeError(u"Object.defineProperty called on non-object");
                return nullptr;
            }
            JSObject* O = static_cast<JSObject*>(vals[0]);
            if(vals.size() < 2)
            {
                *e = *Error::TypeError(u"Object.defineProperty need 3 arguments");
                return nullptr;
            }
            std::u16string name = ::es::ToString(e, vals[1]);
            if(!e->IsOk())
                return nullptr;
            PropertyDescriptor* desc = ToPropertyDescriptor(e, vals[2]);
            if(!e->IsOk())
                return nullptr;
            O->DefineOwnProperty(e, name, desc, true);
            return O;
        }

        static JSValue* defineProperties(Error* e, JSValue* this_arg, const std::vector<JSValue*>& vals)
        {
            assert(false);
        }

        static JSValue* seal(Error* e, JSValue* this_arg, const std::vector<JSValue*>& vals)
        {
            assert(false);
        }

        static JSValue* freeze(Error* e, JSValue* this_arg, const std::vector<JSValue*>& vals)
        {
            assert(false);
        }

        static JSValue* preventExtensions(Error* e, JSValue* this_arg, const std::vector<JSValue*>& vals)
        {
            if(vals.size() == 0 || !vals[0]->IsObject())
            {
                *e = *Error::TypeError(u"Object.preventExtensions called on non-object");
                return nullptr;
            }
            JSObject* obj = static_cast<JSObject*>(vals[0]);
            obj->SetExtensible(false);
            return obj;
        }

        static JSValue* isSealed(Error* e, JSValue* this_arg, const std::vector<JSValue*>& vals)
        {
            assert(false);
        }

        static JSValue* isFrozen(Error* e, JSValue* this_arg, const std::vector<JSValue*>& vals)
        {
            assert(false);
        }

        static JSValue* isExtensible(Error* e, JSValue* this_arg, const std::vector<JSValue*>& vals)
        {
            if(vals.size() < 1 || !vals[0]->IsObject())
            {
                *e = *Error::TypeError(u"Object.isExtensible called on non-object");
                return nullptr;
            }
            JSObject* obj = static_cast<JSObject*>(vals[0]);
            return Bool::Wrap(obj->Extensible());
        }

        static JSValue* keys(Error* e, JSValue* this_arg, const std::vector<JSValue*>& vals);

        // ES6
        static JSValue* setPrototypeOf(Error* e, JSValue* this_arg, const std::vector<JSValue*>& vals)
        {
            if(vals.size() < 2)
            {
                *e = *Error::TypeError(u"Object.preventExtensions need 2 arguments");
                return nullptr;
            }
            vals[0]->CheckObjectCoercible(e);
            if(!e->IsOk())
                return nullptr;
            if(!(vals[1]->IsNull() || vals[1]->IsObject()))
            {
                *e = *Error::TypeError(u"");
                return nullptr;
            }
            if(!vals[0]->IsObject())
            {
                return vals[0];
            }
            static_cast<JSObject*>(vals[0])->SetPrototype(vals[1]);
            return vals[0];
        }

        static JSValue* toString(Error* e, JSValue* this_arg, const std::vector<JSValue*>& vals)
        {
            return new String(u"function Object() { [native code] }");
        }

    private:
        ObjectConstructor() : JSObject(OBJ_OTHER, u"Object", true, nullptr, true, true)
        {
        }
    };

    bool ToBoolean(JSValue* input);

    JSValue* FromPropertyDescriptor(Error* e, JSValue* value)
    {
        if(value->IsUndefined())
        {
            return Undefined::Instance();
        }
        PropertyDescriptor* desc = static_cast<PropertyDescriptor*>(value);
        JSObject* obj = new Object();
        if(desc->IsDataDescriptor())
        {
            PropertyDescriptor* value_desc = new PropertyDescriptor();
            value_desc->SetDataDescriptor(desc->Value(), true, true, true);
            obj->DefineOwnProperty(e, u"value", value_desc, false);
            if(!e->IsOk())
                return nullptr;

            PropertyDescriptor* writable_desc = new PropertyDescriptor();
            writable_desc->SetDataDescriptor(Bool::Wrap(desc->Writable()), true, true, true);
            obj->DefineOwnProperty(e, u"writable", writable_desc, false);
            if(!e->IsOk())
                return nullptr;
        }
        else
        {
            assert(desc->IsAccessorDescriptor());
            PropertyDescriptor* get_desc = new PropertyDescriptor();
            get_desc->SetDataDescriptor(desc->Get(), true, true, true);
            obj->DefineOwnProperty(e, u"get", get_desc, false);
            if(!e->IsOk())
                return nullptr;

            PropertyDescriptor* set_desc = new PropertyDescriptor();
            set_desc->SetDataDescriptor(desc->Set(), true, true, true);
            obj->DefineOwnProperty(e, u"set", set_desc, false);
            if(!e->IsOk())
                return nullptr;
        }

        PropertyDescriptor* enumerable_desc = new PropertyDescriptor();
        enumerable_desc->SetDataDescriptor(Bool::Wrap(desc->Enumerable()), true, true, true);
        obj->DefineOwnProperty(e, u"get", enumerable_desc, false);
        if(!e->IsOk())
            return nullptr;

        PropertyDescriptor* configurable_desc = new PropertyDescriptor();
        configurable_desc->SetDataDescriptor(Bool::Wrap(desc->Configurable()), true, true, true);
        obj->DefineOwnProperty(e, u"set", configurable_desc, false);
        if(!e->IsOk())
            return nullptr;

        return obj;
    }

    PropertyDescriptor* ToPropertyDescriptor(Error* e, JSValue* val)
    {
        if(!val->IsObject())
        {
            *e = *Error::TypeError();
            return nullptr;
        }
        JSObject* obj = static_cast<JSObject*>(val);
        PropertyDescriptor* desc = new PropertyDescriptor();
        if(obj->HasProperty(u"enumerable"))
        {
            JSValue* value = obj->Get(e, u"enumerable");
            desc->SetEnumerable(ToBoolean(value));
        }
        if(obj->HasProperty(u"configurable"))
        {
            JSValue* value = obj->Get(e, u"configurable");
            desc->SetConfigurable(ToBoolean(value));
        }
        if(obj->HasProperty(u"value"))
        {
            JSValue* value = obj->Get(e, u"value");
            desc->SetValue(value);
        }
        if(obj->HasProperty(u"writable"))
        {
            JSValue* value = obj->Get(e, u"writable");
            desc->SetWritable(ToBoolean(value));
        }
        if(obj->HasProperty(u"get"))
        {
            JSValue* value = obj->Get(e, u"get");
            if(!value->IsCallable() && !value->IsUndefined())
            {
                *e = *Error::TypeError(u"getter not callable.");
            }
            desc->SetGet(value);
        }
        if(obj->HasProperty(u"set"))
        {
            JSValue* value = obj->Get(e, u"set");
            if(!value->IsCallable() && !value->IsUndefined())
            {
                *e = *Error::TypeError(u"setter not callable.");
            }
            desc->SetSet(value);
        }
        if(desc->HasSet() || desc->HasGet())
        {
            if(desc->HasValue() || desc->HasWritable())
            {
                *e = *Error::TypeError(u"cannot have both get/set and value/writable");
                return nullptr;
            }
        }
        return desc;
    }

    class ErrorProto : public JSObject
    {
    public:
        static ErrorProto* Instance()
        {
            static ErrorProto singleton;
            return &singleton;
        }

        JSValue* Call(Error* e, JSValue* this_arg, const std::vector<JSValue*>& arguments = {}) override
        {
            return Undefined::Instance();
        }

        static JSValue* toString(Error* e, JSValue* this_arg, const std::vector<JSValue*>& vals)
        {
            assert(false);
        }

    private:
        ErrorProto() : JSObject(OBJ_ERROR, u"Error", true, nullptr, false, true)
        {
        }
    };

    class ErrorObject : public JSObject
    {
    public:
        ErrorObject(Error* e) : JSObject(OBJ_ERROR, u"Error", true, nullptr, false, false), e_(e)
        {
            SetPrototype(ErrorProto::Instance());
            AddValueProperty(u"message", new String(e->message()), true, false, false);
        }

        Error* e()
        {
            return e_;
        }
        Error::Type ErrorType()
        {
            return e_->type();
        }
        std::u16string ErrorMessage()
        {
            return e_->message();
        }

        std::string ToString()
        {
            return log::ToString(e_->message());
        }

    private:
        Error* e_;
    };

    class ErrorConstructor : public JSObject
    {
    public:
        static ErrorConstructor* Instance()
        {
            static ErrorConstructor singleton;
            return &singleton;
        }

        JSValue* Call(Error* e, JSValue* this_arg, const std::vector<JSValue*>& arguments = {}) override
        {
            return Construct(e, arguments);
        }

        JSObject* Construct(Error* e, const std::vector<JSValue*>& arguments) override
        {
            if(arguments.size() == 0 || arguments[0]->IsUndefined())
                return new ErrorObject(Error::NativeError(::es::ToString(nullptr, Undefined::Instance())));
            std::u16string s = ::es::ToString(e, arguments[0]);
            if(!e->IsOk())
                return nullptr;
            return new ErrorObject(Error::NativeError(s));
        }

        static JSValue* toString(Error* e, JSValue* this_arg, const std::vector<JSValue*>& vals)
        {
            return new String(u"function Error() { [native code] }");
        }

    private:
        ErrorConstructor() : JSObject(OBJ_OTHER, u"Error", true, nullptr, true, true)
        {
        }
    };

    // NOTE(zhuzilin) Completion is the spec type to represent
    // the evaluation result of the statement.
    // It won't interact with other types, so does not need to
    // inherit JSValue.
    struct Completion
    {
        enum Type
        {
            NORMAL,
            BREAK,
            CONTINUE,
            RETURN,
            THROW,
        };

        Completion() : Completion(NORMAL, nullptr, u"")
        {
        }

        Completion(Type type, JSValue* value, const std::u16string& target) : type(type), value(value), target(target)
        {
        }

        bool IsAbruptCompletion()
        {
            return type != NORMAL;
        }
        bool IsThrow()
        {
            return type == THROW;
        }

        Type type;
        JSValue* value;
        std::u16string target;
    };

    double ToNumber(Error* e, JSValue* input);
    std::u16string NumberToString(double m);
    Completion EvalProgram(AST* ast);

    class FunctionProto : public JSObject
    {
    public:
        static FunctionProto* Instance()
        {
            static FunctionProto singleton;
            return &singleton;
        }

        JSValue* Call(Error* e, JSValue* this_arg, const std::vector<JSValue*>& arguments = {}) override
        {
            return Undefined::Instance();
        }

        static JSValue* toString(Error* e, JSValue* this_arg, const std::vector<JSValue*>& vals);

        // 15.3.4.3 Function.prototype.apply (thisArg, argArray)
        static JSValue* apply(Error* e, JSValue* this_arg, const std::vector<JSValue*>& vals)
        {
            JSValue* val = RuntimeContext::TopValue();
            if(!val->IsObject())
            {
                *e = *Error::TypeError(u"Function.prototype.apply called on non-object");
                return nullptr;
            }
            JSObject* func = static_cast<JSObject*>(val);
            if(!func->IsCallable())
            {
                *e = *Error::TypeError(u"Function.prototype.apply called on non-callable");
                return nullptr;
            }
            if(vals.size() == 0)
            {
                return func->Call(e, Undefined::Instance(), {});
            }
            if(vals.size() < 2 || vals[1]->IsNull() || vals[1]->IsUndefined())
            {// 2
                return func->Call(e, vals[0], {});
            }
            if(!vals[1]->IsObject())
            {// 3
                *e = *Error::TypeError(u"Function.prototype.apply's argument is non-object");
                return nullptr;
            }
            JSObject* arg_array = static_cast<JSObject*>(vals[1]);
            JSValue* len = arg_array->Get(e, u"length");
            if(!e->IsOk())
                return nullptr;
            size_t n = ToNumber(e, len);
            std::vector<JSValue*> arg_list;// 6
            size_t index = 0;// 7
            while(index < n)
            {// 8
                std::u16string index_name = ::es::NumberToString(index);
                if(!e->IsOk())
                    return nullptr;
                JSValue* next_arg = arg_array->Get(e, index_name);
                if(!e->IsOk())
                    return nullptr;
                arg_list.emplace_back(next_arg);
                index++;
            }
            return func->Call(e, vals[0], arg_list);
        }

        static JSValue* call(Error* e, JSValue* this_arg, const std::vector<JSValue*>& vals)
        {
            JSValue* val = RuntimeContext::TopValue();
            if(!val->IsObject())
            {
                *e = *Error::TypeError(u"Function.prototype.call called on non-object");
                return nullptr;
            }
            JSObject* func = static_cast<JSObject*>(val);
            if(!func->IsCallable())
            {
                *e = *Error::TypeError(u"Function.prototype.call called on non-callable");
                return nullptr;
            }
            if(vals.size())
            {
                JSValue* this_arg = vals[0];
                return func->Call(e, this_arg, std::vector<JSValue*>(vals.begin() + 1, vals.end()));
            }
            else
            {
                return func->Call(e, Undefined::Instance(), {});
            }
        }

        static JSValue* bind(Error* e, JSValue* this_arg, const std::vector<JSValue*>& vals);

    private:
        FunctionProto() : JSObject(OBJ_FUNC, u"Function", true, nullptr, false, true)
        {
        }
    };

    void EnterFunctionCode(Error* e, JSObject* f, ProgramOrFunctionBody* body, JSValue* this_arg, const std::vector<JSValue*>& args, bool strict);

    class FunctionObject : public JSObject
    {
    public:
        FunctionObject(const std::vector<std::u16string>& names, AST* body, LexicalEnvironment* scope, bool from_bind_ = false)
        : JSObject(OBJ_FUNC, u"Function", true, nullptr, true, true), formal_params_(names), scope_(scope), from_bind_(from_bind_)
        {
            // 13.2 Creating Function Objects
            SetPrototype(FunctionProto::Instance());
            // Whether the function is made from bind.
            if(body != nullptr)
            {
                assert(body->type() == AST::AST_FUNC_BODY);
                body_ = static_cast<ProgramOrFunctionBody*>(body);
                strict_ = body_->strict() || RuntimeContext::TopContext()->strict();
                AddValueProperty(u"length", new Number(names.size()), false, false, false);// 14 & 15
                JSObject* proto = new Object();// 16
                proto->AddValueProperty(u"constructor", this, true, false, true);
                // 15.3.5.2 prototype
                AddValueProperty(u"prototype", proto, true, false, false);
                if(strict_)
                {
                    // TODO(zhuzilin) thrower
                }
            }
        }

        virtual LexicalEnvironment* Scope()
        {
            return scope_;
        };
        virtual std::vector<std::u16string> FormalParameters()
        {
            return formal_params_;
        };
        virtual AST* Code()
        {
            return body_;
        }
        virtual bool strict()
        {
            return strict_;
        }
        bool from_bind()
        {
            return from_bind_;
        }

        // 13.2.1 [[Call]]
        virtual JSValue* Call(Error* e, JSValue* this_arg, const std::vector<JSValue*>& arguments) override
        {
            log::PrintSource("enter FunctionObject::Call ", body_->source());
            EnterFunctionCode(e, this, body_, this_arg, arguments, strict_);
            if(!e->IsOk())
                return nullptr;

            Completion result;
            if(body_ != nullptr)
            {
                result = EvalProgram(body_);
            }
            RuntimeContext::Global()->PopContext();// 3

            switch(result.type)
            {
                case Completion::RETURN:
                    return result.value;
                case Completion::THROW:
                {
                    std::u16string message = ::es::ToString(e, result.value);
                    if(result.value->IsObject())
                    {
                        JSObject* obj = static_cast<JSObject*>(result.value);
                        if(obj->obj_type() == JSObject::OBJ_ERROR)
                        {
                            *e = *(static_cast<ErrorObject*>(obj)->e());
                            return nullptr;
                        }
                    }
                    *e = *Error::NativeError(message);
                    return nullptr;
                }
                default:
                    assert(result.type == Completion::NORMAL);
                    return Undefined::Instance();
            }
        }

        // 13.2.2 [[Construct]]
        virtual JSObject* Construct(Error* e, const std::vector<JSValue*>& arguments) override
        {
            log::PrintSource("enter FunctionObject::Construct");
            JSObject* obj = new JSObject(OBJ_OTHER, u"Object", true, nullptr, false, false);
            JSValue* proto = Get(e, u"prototype");
            if(!e->IsOk())
                return nullptr;
            if(proto->IsObject())
            {// 6
                obj->SetPrototype(proto);
            }
            else
            {// 7
                obj->SetPrototype(ObjectProto::Instance());
            }
            JSValue* result = Call(e, obj, arguments);// 8
            if(!e->IsOk())
                return nullptr;
            if(result->IsObject())// 9
                return static_cast<JSObject*>(result);
            return obj;// 10
        }

        // 15.3.5.3 [[HasInstance]] (V)
        virtual bool HasInstance(Error* e, JSValue* V) override
        {
            if(!V->IsObject())
                return false;
            JSValue* O = Get(e, u"prototype");
            if(!e->IsOk())
                return false;
            if(!O->IsObject())
            {
                *e = *Error::TypeError();
                return false;
            }
            while(!V->IsNull())
            {
                if(V == O)
                    return true;
                V = static_cast<JSObject*>(V)->Get(e, u"prototype");
                if(!e->IsOk())
                    return false;
            }
            return false;
        }

        // 15.3.5.4 [[Get]] (P)
        JSValue* Get(Error* e, const std::u16string& P) override
        {
            JSValue* v = JSObject::Get(e, P);
            if(!e->IsOk())
                return nullptr;
            if(P == u"caller")
            {// 2
                if(v->IsObject())
                {
                    JSObject* v_obj = static_cast<JSObject*>(v);
                    if(v_obj->IsFunction())
                    {
                        FunctionObject* v_func = static_cast<FunctionObject*>(v);
                        if(v_func->strict())
                        {
                            *e = *Error::TypeError();
                            return nullptr;
                        }
                    }
                }
            }
            return v;
        }

        std::string ToString() override
        {
            std::string result = "Function(";
            if(formal_params_.size() > 0)
            {
                result += log::ToString(formal_params_[0]);
                for(size_t i = 1; i < formal_params_.size(); i++)
                {
                    result += "," + log::ToString(formal_params_[i]);
                }
            }
            result += ")";
            return result;
        }

    protected:
        bool from_bind_;

    private:
        std::vector<std::u16string> formal_params_;
        LexicalEnvironment* scope_;
        ProgramOrFunctionBody* body_;
        bool strict_;
    };

    class BindFunctionObject : public FunctionObject
    {
    public:
        BindFunctionObject(JSObject* target_function, JSValue* bound_this, const std::vector<JSValue*>& bound_args)
        : FunctionObject({}, nullptr, nullptr, true), target_function_(target_function), bound_this_(bound_this),
          bound_args_(bound_args)
        {
        }

        LexicalEnvironment* Scope() override
        {
            assert(false);
        };
        std::vector<std::u16string> FormalParameters() override
        {
            assert(false);
        };
        AST* Code() override
        {
            assert(false);
        }
        bool strict() override
        {
            assert(false);
        }

        JSObject* TargetFunction()
        {
            return target_function_;
        }
        JSValue* BoundThis()
        {
            return bound_this_;
        }
        std::vector<JSValue*> BoundArgs()
        {
            return bound_args_;
        }

        virtual JSValue* Call(Error* e, JSValue* this_arg, const std::vector<JSValue*>& extra_args) override
        {
            std::vector<JSValue*> args;
            args.insert(args.end(), bound_args_.begin(), bound_args_.end());
            args.insert(args.end(), extra_args.begin(), extra_args.end());
            return target_function_->Call(e, bound_this_, args);
        }

        // 13.2.2 [[Construct]]
        virtual JSObject* Construct(Error* e, const std::vector<JSValue*>& extra_args) override
        {
            if(!target_function_->IsConstructor())
            {
                *e = *Error::TypeError(u"target function has no [[Construct]] internal method");
                return nullptr;
            }
            std::vector<JSValue*> args;
            args.insert(args.end(), bound_args_.begin(), bound_args_.end());
            args.insert(args.end(), extra_args.begin(), extra_args.end());
            return target_function_->Construct(e, args);
        }

        // 15.3.4.5.3 [[HasInstance]] (V)
        virtual bool HasInstance(Error* e, JSValue* V) override
        {
            return target_function_->HasInstance(e, V);
        }

    private:
        JSObject* target_function_;
        JSValue* bound_this_;
        std::vector<JSValue*> bound_args_;
    };

    class FunctionConstructor : public JSObject
    {
    public:
        static FunctionConstructor* Instance()
        {
            static FunctionConstructor singleton;
            return &singleton;
        }

        // 15.3.1 The Function Constructor Called as a Function
        JSValue* Call(Error* e, JSValue* this_arg, const std::vector<JSValue*>& arguments = {}) override
        {
            return Construct(e, arguments);
        }

        // 15.3.2.1 new Function (p1, p2, … , pn, body)
        JSObject* Construct(Error* e, const std::vector<JSValue*>& arguments) override
        {
            log::PrintSource("enter FunctionConstructor::Construct");
            size_t arg_count = arguments.size();
            std::u16string P = u"";
            std::u16string body = u"";
            if(arg_count == 1)
            {
                body = ::es::ToString(e, arguments[0]);
                if(!e->IsOk())
                    return nullptr;
            }
            else if(arg_count > 1)
            {
                P += ::es::ToString(e, arguments[0]);
                if(!e->IsOk())
                    return nullptr;
                for(size_t i = 1; i < arg_count - 1; i++)
                {
                    P += u"," + ::es::ToString(e, arguments[i]);
                    if(!e->IsOk())
                        return nullptr;
                }
                body = ::es::ToString(e, arguments[arg_count - 1]);
                if(!e->IsOk())
                    return nullptr;
            }
            std::vector<std::u16string> names;
            AST* body_ast;
            if(P.size() > 0)
            {
                Parser parser(P);
                names = parser.ParseFormalParameterList();
                if(names.size() == 0)
                {
                    *e = *Error::SyntaxError(u"invalid parameter name");
                    return nullptr;
                }
            }
            {
                Parser parser(body);
                body_ast = parser.ParseFunctionBody(Token::TK_EOS);
                if(body_ast->IsIllegal())
                {
                    *e = *Error::SyntaxError(u"failed to parse function body: " + body_ast->source());
                    return nullptr;
                }
            }
            LexicalEnvironment* scope = LexicalEnvironment::Global();
            bool strict = static_cast<ProgramOrFunctionBody*>(body_ast)->strict();
            if(strict)
            {
                // 13.1
                if(HaveDuplicate(names))
                {
                    *e = *Error::SyntaxError();
                    return nullptr;
                }
                for(auto name : names)
                {
                    if(name == u"eval" || name == u"arguments")
                    {
                        *e = *Error::SyntaxError();
                        return nullptr;
                    }
                }
            }
            return new FunctionObject(names, body_ast, scope);
        }

        static JSValue* toString(Error* e, JSValue* this_arg, const std::vector<JSValue*>& vals)
        {
            return new String(u"function Function() { [native code] }");
        }

    private:
        FunctionConstructor() : JSObject(OBJ_OTHER, u"Function", true, nullptr, true, true)
        {
        }
    };

    JSValue* FunctionProto::toString(Error* e, JSValue* this_arg, const std::vector<JSValue*>& vals)
    {
        JSValue* val = RuntimeContext::TopValue();
        if(!val->IsObject())
        {
            *e = *Error::TypeError(u"Function.prototype.toString called on non-object");
            return nullptr;
        }
        JSObject* obj = static_cast<JSObject*>(val);
        if(obj->obj_type() != JSObject::OBJ_FUNC)
        {
            *e = *Error::TypeError(u"Function.prototype.toString called on non-function");
            return nullptr;
        }
        FunctionObject* func = static_cast<FunctionObject*>(obj);
        std::u16string str = u"function (";
        auto params = func->FormalParameters();
        if(params.size() > 0)
        {
            str += params[0];
            for(size_t i = 1; i < params.size(); i++)
            {
                str += u"," + params[i];
            }
        }
        str += u")";
        return new String(str);
    }

    // 15.3.4.5 Function.prototype.bind (thisArg [, arg1 [, arg2, …]])
    JSValue* FunctionProto::bind(Error* e, JSValue* this_arg, const std::vector<JSValue*>& vals)
    {
        JSValue* val = RuntimeContext::TopValue();
        if(!val->IsCallable())
        {
            *e = *Error::TypeError(u"Function.prototype.call called on non-callable");
            return nullptr;
        }
        JSObject* target = static_cast<JSObject*>(val);
        JSValue* this_arg_for_F = Undefined::Instance();
        if(vals.size() > 0)
            this_arg_for_F = vals[0];
        std::vector<JSValue*> A;
        if(vals.size() > 1)
        {
            A = std::vector<JSValue*>(vals.begin() + 1, vals.end());
        }
        BindFunctionObject* F = new BindFunctionObject(target, this_arg_for_F, A);
        size_t len = 0;
        if(target->Class() == u"Function")
        {
            size_t L = ToNumber(e, target->Get(e, u"length"));
            if(L - A.size() > 0)
                len = L - A.size();
        }
        F->AddValueProperty(u"length", new Number(len), false, false, false);
        // 19
        // TODO(zhuzilin) thrower
        return F;
    }

    FunctionObject* InstantiateFunctionDeclaration(Error* e, Function* func_ast)
    {
        assert(func_ast->is_named());
        std::u16string identifier = func_ast->name();
        auto func_env = LexicalEnvironment::NewDeclarativeEnvironment(// 1
        RuntimeContext::TopLexicalEnv());
        auto env_rec = static_cast<DeclarativeEnvironmentRecord*>(func_env->env_rec());// 2
        env_rec->CreateImmutableBinding(identifier);// 3
        auto body = static_cast<ProgramOrFunctionBody*>(func_ast->body());
        bool strict = body->strict() || RuntimeContext::TopContext()->strict();
        if(strict)
        {
            // 13.1
            if(HaveDuplicate(func_ast->params()))
            {
                *e = *Error::SyntaxError();
                return nullptr;
            }
            for(auto name : func_ast->params())
            {
                if(name == u"eval" || name == u"arguments")
                {
                    *e = *Error::SyntaxError();
                    return nullptr;
                }
            }
            if(func_ast->name() == u"eval" || func_ast->name() == u"arguments")
            {
                *e = *Error::SyntaxError();
                return nullptr;
            }
        }
        FunctionObject* closure = new FunctionObject(func_ast->params(), func_ast->body(), func_env);// 4
        env_rec->InitializeImmutableBinding(identifier, closure);// 5
        return closure;// 6
    }

    JSValue* EvalFunction(Error* e, AST* ast)
    {
        assert(ast->type() == AST::AST_FUNC);
        Function* func_ast = static_cast<Function*>(ast);

        if(func_ast->is_named())
        {
            return InstantiateFunctionDeclaration(e, func_ast);
        }
        else
        {
            auto body = static_cast<ProgramOrFunctionBody*>(func_ast->body());
            bool strict = body->strict() || RuntimeContext::TopContext()->strict();
            if(strict)
            {
                // 13.1
                if(HaveDuplicate(func_ast->params()))
                {
                    *e = *Error::SyntaxError();
                    return nullptr;
                }
                for(auto name : func_ast->params())
                {
                    if(name == u"eval" || name == u"arguments")
                    {
                        *e = *Error::SyntaxError();
                        return nullptr;
                    }
                }
            }
            return new FunctionObject(func_ast->params(), func_ast->body(), RuntimeContext::TopLexicalEnv());
        }
    }

    // TODO(zhuzilin) move this function to a better place
    void JSObject::AddFuncProperty(const std::u16string& name, inner_func callable, bool writable, bool enumerable, bool configurable)
    {
        JSObject* value = new JSObject(OBJ_INNER_FUNC, u"InternalFunc", false, nullptr, false, true, callable);
        value->SetPrototype(FunctionProto::Instance());
        AddValueProperty(name, value, writable, enumerable, configurable);
    }

    double ToNumber(Error* e, JSValue* input);
    JSObject* ToObject(Error* e, JSValue* input);

    class NumberProto : public JSObject
    {
    public:
        static NumberProto* Instance()
        {
            static NumberProto singleton;
            return &singleton;
        }

        static JSValue* toString(Error* e, JSValue* this_arg, const std::vector<JSValue*>& vals)
        {
            assert(false);
        }

        static JSValue* toLocaleString(Error* e, JSValue* this_arg, const std::vector<JSValue*>& vals)
        {
            assert(false);
        }

        static JSValue* valueOf(Error* e, JSValue* this_arg, const std::vector<JSValue*>& vals)
        {
            JSValue* val = RuntimeContext::TopValue();
            if(val->IsObject())
            {
                JSObject* obj = static_cast<JSObject*>(val);
                if(obj->obj_type() == JSObject::OBJ_NUMBER)
                {
                    return obj->PrimitiveValue();
                }
            }
            else if(val->IsNumber())
            {
                return val;
            }
            *e = *Error::TypeError(u"Number.prototype.valueOf called with non-number");
            return nullptr;
        }

        static JSValue* toFixed(Error* e, JSValue* this_arg, const std::vector<JSValue*>& vals)
        {
            assert(false);
        }

        static JSValue* toExponential(Error* e, JSValue* this_arg, const std::vector<JSValue*>& vals)
        {
            assert(false);
        }

        static JSValue* toPrecision(Error* e, JSValue* this_arg, const std::vector<JSValue*>& vals)
        {
            assert(false);
        }

    private:
        NumberProto() : JSObject(OBJ_NUMBER, u"Number", true, Number::Zero(), false, false)
        {
        }
    };

    class NumberObject : public JSObject
    {
    public:
        NumberObject(JSValue* primitive_value)
        : JSObject(OBJ_NUMBER,
                   u"Number",
                   true,// extensible
                   primitive_value,
                   false,
                   false)
        {
            SetPrototype(NumberProto::Instance());
        }
    };

    class NumberConstructor : public JSObject
    {
    public:
        static NumberConstructor* Instance()
        {
            static NumberConstructor singleton;
            return &singleton;
        }

        // 15.7.1.1 Number ( [ value ] )
        JSValue* Call(Error* e, JSValue* this_arg, const std::vector<JSValue*>& arguments = {}) override
        {
            Number* js_num;
            if(arguments.size() == 0)
            {
                js_num = Number::Zero();
            }
            else
            {
                double num = ToNumber(e, arguments[0]);
                if(!e->IsOk())
                    return nullptr;
                js_num = new Number(num);
            }
            return js_num;
        }

        JSObject* Construct(Error* e, const std::vector<JSValue*>& arguments) override
        {
            Number* js_num;
            if(arguments.size() == 0)
            {
                js_num = Number::Zero();
            }
            else
            {
                double num = ToNumber(e, arguments[0]);
                if(!e->IsOk())
                    return nullptr;
                js_num = new Number(num);
            }
            return new NumberObject(js_num);
        }

        static JSValue* toString(Error* e, JSValue* this_arg, const std::vector<JSValue*>& vals)
        {
            return new String(u"function Number() { [native code] }");
        }

    private:
        NumberConstructor() : JSObject(OBJ_OTHER, u"Number", true, nullptr, true, true)
        {
        }
    };

    bool ToBoolean(JSValue*);

    class BoolProto : public JSObject
    {
    public:
        static BoolProto* Instance()
        {
            static BoolProto singleton;
            return &singleton;
        }

        JSValue* Call(Error* e, JSValue* this_arg, const std::vector<JSValue*>& arguments = {}) override
        {
            return Undefined::Instance();
        }

        static JSValue* toString(Error* e, JSValue* this_arg, const std::vector<JSValue*>& vals)
        {
            return ToBoolean(this_arg) ? String::True() : String::False();
        }

        static JSValue* valueOf(Error* e, JSValue* this_arg, const std::vector<JSValue*>& vals)
        {
            assert(false);
        }

    private:
        BoolProto() : JSObject(OBJ_BOOL, u"Boolean", true, Bool::False(), false, true)
        {
        }
    };

    class BoolObject : public JSObject
    {
    public:
        BoolObject(JSValue* primitive_value) : JSObject(OBJ_BOOL, u"Boolean", true, primitive_value, false, false)
        {
            SetPrototype(BoolProto::Instance());
        }
    };

    class BoolConstructor : public JSObject
    {
    public:
        static BoolConstructor* Instance()
        {
            static BoolConstructor singleton;
            return &singleton;
        }

        JSValue* Call(Error* e, JSValue* this_arg, const std::vector<JSValue*>& arguments = {}) override
        {
            bool b;
            if(arguments.size() == 0)
                b = ToBoolean(Undefined::Instance());
            else
                b = ToBoolean(arguments[0]);
            return Bool::Wrap(b);
        }

        JSObject* Construct(Error* e, const std::vector<JSValue*>& arguments) override
        {
            bool b;
            if(arguments.size() == 0)
                b = ToBoolean(Undefined::Instance());
            else
                b = ToBoolean(arguments[0]);
            return new BoolObject(Bool::Wrap(b));
        }

        static JSValue* toString(Error* e, JSValue* this_arg, const std::vector<JSValue*>& vals)
        {
            return new String(u"function Bool() { [native code] }");
        }

    private:
        BoolConstructor() : JSObject(OBJ_OTHER, u"Boolean", true, nullptr, true, true)
        {
        }
    };

    std::u16string ToString(Error* e, JSValue* input);
    double ToInteger(Error* e, JSValue* input);
    double ToUint16(Error* e, JSValue* input);
    std::u16string NumberToString(double m);

    class StringProto : public JSObject
    {
    public:
        static StringProto* Instance()
        {
            static StringProto singleton;
            return &singleton;
        }

        static JSValue* toString(Error* e, JSValue* this_arg, const std::vector<JSValue*>& vals)
        {
            JSValue* val = RuntimeContext::TopValue();
            if(!val->IsObject())
            {
                *e = *Error::TypeError(u"String.prototype.toString called with non-object");
                return nullptr;
            }
            JSObject* obj = static_cast<JSObject*>(val);
            if(obj->obj_type() != JSObject::OBJ_STRING)
            {
                *e = *Error::TypeError(u"String.prototype.toString called with non-string");
                return nullptr;
            }
            return obj->PrimitiveValue();
        }

        static JSValue* valueOf(Error* e, JSValue* this_arg, const std::vector<JSValue*>& vals)
        {
            JSValue* val = RuntimeContext::TopValue();
            if(!val->IsObject())
            {
                *e = *Error::TypeError(u"String.prototype.valueOf called with non-object");
                return nullptr;
            }
            JSObject* obj = static_cast<JSObject*>(val);
            if(obj->obj_type() != JSObject::OBJ_STRING)
            {
                *e = *Error::TypeError(u"String.prototype.valueOf called with non-string");
                return nullptr;
            }
            return obj->PrimitiveValue();
        }

        static JSValue* charAt(Error* e, JSValue* this_arg, const std::vector<JSValue*>& vals)
        {
            if(vals.size() == 0)
                return String::Empty();
            JSValue* val = RuntimeContext::TopValue();
            val->CheckObjectCoercible(e);
            if(!e->IsOk())
                return nullptr;
            std::u16string S = ::es::ToString(e, val);
            if(!e->IsOk())
                return nullptr;
            int position = ToInteger(e, vals[0]);
            if(!e->IsOk())
                return nullptr;
            if(position < 0 || position >= S.size())
                return String::Empty();
            return new String(S.substr(position, 1));
        }

        static JSValue* charCodeAt(Error* e, JSValue* this_arg, const std::vector<JSValue*>& vals)
        {
            if(vals.size() == 0)
                return Number::NaN();
            JSValue* val = RuntimeContext::TopValue();
            val->CheckObjectCoercible(e);
            if(!e->IsOk())
                return nullptr;
            std::u16string S = ::es::ToString(e, val);
            if(!e->IsOk())
                return nullptr;
            int position = ToInteger(e, vals[0]);
            if(!e->IsOk())
                return nullptr;
            if(position < 0 || position >= S.size())
                return Number::NaN();
            return new Number((double)S[position]);
        }

        static JSValue* concat(Error* e, JSValue* this_arg, const std::vector<JSValue*>& vals)
        {
            JSValue* val = RuntimeContext::TopValue();
            val->CheckObjectCoercible(e);
            if(!e->IsOk())
                return nullptr;
            std::u16string S = ::es::ToString(e, val);
            if(!e->IsOk())
                return nullptr;
            std::u16string R = S;
            std::vector<JSValue*> args = vals;
            for(auto arg : args)
            {
                std::u16string next = ::es::ToString(e, arg);
                if(!e->IsOk())
                    return nullptr;
                R += next;
            }
            return new String(R);
        }

        static JSValue* indexOf(Error* e, JSValue* this_arg, const std::vector<JSValue*>& vals)
        {
            JSValue* val = RuntimeContext::TopValue();
            val->CheckObjectCoercible(e);
            if(!e->IsOk())
                return nullptr;
            std::u16string S = ::es::ToString(e, val);
            if(!e->IsOk())
                return nullptr;
            JSValue* search_string;
            if(vals.size() == 0)
                search_string = Undefined::Instance();
            else
                search_string = vals[0];
            std::u16string search_str = ::es::ToString(e, search_string);
            if(!e->IsOk())
                return nullptr;
            double pos;
            if(vals.size() < 2 || vals[1]->IsUndefined())
                pos = 0;
            else
            {
                pos = ToInteger(e, vals[1]);
                if(!e->IsOk())
                    return nullptr;
            }
            int start = fmin(fmax(pos, 0), S.size());
            size_t find_pos = S.find(search_str, start);
            if(find_pos != std::u16string::npos)
            {
                return new Number(find_pos);
            }
            return new Number(-1);
        }

        static JSValue* lastIndexOf(Error* e, JSValue* this_arg, const std::vector<JSValue*>& vals)
        {
            JSValue* val = RuntimeContext::TopValue();
            val->CheckObjectCoercible(e);
            if(!e->IsOk())
                return nullptr;
            std::u16string S = ::es::ToString(e, val);
            if(!e->IsOk())
                return nullptr;
            JSValue* search_string;
            if(vals.size() == 0)
                search_string = Undefined::Instance();
            else
                search_string = vals[0];
            std::u16string search_str = ::es::ToString(e, search_string);
            if(!e->IsOk())
                return nullptr;
            double pos;
            if(vals.size() < 2 || vals[1]->IsUndefined())
                pos = nan("");
            else
            {
                pos = ToNumber(e, vals[1]);
                if(!e->IsOk())
                    return nullptr;
            }
            int start;
            if(isnan(pos))
                start = S.size();
            else
                start = fmin(fmax(pos, 0), S.size());
            size_t find_pos = S.rfind(search_str, start);
            if(find_pos != std::u16string::npos)
            {
                return new Number(find_pos);
            }
            return new Number(-1);
        }

        static JSValue* localeCompare(Error* e, JSValue* this_arg, const std::vector<JSValue*>& vals)
        {
            assert(false);
        }

        static JSValue* match(Error* e, JSValue* this_arg, const std::vector<JSValue*>& vals)
        {
            assert(false);
        }

        static JSValue* replace(Error* e, JSValue* this_arg, const std::vector<JSValue*>& vals)
        {
            assert(false);
        }

        static JSValue* search(Error* e, JSValue* this_arg, const std::vector<JSValue*>& vals)
        {
            assert(false);
        }

        static JSValue* slice(Error* e, JSValue* this_arg, const std::vector<JSValue*>& vals)
        {
            assert(false);
        }

        static JSValue* split(Error* e, JSValue* this_arg, const std::vector<JSValue*>& vals)
        {
            assert(false);
        }

        static JSValue* substring(Error* e, JSValue* this_arg, const std::vector<JSValue*>& vals)
        {
            if(vals.size() == 0)
                return Number::NaN();
            JSValue* val = RuntimeContext::TopValue();
            val->CheckObjectCoercible(e);
            if(!e->IsOk())
                return nullptr;
            std::u16string S = ::es::ToString(e, val);
            int len = S.size();
            if(!e->IsOk())
                return nullptr;
            int int_start = ToInteger(e, vals[0]);
            if(!e->IsOk())
                return nullptr;
            int int_end;
            if(vals.size() < 2 || vals[0]->IsUndefined())
            {
                int_end = S.size();
            }
            else
            {
                int_end = ToInteger(e, vals[1]);
                if(!e->IsOk())
                    return nullptr;
            }
            int final_start = fmin(fmax(int_start, 0), len);
            int final_end = fmin(fmax(int_end, 0), len);
            int from = fmin(final_start, final_end);
            int to = fmax(final_start, final_end);
            return new String(S.substr(from, to - from));
        }

        static JSValue* toLowerCase(Error* e, JSValue* this_arg, const std::vector<JSValue*>& vals)
        {
            assert(false);
        }

        static JSValue* toLocaleLowerCase(Error* e, JSValue* this_arg, const std::vector<JSValue*>& vals)
        {
            assert(false);
        }

        static JSValue* toUpperCase(Error* e, JSValue* this_arg, const std::vector<JSValue*>& vals)
        {
            assert(false);
        }

        static JSValue* toLocaleUpperCase(Error* e, JSValue* this_arg, const std::vector<JSValue*>& vals)
        {
            assert(false);
        }

        static JSValue* trim(Error* e, JSValue* this_arg, const std::vector<JSValue*>& vals)
        {
            assert(false);
        }

    private:
        StringProto() : JSObject(OBJ_OTHER, u"String", true, String::Empty(), false, false)
        {
        }
    };

    class StringObject : public JSObject
    {
    public:
        StringObject(JSValue* primitive_value)
        : JSObject(OBJ_STRING,
                   u"String",
                   true,// extensible
                   primitive_value,
                   false,
                   false)
        {
            SetPrototype(StringProto::Instance());
            assert(primitive_value->IsString());
            double length = static_cast<String*>(primitive_value)->data().size();
            AddValueProperty(u"length", new Number(length), false, false, false);
        }

        JSValue* GetOwnProperty(const std::u16string& P) override
        {
            JSValue* val = JSObject::GetOwnProperty(P);
            if(!val->IsUndefined())
                return val;
            Error* e = Error::Ok();
            int index = ToInteger(e, new String(P));// this will never has error.
            if(NumberToString(fabs(index)) != P)
                return Undefined::Instance();
            std::u16string str = static_cast<String*>(PrimitiveValue())->data();
            int len = str.size();
            if(len <= index)
                return Undefined::Instance();
            PropertyDescriptor* desc = new PropertyDescriptor();
            desc->SetDataDescriptor(new String(str.substr(index, 1)), true, false, false);
            return desc;
        }
    };

    class StringConstructor : public JSObject
    {
    public:
        static StringConstructor* Instance()
        {
            static StringConstructor singleton;
            return &singleton;
        }

        // 15.5.1.1 String ( [ value ] )
        JSValue* Call(Error* e, JSValue* this_arg, const std::vector<JSValue*>& arguments = {}) override
        {
            if(arguments.size() == 0)
                return String::Empty();
            return new String(::es::ToString(e, arguments[0]));
        }

        // 15.5.2.1 new String ( [ value ] )
        JSObject* Construct(Error* e, const std::vector<JSValue*>& arguments) override
        {
            if(arguments.size() == 0)
                return new StringObject(String::Empty());
            std::u16string str = ::es::ToString(e, arguments[0]);
            if(!e->IsOk())
                return nullptr;
            return new StringObject(new String(str));
        }

        static JSValue* fromCharCode(Error* e, JSValue* this_arg, const std::vector<JSValue*>& vals)
        {
            std::u16string result = u"";
            for(JSValue* val : vals)
            {
                char16_t c = ToUint16(e, val);
                if(!e->IsOk())
                    return nullptr;
                result += c;
            }
            return new String(result);
        }

        static JSValue* toString(Error* e, JSValue* this_arg, const std::vector<JSValue*>& vals)
        {
            return new String(u"function String() { [native code] }");
        }

    private:
        StringConstructor() : JSObject(OBJ_OTHER, u"String", true, nullptr, true, true)
        {
        }
    };

    bool ToBoolean(JSValue* input);
    double ToNumber(Error* e, JSValue* input);
    double ToUint32(Error* e, JSValue* input);
    std::u16string ToString(Error* e, JSValue* input);
    std::u16string NumberToString(double m);
    double StringToNumber(const std::u16string& source);
    JSObject* ToObject(Error* e, JSValue* input);

    bool IsArrayIndex(const std::u16string& P)
    {
        return P == NumberToString(uint32_t(StringToNumber(P)));
    }

    class ArrayProto : public JSObject
    {
    public:
        static ArrayProto* Instance()
        {
            static ArrayProto singleton;
            return &singleton;
        }

        // 15.4.4.2 Array.prototype.toString ( )
        static JSValue* toString(Error* e, JSValue* this_arg, const std::vector<JSValue*>& vals)
        {
            JSObject* array = ToObject(e, RuntimeContext::TopValue());
            JSValue* func = array->Get(e, u"join");
            if(!e->IsOk())
                return nullptr;
            if(!func->IsCallable())
            {
                func = ObjectProto::Instance()->Get(e, u"toString");
                if(!e->IsOk())
                    return nullptr;
            }
            return static_cast<JSObject*>(func)->Call(e, this_arg, vals);
        }

        static JSValue* toLocaleString(Error* e, JSValue* this_arg, const std::vector<JSValue*>& vals)
        {
            assert(false);
        }

        static JSValue* concat(Error* e, JSValue* this_arg, const std::vector<JSValue*>& vals)
        {
            assert(false);
        }

        static JSValue* join(Error* e, JSValue* this_arg, const std::vector<JSValue*>& vals)
        {
            JSObject* O = ToObject(e, RuntimeContext::TopValue());
            Number* len_val = static_cast<Number*>(O->Get(e, u"length"));
            size_t len = len_val->data();

            std::u16string sep = u",";
            if(vals.size() > 0 && !vals[0]->IsUndefined())
            {
                sep = ::es::ToString(e, vals[0]);
                if(!e->IsOk())
                    return nullptr;
            }
            if(len == 0)
                return String::Empty();
            JSValue* element0 = O->Get(e, u"0");
            if(!e->IsOk())
                return nullptr;
            std::u16string R = u"";
            if(!element0->IsUndefined() && !element0->IsNull())
            {
                R = ::es::ToString(e, element0);
            }
            for(double k = 1; k < len; k++)
            {
                JSValue* element = O->Get(e, NumberToString(k));
                if(!e->IsOk())
                    return nullptr;
                std::u16string next = u"";
                if(!element->IsUndefined() && !element->IsNull())
                {
                    next = ::es::ToString(e, element);
                }
                R += sep + next;
            }
            return new String(R);
        }

        // 15.4.4.6 Array.prototype.pop ( )
        static JSValue* pop(Error* e, JSValue* this_arg, const std::vector<JSValue*>& vals)
        {
            JSObject* O = ToObject(e, RuntimeContext::TopValue());
            if(!e->IsOk())
                return nullptr;
            size_t len = ToNumber(e, O->Get(e, u"length"));
            if(!e->IsOk())
                return nullptr;
            if(len == 0)
            {
                O->Put(e, u"length", Number::Zero(), true);
                if(!e->IsOk())
                    return nullptr;
                return Undefined::Instance();
            }
            else
            {
                assert(len > 0);
                std::u16string indx = NumberToString(len - 1);
                JSValue* element = O->Get(e, indx);
                if(!e->IsOk())
                    return nullptr;
                O->Delete(e, indx, true);
                if(!e->IsOk())
                    return nullptr;
                O->Put(e, u"length", new Number(len - 1), true);
                if(!e->IsOk())
                    return nullptr;
                return element;
            }
        }

        // 15.4.4.7 Array.prototype.push ( [ item1 [ , item2 [ , … ] ] ] )
        static JSValue* push(Error* e, JSValue* this_arg, const std::vector<JSValue*>& vals)
        {
            JSObject* O = ToObject(e, RuntimeContext::TopValue());
            if(!e->IsOk())
                return nullptr;
            double n = ToNumber(e, O->Get(e, u"length"));
            if(!e->IsOk())
                return nullptr;
            for(JSValue* E : vals)
            {
                O->Put(e, NumberToString(n), E, true);
                if(!e->IsOk())
                    return nullptr;
                n++;
            }
            Number* num = new Number(n);
            O->Put(e, u"length", num, true);
            if(!e->IsOk())
                return nullptr;
            return num;
        }

        static JSValue* reverse(Error* e, JSValue* this_arg, const std::vector<JSValue*>& vals)
        {
            assert(false);
        }

        static JSValue* shift(Error* e, JSValue* this_arg, const std::vector<JSValue*>& vals)
        {
            assert(false);
        }

        static JSValue* slice(Error* e, JSValue* this_arg, const std::vector<JSValue*>& vals)
        {
            assert(false);
        }

        static JSValue* sort(Error* e, JSValue* this_arg, const std::vector<JSValue*>& vals)
        {
            assert(false);
        }

        static JSValue* splice(Error* e, JSValue* this_arg, const std::vector<JSValue*>& vals)
        {
            assert(false);
        }

        static JSValue* unshift(Error* e, JSValue* this_arg, const std::vector<JSValue*>& vals)
        {
            assert(false);
        }

        static JSValue* indexOf(Error* e, JSValue* this_arg, const std::vector<JSValue*>& vals)
        {
            assert(false);
        }

        static JSValue* lastIndexOf(Error* e, JSValue* this_arg, const std::vector<JSValue*>& vals)
        {
            assert(false);
        }

        static JSValue* every(Error* e, JSValue* this_arg, const std::vector<JSValue*>& vals)
        {
            assert(false);
        }

        static JSValue* some(Error* e, JSValue* this_arg, const std::vector<JSValue*>& vals)
        {
            assert(false);
        }

        static JSValue* forEach(Error* e, JSValue* this_arg, const std::vector<JSValue*>& vals);

        static JSValue* toLocaleUpperCase(Error* e, JSValue* this_arg, const std::vector<JSValue*>& vals)
        {
            assert(false);
        }

        static JSValue* map(Error* e, JSValue* this_arg, const std::vector<JSValue*>& vals);

        static JSValue* filter(Error* e, JSValue* this_arg, const std::vector<JSValue*>& vals);

        static JSValue* reduce(Error* e, JSValue* this_arg, const std::vector<JSValue*>& vals)
        {
            assert(false);
        }

        static JSValue* reduceRight(Error* e, JSValue* this_arg, const std::vector<JSValue*>& vals)
        {
            assert(false);
        }

    private:
        ArrayProto() : JSObject(OBJ_ARRAY, u"Array", true, nullptr, false, false)
        {
        }
    };

    class ArrayObject : public JSObject
    {
    public:
        ArrayObject(double len) : JSObject(OBJ_ARRAY, u"Array", true, nullptr, false, false)
        {
            SetPrototype(ArrayProto::Instance());
            // Not using AddValueProperty here to by pass the override DefineOwnProperty
            PropertyDescriptor* desc = new PropertyDescriptor();
            desc->SetDataDescriptor(new Number(len), true, false, false);
            JSObject::DefineOwnProperty(nullptr, u"length", desc, false);
        }

        bool DefineOwnProperty(Error* e, const std::u16string& P, PropertyDescriptor* desc, bool throw_flag) override
        {
            auto old_len_desc = static_cast<PropertyDescriptor*>(GetOwnProperty(u"length"));
            assert(!old_len_desc->IsUndefined());
            double old_len = ToNumber(e, old_len_desc->Value());
            if(P == u"length")
            {// 3
                if(!desc->HasValue())
                {// 3.a
                    return JSObject::DefineOwnProperty(e, u"length", desc, throw_flag);
                }
                PropertyDescriptor* new_len_desc = new PropertyDescriptor();
                new_len_desc->Set(desc);
                double new_len = ToUint32(e, desc->Value());
                if(!e->IsOk())
                    goto reject;
                double new_num = ToNumber(e, desc->Value());
                if(!e->IsOk())
                    goto reject;
                if(new_len != new_num)
                {
                    *e = *Error::RangeError(u"length of array need to be uint32.");
                    return false;
                }
                new_len_desc->SetValue(new Number(new_len));
                if(new_len >= old_len)
                {// 3.f
                    return JSObject::DefineOwnProperty(e, u"length", new_len_desc, throw_flag);
                }
                if(!old_len_desc->Writable())// 3.g
                    goto reject;
                bool new_writable;
                if(new_len_desc->HasWritable() && new_len_desc->Writable())
                {// 3.h
                    new_writable = true;
                }
                else
                {// 3.l
                    new_writable = false;
                    new_len_desc->SetWritable(true);
                }
                bool succeeded = JSObject::DefineOwnProperty(e, u"length", new_len_desc, throw_flag);
                if(!succeeded)
                    return false;// 3.k
                while(new_len < old_len)
                {// 3.l
                    old_len--;
                    bool delete_succeeded = Delete(e, ::es::ToString(e, new Number(old_len)), false);
                    if(!delete_succeeded)
                    {// 3.l.iii
                        new_len_desc->SetValue(new Number(old_len + 1));
                        if(!new_writable)// 3.l.iii.2
                            new_len_desc->SetWritable(false);
                        JSObject::DefineOwnProperty(e, u"length", new_len_desc, false);
                        goto reject;// 3.l.iii.4
                    }
                }
                if(!new_writable)
                {// 3.m
                    auto tmp = new PropertyDescriptor();
                    tmp->SetWritable(false);
                    assert(JSObject::DefineOwnProperty(e, u"length", new_len_desc, false));
                    return true;
                }
                return true;// 3.n
            }
            else
            {
                if(IsArrayIndex(P))
                {// 4
                    double index = StringToNumber(P);
                    if(index >= old_len && !old_len_desc->Writable())// 4.b
                        goto reject;
                    bool succeeded = JSObject::DefineOwnProperty(e, P, desc, false);
                    if(!succeeded)
                        goto reject;
                    if(index >= old_len)
                    {// 4.e
                        old_len_desc->SetValue(new Number(index + 1));
                        return JSObject::DefineOwnProperty(e, u"length", old_len_desc, false);
                    }
                    return true;
                }
            }
            return JSObject::DefineOwnProperty(e, P, desc, throw_flag);
        reject:
            log::PrintSource("Array::DefineOwnProperty reject ", P, " " + desc->ToString());
            if(throw_flag)
            {
                *e = *Error::TypeError();
            }
            return false;
        }

        std::string ToString() override
        {
            size_t num = ToNumber(nullptr, Get(nullptr, u"length"));
            return "Array(" + std::to_string(num) + ")";
        }
    };

    class ArrayConstructor : public JSObject
    {
    public:
        static ArrayConstructor* Instance()
        {
            static ArrayConstructor singleton;
            return &singleton;
        }

        // 15.5.1.1 Array ( [ value ] )
        JSValue* Call(Error* e, JSValue* this_arg, const std::vector<JSValue*>& arguments = {}) override
        {
            return Construct(e, arguments);
        }

        // 15.5.2.1 new Array ( [ value ] )
        JSObject* Construct(Error* e, const std::vector<JSValue*>& arguments) override
        {
            if(arguments.size() == 1 && arguments[0]->IsNumber())
            {
                Number* len = static_cast<Number*>(arguments[0]);
                if(len->data() == ToUint32(e, len))
                {
                    return new ArrayObject(len->data());
                }
                else
                {
                    *e = *Error::RangeError(u"Invalid array length");
                    return nullptr;
                }
            }
            ArrayObject* arr = new ArrayObject(arguments.size());
            for(size_t i = 0; i < arguments.size(); i++)
            {
                JSValue* arg = arguments[i];
                arr->AddValueProperty(::es::NumberToString(i), arg, true, true, true);
            }
            return arr;
        }

        static JSValue* isArray(Error* e, JSValue* this_arg, const std::vector<JSValue*>& vals)
        {
            if(vals.size() == 0 || !vals[0]->IsObject())
                return Bool::False();
            JSObject* obj = static_cast<JSObject*>(vals[0]);
            return Bool::Wrap(obj->Class() == u"Array");
        }

        static JSValue* toString(Error* e, JSValue* this_arg, const std::vector<JSValue*>& vals)
        {
            return new String(u"function Array() { [native code] }");
        }

    private:
        ArrayConstructor() : JSObject(OBJ_OTHER, u"Array", true, nullptr, true, true)
        {
        }
    };

    // 15.4.4.18 Array.prototype.forEach ( callbackfn [ , thisArg ] )
    JSValue* ArrayProto::forEach(Error* e, JSValue* this_arg, const std::vector<JSValue*>& vals)
    {
        JSObject* O = ToObject(e, RuntimeContext::TopValue());
        if(!e->IsOk())
            return nullptr;
        size_t len = ToNumber(e, O->Get(e, u"length"));
        if(vals.size() == 0 || !vals[0]->IsCallable())
        {// 4
            *e = *Error::TypeError(u"Array.prototype.forEach called on non-callable");
            return nullptr;
        }
        JSObject* callbackfn = static_cast<JSObject*>(vals[0]);
        JSValue* T;
        if(vals.size() < 2)
        {
            T = Undefined::Instance();
        }
        else
        {
            T = vals[1];
        }
        ArrayObject* A = new ArrayObject(len);
        for(size_t k = 0; k < len; k++)
        {
            std::u16string p_k = NumberToString(k);
            bool k_present = O->HasProperty(p_k);
            if(!e->IsOk())
                return nullptr;
            if(k_present)
            {
                JSValue* k_value = O->Get(e, p_k);
                if(!e->IsOk())
                    return nullptr;
                JSValue* mapped_value = callbackfn->Call(e, T, { k_value, new Number(k), O });
                if(!e->IsOk())
                    return nullptr;
            }
        }
        return Undefined::Instance();
    }

    // 15.4.4.19 Array.prototype.map ( callbackfn [ , thisArg ] )
    JSValue* ArrayProto::map(Error* e, JSValue* this_arg, const std::vector<JSValue*>& vals)
    {
        JSObject* O = ToObject(e, RuntimeContext::TopValue());
        if(!e->IsOk())
            return nullptr;
        size_t len = ToNumber(e, O->Get(e, u"length"));
        if(vals.size() == 0 || !vals[0]->IsCallable())
        {// 4
            *e = *Error::TypeError(u"Array.prototype.map called on non-callable");
            return nullptr;
        }
        JSObject* callbackfn = static_cast<JSObject*>(vals[0]);
        JSValue* T;
        if(vals.size() < 2)
        {
            T = Undefined::Instance();
        }
        else
        {
            T = vals[1];
        }
        ArrayObject* A = new ArrayObject(len);
        for(size_t k = 0; k < len; k++)
        {
            std::u16string p_k = NumberToString(k);
            bool k_present = O->HasProperty(p_k);
            if(!e->IsOk())
                return nullptr;
            if(k_present)
            {
                JSValue* k_value = O->Get(e, p_k);
                if(!e->IsOk())
                    return nullptr;
                JSValue* mapped_value = callbackfn->Call(e, T, { k_value, new Number(k), O });
                if(!e->IsOk())
                    return nullptr;
                A->AddValueProperty(p_k, mapped_value, true, true, true);
            }
        }
        return A;
    }

    // 15.4.4.20 Array.prototype.filter ( callbackfn [ , thisArg ] )
    JSValue* ArrayProto::filter(Error* e, JSValue* this_arg, const std::vector<JSValue*>& vals)
    {
        JSObject* O = ToObject(e, RuntimeContext::TopValue());
        if(!e->IsOk())
            return nullptr;
        size_t len = ToNumber(e, O->Get(e, u"length"));
        if(vals.size() == 0 || !vals[0]->IsCallable())
        {// 4
            *e = *Error::TypeError(u"Array.prototype.filter called on non-callable");
            return nullptr;
        }
        JSObject* callbackfn = static_cast<JSObject*>(vals[0]);
        JSValue* T;
        if(vals.size() < 2)
        {
            T = Undefined::Instance();
        }
        else
        {
            T = vals[1];
        }
        size_t to = 0;
        ArrayObject* A = new ArrayObject(len);
        for(size_t k = 0; k < len; k++)
        {
            std::u16string p_k = NumberToString(k);
            bool k_present = O->HasProperty(p_k);
            if(!e->IsOk())
                return nullptr;
            if(k_present)
            {
                JSValue* k_value = O->Get(e, p_k);
                if(!e->IsOk())
                    return nullptr;
                JSValue* selected = callbackfn->Call(e, T, { k_value, new Number(k), O });
                if(!e->IsOk())
                    return nullptr;
                if(ToBoolean(selected))
                {
                    A->AddValueProperty(NumberToString(to), k_value, true, true, true);
                    to++;
                }
            }
        }
        return A;
    }

    JSValue* ObjectConstructor::keys(Error* e, JSValue* this_arg, const std::vector<JSValue*>& vals)
    {
        if(vals.size() < 1 || !vals[0]->IsObject())
        {
            *e = *Error::TypeError(u"Object.keys called on non-object");
            return nullptr;
        }
        JSObject* O = static_cast<JSObject*>(vals[0]);
        auto properties = O->AllEnumerableProperties();
        size_t n = properties.size();
        ArrayObject* arr_obj = new ArrayObject(n);
        for(size_t index = 0; index < n; index++)
        {
            arr_obj->AddValueProperty(NumberToString(index), new String(properties[index].first), true, true, true);
        }
        return arr_obj;
    }

    // 10.6 Arguments Object
    class ArgumentsObject : public JSObject
    {
    public:
        ArgumentsObject(JSObject* parameter_map, size_t len)
        : JSObject(OBJ_OBJECT, u"Arguments", true, nullptr, false, false), parameter_map_(parameter_map)
        {
            SetPrototype(ObjectProto::Instance());
            AddValueProperty(u"length", new Number(len), true, false, true);
        }

        JSObject* ParameterMap()
        {
            return parameter_map_;
        }

        JSValue* Get(Error* e, const std::u16string& P) override
        {
            JSObject* map = ParameterMap();
            JSValue* is_mapped = map->GetOwnProperty(P);
            if(is_mapped->IsUndefined())
            {// 3
                JSValue* v = JSObject::Get(e, P);
                if(!e->IsOk())
                    return nullptr;
                if(P == u"caller")
                {
                    if(v->IsObject())
                    {
                        JSObject* obj = static_cast<JSObject*>(v);
                        if(obj->IsFunction())
                        {
                            FunctionObject* func = static_cast<FunctionObject*>(obj);
                            if(func->strict())
                            {
                                *e = *Error::TypeError(u"caller could not be function object");
                            }
                        }
                    }
                }
                return v;
            }
            // 4
            return map->Get(e, P);
        }

        JSValue* GetOwnProperty(const std::u16string& P) override
        {
            JSValue* val = JSObject::GetOwnProperty(P);
            if(val->IsUndefined())
                return val;
            PropertyDescriptor* desc = static_cast<PropertyDescriptor*>(val);
            JSObject* map = ParameterMap();
            JSValue* is_mapped = map->GetOwnProperty(P);
            if(!is_mapped->IsUndefined())
            {// 5
                desc->SetValue(map->Get(nullptr, P));
            }
            return desc;
        }

        bool DefineOwnProperty(Error* e, const std::u16string& P, PropertyDescriptor* desc, bool throw_flag) override
        {
            JSObject* map = ParameterMap();
            JSValue* is_mapped = map->GetOwnProperty(P);
            bool allowed = JSObject::DefineOwnProperty(e, P, desc, false);
            if(!allowed)
            {
                if(throw_flag)
                {
                    *e = *Error::TypeError(u"DefineOwnProperty " + P + u" failed");
                }
                return false;
            }
            if(!is_mapped->IsUndefined())
            {// 5
                if(desc->IsAccessorDescriptor())
                {
                    map->Delete(e, P, false);
                }
                else
                {
                    if(desc->HasValue())
                    {
                        map->Put(e, P, desc->Value(), false);
                    }
                    if(desc->HasWritable() && !desc->Writable())
                    {
                        map->Delete(e, P, false);
                    }
                }
            }
            return true;
        }

        bool Delete(Error* e, const std::u16string& P, bool throw_flag) override
        {
            JSObject* map = ParameterMap();
            JSValue* is_mapped = map->GetOwnProperty(P);
            bool result = JSObject::Delete(e, P, throw_flag);
            if(!e->IsOk())
                return false;
            if(result && !is_mapped->IsUndefined())
            {
                map->Delete(e, P, false);
            }
            return result;
        }

        inline std::string ToString() override
        {
            return "ArgumentsObject";
        }

    private:
        JSObject* parameter_map_;
    };

    bool ToBoolean(JSValue*);

    class Console : public JSObject
    {
    public:
        static Console* Instance()
        {
            static Console singleton;
            return &singleton;
        }

        static JSValue* log(Error* e, JSValue* this_arg, const std::vector<JSValue*>& vals)
        {
            for(auto val : vals)
            {
                std::cout << val->ToString() << " ";
            }
            std::cout << std::endl;
            return Number::Zero();
        }

    private:
        Console() : JSObject(OBJ_BOOL, u"Console", true, Bool::False(), false, false)
        {
            AddFuncProperty(u"log", log, false, false, false);
        }
    };

    enum CodeType
    {
        CODE_GLOBAL = 0,
        CODE_FUNC,
        CODE_EVAL,
    };

    JSValue* MakeArgGetter(const std::u16string& name, LexicalEnvironment* env)
    {
        Parser parser(u"return " + name + u";");
        ProgramOrFunctionBody* body = static_cast<ProgramOrFunctionBody*>(parser.ParseFunctionBody(Token::TK_EOS));
        return new FunctionObject({}, body, env);
    }

    JSValue* MakeArgSetter(const std::u16string& name, LexicalEnvironment* env)
    {
        std::u16string param = name + u"_arg";
        Parser parser(name + u" = " + param);
        ProgramOrFunctionBody* body = static_cast<ProgramOrFunctionBody*>(parser.ParseFunctionBody(Token::TK_EOS));
        return new FunctionObject({ param }, body, env);
    }

    // 10.6 Arguments Object
    JSObject* CreateArgumentsObject(FunctionObject* func, std::vector<JSValue*>& args, LexicalEnvironment* env, bool strict)
    {
        std::vector<std::u16string> names = func->FormalParameters();
        int len = args.size();
        Object* map = new Object();// 8
        JSObject* obj = new ArgumentsObject(map, len);
        int indx = len - 1;// 10
        std::set<std::u16string> mapped_names;
        while(indx >= 0)
        {// 11
            JSValue* val = args[indx];// 11.a
            obj->AddValueProperty(NumberToString(indx), val, true, true, true);// 11.b
            if(indx < names.size())
            {// 11.c
                std::u16string name = names[indx];// 11.c.i
                if(!strict && mapped_names.find(name) == mapped_names.end())
                {// 11.c.ii
                    mapped_names.insert(name);
                    JSValue* g = MakeArgGetter(name, env);
                    JSValue* p = MakeArgSetter(name, env);
                    PropertyDescriptor* desc = new PropertyDescriptor();
                    desc->SetSet(p);
                    desc->SetGet(g);
                    desc->SetConfigurable(true);
                    map->DefineOwnProperty(nullptr, NumberToString(indx), desc, false);
                }
            }
            indx--;// 11.d
        }
        if(!strict)
        {// 13
            obj->AddValueProperty(u"callee", func, true, false, true);
        }
        else
        {// 14
            // TODO(zhuzilin) thrower
        }
        return obj;// 15
    }

    void FindAllVarDecl(const std::vector<AST*>& stmts, const std::vector<VarDecl*>& idecls)
    {
        auto decls = idecls;
        for(auto stmt : stmts)
        {
            switch(stmt->type())
            {
                case AST::AST_STMT_VAR:
                {
                    VarStmt* var_stmt = static_cast<VarStmt*>(stmt);
                    for(auto d : var_stmt->decls())
                    {
                        decls.emplace_back(d);
                    }
                    break;
                }
                case AST::AST_STMT_FOR:
                {
                    For* for_stmt = static_cast<For*>(stmt);
                    if(for_stmt->expr0s().size() > 0 && for_stmt->expr0s()[0]->type() == AST::AST_STMT_VAR_DECL)
                    {
                        for(AST* ast : for_stmt->expr0s())
                        {
                            VarDecl* d = static_cast<VarDecl*>(ast);
                            decls.emplace_back(d);
                        }
                    }
                    FindAllVarDecl({ for_stmt->statement() }, decls);
                    break;
                }
                case AST::AST_STMT_FOR_IN:
                {
                    ForIn* for_in_stmt = static_cast<ForIn*>(stmt);
                    if(for_in_stmt->expr0()->type() == AST::AST_STMT_VAR_DECL)
                    {
                        VarDecl* d = static_cast<VarDecl*>(for_in_stmt->expr0());
                        decls.emplace_back(d);
                    }
                    FindAllVarDecl({ for_in_stmt->statement() }, decls);
                    break;
                }
                case AST::AST_STMT_BLOCK:
                {
                    Block* block = static_cast<Block*>(stmt);
                    FindAllVarDecl(block->statements(), decls);
                    break;
                }
                case AST::AST_STMT_TRY:
                {
                    Try* try_stmt = static_cast<Try*>(stmt);
                    FindAllVarDecl({ try_stmt->try_block() }, decls);
                    if(try_stmt->catch_block() != nullptr)
                        FindAllVarDecl({ try_stmt->catch_block() }, decls);
                    if(try_stmt->finally_block() != nullptr)
                        FindAllVarDecl({ try_stmt->finally_block() }, decls);
                    break;
                }
                // TODO(zhuzilin) fill the other statements.
                default:
                    break;
            }
        }
    }

    // 10.5 Declaration Binding Instantiation
    void DeclarationBindingInstantiation(
    Error* e, ExecutionContext* context, AST* code, CodeType code_type, FunctionObject* f = nullptr, std::vector<JSValue*> args = {})
    {
        auto env = context->variable_env()->env_rec();// 1
        bool configurable_bindings = false;
        ProgramOrFunctionBody* body = static_cast<ProgramOrFunctionBody*>(code);
        if(code_type == CODE_EVAL)
        {
            configurable_bindings = true;// 2
        }
        bool strict = body->strict();// 3
        if(code_type == CODE_FUNC)
        {// 4
            assert(f != nullptr);
            auto names = f->FormalParameters();// 4.a
            Function* func_ast = static_cast<Function*>(code);
            size_t arg_count = args.size();// 4.b
            size_t n = 0;// 4.c
            for(auto arg_name : names)
            {// 4.d
                JSValue* v = Undefined::Instance();
                if(n < arg_count)// 4.d.i & 4.d.ii
                    v = args[n++];
                bool arg_already_declared = env->HasBinding(arg_name);// 4.d.iii
                if(!arg_already_declared)
                {// 4.d.iv
                    // NOTE(zhuzlin) I'm not sure if this should be false.
                    env->CreateMutableBinding(e, arg_name, false);
                    if(!e->IsOk())
                        return;
                }
                env->SetMutableBinding(e, arg_name, v, strict);// 4.d.v
                if(!e->IsOk())
                    return;
            }
        }
        // 5
        for(Function* func_decl : body->func_decls())
        {
            assert(func_decl->is_named());
            std::u16string fn = func_decl->name();
            FunctionObject* fo = InstantiateFunctionDeclaration(e, func_decl);
            if(!e->IsOk())
                return;
            bool func_already_declared = env->HasBinding(fn);
            if(!func_already_declared)
            {// 5.d
                env->CreateMutableBinding(e, fn, configurable_bindings);
                if(!e->IsOk())
                    return;
            }
            else
            {// 5.e
                auto go = GlobalObject::Instance();
                auto existing_prop = go->GetProperty(fn);
                assert(!existing_prop->IsUndefined());
                auto existing_prop_desc = static_cast<PropertyDescriptor*>(existing_prop);
                if(existing_prop_desc->Configurable())
                {// 5.e.iii
                    auto new_desc = new PropertyDescriptor();
                    new_desc->SetDataDescriptor(Undefined::Instance(), true, true, configurable_bindings);
                    go->DefineOwnProperty(e, fn, new_desc, true);
                    if(!e->IsOk())
                        return;
                }
                else
                {// 5.e.iv
                    if(existing_prop_desc->IsAccessorDescriptor()
                       || !(existing_prop_desc->HasConfigurable() && existing_prop_desc->Configurable()
                            && existing_prop_desc->HasEnumerable() && existing_prop_desc->Enumerable()))
                    {
                        *e = *Error::TypeError();
                        return;
                    }
                }
            }
            env->SetMutableBinding(e, fn, fo, strict);// 5.f
        }
        // 6
        bool arguments_already_declared = env->HasBinding(u"arguments");
        // 7
        if(code_type == CODE_FUNC && !arguments_already_declared)
        {
            auto args_obj = CreateArgumentsObject(f, args, context->variable_env(), strict);
            if(strict)
            {// 7.b
                DeclarativeEnvironmentRecord* decl_env = static_cast<DeclarativeEnvironmentRecord*>(env);
                decl_env->CreateImmutableBinding(u"arguments");
                decl_env->InitializeImmutableBinding(u"arguments", args_obj);
            }
            else
            {// 7.c
                // NOTE(zhuzlin) I'm not sure if this should be false.
                env->CreateMutableBinding(e, u"arguments", false);
                env->SetMutableBinding(e, u"arguments", args_obj, false);
            }
        }
        // 8
        std::vector<VarDecl*> decls;
        FindAllVarDecl(body->statements(), decls);
        for(VarDecl* d : decls)
        {
            std::u16string dn = d->ident();
            bool var_already_declared = env->HasBinding(dn);
            if(!var_already_declared)
            {
                env->CreateMutableBinding(e, dn, configurable_bindings);
                if(!e->IsOk())
                    return;
                env->SetMutableBinding(e, dn, Undefined::Instance(), strict);
                if(!e->IsOk())
                    return;
            }
        }
    }

    // 10.4.1
    void EnterGlobalCode(Error* e, AST* ast)
    {
        ProgramOrFunctionBody* program;
        if(ast->type() == AST::AST_PROGRAM)
        {
            program = static_cast<ProgramOrFunctionBody*>(ast);
        }
        else
        {
            // TODO(zhuzilin) This is for test. Add test label like #ifdefine TEST
            program = new ProgramOrFunctionBody(AST::AST_PROGRAM, false);
            program->AddStatement(ast);
        }
        // 1 10.4.1.1
        LexicalEnvironment* global_env = LexicalEnvironment::Global();
        ExecutionContext* context = new ExecutionContext(global_env, global_env, GlobalObject::Instance(), program->strict());
        RuntimeContext::Global()->AddContext(context);
        // 2
        DeclarationBindingInstantiation(e, context, program, CODE_GLOBAL);
    }

    // 10.4.2
    void EnterEvalCode(Error* e, AST* ast)
    {
        assert(ast->type() == AST::AST_PROGRAM);
        ProgramOrFunctionBody* program = static_cast<ProgramOrFunctionBody*>(ast);
        ExecutionContext* context;
        LexicalEnvironment* variable_env;
        LexicalEnvironment* lexical_env;
        JSValue* this_binding;
        if(!GlobalObject::Instance()->direct_eval())
        {// 1
            LexicalEnvironment* global_env = LexicalEnvironment::Global();
            variable_env = global_env;
            lexical_env = global_env;
            this_binding = GlobalObject::Instance();
        }
        else
        {// 2
            ExecutionContext* calling_context = RuntimeContext::TopContext();
            variable_env = calling_context->variable_env();
            lexical_env = calling_context->lexical_env();
            this_binding = calling_context->this_binding();
        }
        bool strict = RuntimeContext::TopContext()->strict() || (program->strict() && GlobalObject::Instance()->direct_eval());
        if(strict)
        {// 3
            LexicalEnvironment* strict_var_env = LexicalEnvironment::NewDeclarativeEnvironment(lexical_env);
            lexical_env = strict_var_env;
            variable_env = strict_var_env;
        }
        context = new ExecutionContext(variable_env, lexical_env, this_binding, strict);
        RuntimeContext::Global()->AddContext(context);
        // 4
        DeclarationBindingInstantiation(e, context, program, CODE_EVAL);
    }

    // 15.1.2.1 eval(X)
    JSValue* GlobalObject::eval(Error* e, JSValue* this_arg, const std::vector<JSValue*>& vals)
    {
        log::PrintSource("enter GlobalObject::eval");
        if(vals.size() == 0)
            return Undefined::Instance();
        if(!vals[0]->IsString())
            return vals[0];
        std::u16string x = static_cast<String*>(vals[0])->data();
        Parser parser(x);
        AST* program = parser.ParseProgram();
        if(program->IsIllegal())
        {
            *e = *Error::SyntaxError(u"failed to parse eval");
            return nullptr;
        }
        EnterEvalCode(e, program);
        if(!e->IsOk())
            return nullptr;
        Completion result = EvalProgram(program);
        RuntimeContext::Global()->PopContext();

        switch(result.type)
        {
            case Completion::NORMAL:
                if(result.value != nullptr)
                    return result.value;
                else
                    return Undefined::Instance();
            default:
            {
                assert(result.type == Completion::THROW);
                std::u16string message = ::es::ToString(e, result.value);
                if(result.value->IsObject())
                {
                    JSObject* obj = static_cast<JSObject*>(result.value);
                    if(obj->obj_type() == JSObject::OBJ_ERROR)
                    {
                        message = static_cast<ErrorObject*>(obj)->ErrorMessage();
                    }
                }
                *e = *Error::NativeError(message);
                return result.value;
            }
        }
    }

    // 10.4.3
    void EnterFunctionCode(Error* e, JSObject* f, ProgramOrFunctionBody* body, JSValue* this_arg, const std::vector<JSValue*>& args, bool strict)
    {
        assert(f->obj_type() == JSObject::OBJ_FUNC);
        FunctionObject* func = static_cast<FunctionObject*>(f);
        JSValue* this_binding;
        if(strict)
        {// 1
            this_binding = this_arg;
        }
        else
        {// 2 & 3
            this_binding = (this_arg->IsUndefined() || this_arg->IsNull()) ? GlobalObject::Instance() : this_arg;
        }
        LexicalEnvironment* local_env = LexicalEnvironment::NewDeclarativeEnvironment(func->Scope());
        ExecutionContext* context = new ExecutionContext(local_env, local_env, this_binding, strict);// 8
        RuntimeContext::Global()->AddContext(context);
        // 9
        DeclarationBindingInstantiation(e, context, body, CODE_FUNC, func, args);
    }

    void InitGlobalObject()
    {
        auto global_obj = GlobalObject::Instance();
        // 15.1.1 Value Properties of the Global Object
        global_obj->AddValueProperty(u"NaN", Number::NaN(), false, false, false);
        global_obj->AddValueProperty(u"Infinity", Number::PositiveInfinity(), false, false, false);
        global_obj->AddValueProperty(u"undefined", Undefined::Instance(), false, false, false);
        // 15.1.2 Function Properties of the Global Object
        global_obj->AddFuncProperty(u"eval", GlobalObject::eval, true, false, true);
        global_obj->AddFuncProperty(u"parseInt", GlobalObject::parseInt, true, false, true);
        global_obj->AddFuncProperty(u"parseFloat", GlobalObject::parseFloat, true, false, true);
        global_obj->AddFuncProperty(u"isNaN", GlobalObject::isNaN, true, false, true);
        global_obj->AddFuncProperty(u"isFinite", GlobalObject::isFinite, true, false, true);
        // 15.1.3 URI Handling Function Properties
        // TODO(zhuzilin)
        // 15.1.4 Constructor Properties of the Global Object
        global_obj->AddValueProperty(u"Object", ObjectConstructor::Instance(), true, false, true);
        global_obj->AddValueProperty(u"Function", FunctionConstructor::Instance(), true, false, true);
        global_obj->AddValueProperty(u"Number", NumberConstructor::Instance(), true, false, true);
        global_obj->AddValueProperty(u"Boolean", BoolConstructor::Instance(), true, false, true);
        global_obj->AddValueProperty(u"String", StringConstructor::Instance(), true, false, true);
        global_obj->AddValueProperty(u"Array", ArrayConstructor::Instance(), true, false, true);

        global_obj->AddValueProperty(u"Error", ErrorConstructor::Instance(), true, false, true);
        // TODO(zhuzilin) differentiate errors.
        global_obj->AddValueProperty(u"EvalError", ErrorConstructor::Instance(), true, false, true);
        global_obj->AddValueProperty(u"RangeError", ErrorConstructor::Instance(), true, false, true);
        global_obj->AddValueProperty(u"ReferenceError", ErrorConstructor::Instance(), true, false, true);
        global_obj->AddValueProperty(u"SyntaxError", ErrorConstructor::Instance(), true, false, true);
        global_obj->AddValueProperty(u"TypeError", ErrorConstructor::Instance(), true, false, true);
        global_obj->AddValueProperty(u"URIError", ErrorConstructor::Instance(), true, false, true);

        global_obj->AddValueProperty(u"console", Console::Instance(), true, false, true);
    }

    void InitObject()
    {
        ObjectConstructor* constructor = ObjectConstructor::Instance();
        constructor->SetPrototype(FunctionProto::Instance());
        // 15.2.3 Properties of the Object Constructor
        constructor->AddValueProperty(u"prototype", ObjectProto::Instance(), false, false, false);
        constructor->AddFuncProperty(u"toString", ObjectConstructor::toString, false, false, false);
        // TODO(zhuzilin) check if the config is correct.
        constructor->AddFuncProperty(u"getPrototypeOf", ObjectConstructor::getPrototypeOf, false, false, false);
        constructor->AddFuncProperty(u"getOwnPropertyDescriptor", ObjectConstructor::getOwnPropertyDescriptor, false, false, false);
        constructor->AddFuncProperty(u"getOwnPropertyNames", ObjectConstructor::getOwnPropertyNames, false, false, false);
        constructor->AddFuncProperty(u"create", ObjectConstructor::create, false, false, false);
        constructor->AddFuncProperty(u"defineProperty", ObjectConstructor::defineProperty, false, false, false);
        constructor->AddFuncProperty(u"defineProperties", ObjectConstructor::defineProperties, false, false, false);
        constructor->AddFuncProperty(u"seal", ObjectConstructor::seal, false, false, false);
        constructor->AddFuncProperty(u"freeze", ObjectConstructor::freeze, false, false, false);
        constructor->AddFuncProperty(u"preventExtensions", ObjectConstructor::preventExtensions, false, false, false);
        constructor->AddFuncProperty(u"isSealed", ObjectConstructor::isSealed, false, false, false);
        constructor->AddFuncProperty(u"isFrozen", ObjectConstructor::isFrozen, false, false, false);
        constructor->AddFuncProperty(u"isExtensible", ObjectConstructor::isExtensible, false, false, false);
        constructor->AddFuncProperty(u"keys", ObjectConstructor::keys, false, false, false);
        // ES6
        constructor->AddFuncProperty(u"setPrototypeOf", ObjectConstructor::setPrototypeOf, false, false, false);

        ObjectProto* proto = ObjectProto::Instance();
        // 15.2.4 Properties of the Object Prototype Object
        proto->AddValueProperty(u"constructor", ObjectConstructor::Instance(), false, false, false);
        proto->AddFuncProperty(u"toString", ObjectProto::toString, false, false, false);
        proto->AddFuncProperty(u"toLocaleString", ObjectProto::toLocaleString, false, false, false);
        proto->AddFuncProperty(u"valueOf", ObjectProto::valueOf, false, false, false);
        proto->AddFuncProperty(u"hasOwnProperty", ObjectProto::hasOwnProperty, false, false, false);
        proto->AddFuncProperty(u"isPrototypeOf", ObjectProto::isPrototypeOf, false, false, false);
        proto->AddFuncProperty(u"propertyIsEnumerable", ObjectProto::propertyIsEnumerable, false, false, false);
    }

    void InitFunction()
    {
        FunctionConstructor* constructor = FunctionConstructor::Instance();
        constructor->SetPrototype(FunctionProto::Instance());
        // 15.3.3 Properties of the Function Constructor
        constructor->AddValueProperty(u"prototype", FunctionProto::Instance(), false, false, false);
        constructor->AddValueProperty(u"length", Number::One(), false, false, false);
        constructor->AddFuncProperty(u"toString", FunctionConstructor::toString, false, false, false);

        FunctionProto* proto = FunctionProto::Instance();
        proto->SetPrototype(ObjectProto::Instance());
        // 15.2.4 Properties of the Function Prototype Function
        proto->AddValueProperty(u"constructor", FunctionConstructor::Instance(), false, false, false);
        proto->AddFuncProperty(u"toString", FunctionProto::toString, false, false, false);
        proto->AddFuncProperty(u"apply", FunctionProto::apply, false, false, false);
        proto->AddFuncProperty(u"call", FunctionProto::call, false, false, false);
        proto->AddFuncProperty(u"bind", FunctionProto::bind, false, false, false);
    }

    void InitNumber()
    {
        NumberConstructor* constructor = NumberConstructor::Instance();
        constructor->SetPrototype(FunctionProto::Instance());
        // 15.3.3 Properties of the Number Constructor
        constructor->AddValueProperty(u"prototype", NumberProto::Instance(), false, false, false);
        constructor->AddValueProperty(u"length", Number::One(), false, false, false);
        constructor->AddValueProperty(u"MAX_VALUE", new Number(1.7976931348623157e308), false, false, false);
        constructor->AddValueProperty(u"MIN_VALUE", new Number(5e-324), false, false, false);
        constructor->AddValueProperty(u"NaN", Number::NaN(), false, false, false);
        constructor->AddValueProperty(u"NEGATIVE_INFINITY", Number::PositiveInfinity(), false, false, false);
        constructor->AddValueProperty(u"POSITIVE_INFINITY", Number::NegativeInfinity(), false, false, false);

        NumberProto* proto = NumberProto::Instance();
        proto->SetPrototype(ObjectProto::Instance());
        // 15.2.4 Properties of the Number Prototype Number
        proto->AddValueProperty(u"constructor", NumberConstructor::Instance(), false, false, false);
        proto->AddFuncProperty(u"toString", NumberProto::toString, false, false, false);
        proto->AddFuncProperty(u"toLocaleString", NumberProto::toLocaleString, false, false, false);
        proto->AddFuncProperty(u"valueOf", NumberProto::valueOf, false, false, false);
        proto->AddFuncProperty(u"toFixed", NumberProto::toFixed, false, false, false);
        proto->AddFuncProperty(u"toExponential", NumberProto::toExponential, false, false, false);
        proto->AddFuncProperty(u"toPrecision", NumberProto::toPrecision, false, false, false);
    }

    void InitError()
    {
        ErrorConstructor* constructor = ErrorConstructor::Instance();
        constructor->SetPrototype(FunctionProto::Instance());
        // 15.11.3 Properties of the Error Constructor
        constructor->AddValueProperty(u"prototype", ErrorProto::Instance(), false, false, false);
        constructor->AddValueProperty(u"length", Number::One(), false, false, false);
        constructor->AddFuncProperty(u"toString", NumberConstructor::toString, false, false, false);

        ErrorProto* proto = ErrorProto::Instance();
        proto->SetPrototype(ObjectProto::Instance());
        // 15.11.4 Properties of the Error Prototype Object
        proto->AddValueProperty(u"constructor", ErrorConstructor::Instance(), false, false, false);
        proto->AddValueProperty(u"name", new String(u"Error"), false, false, false);
        proto->AddValueProperty(u"message", String::Empty(), true, false, false);
        proto->AddFuncProperty(u"call", ErrorProto::toString, false, false, false);
    }

    void InitBool()
    {
        BoolConstructor* constructor = BoolConstructor::Instance();
        constructor->SetPrototype(FunctionProto::Instance());
        // 15.6.3 Properties of the Boolean Constructor
        constructor->AddValueProperty(u"prototype", BoolProto::Instance(), false, false, false);
        constructor->AddFuncProperty(u"toString", BoolConstructor::toString, false, false, false);

        BoolProto* proto = BoolProto::Instance();
        proto->SetPrototype(ObjectProto::Instance());
        // 15.6.4 Properties of the Boolean Prototype Object
        proto->AddValueProperty(u"constructor", BoolConstructor::Instance(), false, false, false);
        proto->AddFuncProperty(u"toString", BoolProto::toString, false, false, false);
        proto->AddFuncProperty(u"valueOf", BoolProto::valueOf, false, false, false);
    }

    void InitString()
    {
        StringConstructor* constructor = StringConstructor::Instance();
        constructor->SetPrototype(FunctionProto::Instance());
        // 15.3.3 Properties of the String Constructor
        constructor->AddValueProperty(u"prototype", StringProto::Instance(), false, false, false);
        constructor->AddValueProperty(u"length", Number::One(), true, false, false);
        constructor->AddFuncProperty(u"fromCharCode", StringConstructor::fromCharCode, false, false, false);
        constructor->AddFuncProperty(u"toString", StringConstructor::toString, false, false, false);

        StringProto* proto = StringProto::Instance();
        proto->SetPrototype(ObjectProto::Instance());
        // 15.2.4 Properties of the String Prototype String
        proto->AddValueProperty(u"constructor", StringConstructor::Instance(), false, false, false);
        proto->AddFuncProperty(u"toString", StringProto::toString, false, false, false);
        proto->AddFuncProperty(u"valueOf", StringProto::valueOf, false, false, false);
        proto->AddFuncProperty(u"charAt", StringProto::charAt, false, false, false);
        proto->AddFuncProperty(u"charCodeAt", StringProto::charCodeAt, false, false, false);
        proto->AddFuncProperty(u"concat", StringProto::concat, false, false, false);
        proto->AddFuncProperty(u"indexOf", StringProto::indexOf, false, false, false);
        proto->AddFuncProperty(u"lastIndexOf", StringProto::lastIndexOf, false, false, false);
        proto->AddFuncProperty(u"localeCompare", StringProto::localeCompare, false, false, false);
        proto->AddFuncProperty(u"match", StringProto::match, false, false, false);
        proto->AddFuncProperty(u"replace", StringProto::replace, false, false, false);
        proto->AddFuncProperty(u"search", StringProto::search, false, false, false);
        proto->AddFuncProperty(u"slice", StringProto::slice, false, false, false);
        proto->AddFuncProperty(u"split", StringProto::split, false, false, false);
        proto->AddFuncProperty(u"substring", StringProto::substring, false, false, false);
        proto->AddFuncProperty(u"toLowerCase", StringProto::toLowerCase, false, false, false);
        proto->AddFuncProperty(u"toLocaleLowerCase", StringProto::toLocaleLowerCase, false, false, false);
        proto->AddFuncProperty(u"toUpperCase", StringProto::toUpperCase, false, false, false);
        proto->AddFuncProperty(u"toLocaleUpperCase", StringProto::toLocaleUpperCase, false, false, false);
        proto->AddFuncProperty(u"trim", StringProto::trim, false, false, false);
    }

    void InitArray()
    {
        ArrayConstructor* constructor = ArrayConstructor::Instance();
        constructor->SetPrototype(FunctionProto::Instance());
        // 15.6.3 Properties of the Arrayean Constructor
        constructor->AddValueProperty(u"length", Number::One(), false, false, false);
        constructor->AddValueProperty(u"prototype", ArrayProto::Instance(), false, false, false);
        constructor->AddFuncProperty(u"isArray", ArrayConstructor::isArray, false, false, false);
        constructor->AddFuncProperty(u"toString", ArrayConstructor::toString, false, false, false);

        ArrayProto* proto = ArrayProto::Instance();
        proto->SetPrototype(ObjectProto::Instance());
        // 15.6.4 Properties of the Arrayean Prototype Object
        proto->AddValueProperty(u"length", Number::Zero(), false, false, false);
        proto->AddValueProperty(u"constructor", ArrayConstructor::Instance(), false, false, false);
        proto->AddFuncProperty(u"toString", ArrayProto::toString, false, false, false);
        proto->AddFuncProperty(u"toLocaleString", ArrayProto::toLocaleString, false, false, false);
        proto->AddFuncProperty(u"concat", ArrayProto::concat, false, false, false);
        proto->AddFuncProperty(u"join", ArrayProto::join, false, false, false);
        proto->AddFuncProperty(u"pop", ArrayProto::pop, false, false, false);
        proto->AddFuncProperty(u"push", ArrayProto::push, false, false, false);
        proto->AddFuncProperty(u"reverse", ArrayProto::reverse, false, false, false);
        proto->AddFuncProperty(u"shift", ArrayProto::shift, false, false, false);
        proto->AddFuncProperty(u"slice", ArrayProto::slice, false, false, false);
        proto->AddFuncProperty(u"sort", ArrayProto::sort, false, false, false);
        proto->AddFuncProperty(u"splice", ArrayProto::splice, false, false, false);
        proto->AddFuncProperty(u"unshift", ArrayProto::unshift, false, false, false);
        proto->AddFuncProperty(u"indexOf", ArrayProto::indexOf, false, false, false);
        proto->AddFuncProperty(u"lastIndexOf", ArrayProto::lastIndexOf, false, false, false);
        proto->AddFuncProperty(u"every", ArrayProto::every, false, false, false);
        proto->AddFuncProperty(u"some", ArrayProto::some, false, false, false);
        proto->AddFuncProperty(u"forEach", ArrayProto::forEach, false, false, false);
        proto->AddFuncProperty(u"map", ArrayProto::map, false, false, false);
        proto->AddFuncProperty(u"filter", ArrayProto::filter, false, false, false);
        proto->AddFuncProperty(u"reduce", ArrayProto::reduce, false, false, false);
        proto->AddFuncProperty(u"reduceRight", ArrayProto::reduceRight, false, false, false);
    }

    void Init()
    {
        InitGlobalObject();
        InitObject();
        InitFunction();
        InitNumber();
        InitError();
        InitBool();
        InitString();
        InitArray();
    }

    JSValue* ToPrimitive(Error* e, JSValue* input, const std::u16string& preferred_type)
    {
        assert(input->IsLanguageType());
        if(input->IsPrimitive())
        {
            return input;
        }
        JSObject* obj = static_cast<JSObject*>(input);
        return obj->DefaultValue(e, preferred_type);
    }

    bool ToBoolean(JSValue* input)
    {
        assert(input->IsLanguageType());
        switch(input->type())
        {
            case JSValue::JS_UNDEFINED:
            case JSValue::JS_NULL:
                return false;
            case JSValue::JS_BOOL:
                return static_cast<Bool*>(input)->data();
            case JSValue::JS_NUMBER:
            {
                Number* num = static_cast<Number*>(input);
                if(num->data() == 0.0 || num->data() == -0.0 || num->IsNaN())
                {
                    return false;
                }
                return true;
            }
            case JSValue::JS_STRING:
            {
                String* str = static_cast<String*>(input);
                return str->data() != u"";
            }
            case JSValue::JS_OBJECT:
                return true;
            default:
                assert(false);
        }
    }

    double StringToNumber(const std::u16string& source)
    {
        size_t start = 0;
        size_t end = source.size();
        bool positive = true;
        double val = 0;
        while(start < end)
        {
            char16_t c = source[start];
            if(!character::IsWhiteSpace(c) && !character::IsLineTerminator(c))
                break;
            start++;
        }
        while(start < end)
        {
            char16_t c = source[end - 1];
            if(!character::IsWhiteSpace(c) && !character::IsLineTerminator(c))
                break;
            end--;
        }
        if(start == end)
        {
            goto error;
        }
        else if(source[start] == u'-')
        {
            positive = false;
            start++;
        }
        else if(source[start] == u'+')
        {
            start++;
        }
        else if(end - start > 2 && source[start] == u'0' && (source[start + 1] == u'x' || source[start + 1] == u'X'))
        {
            // 0xABCD...
            start += 2;
            while(start < end)
            {
                char16_t c = source[start];
                if(!character::IsHexDigit(c))
                    goto error;
                val = val * 16 + character::Digit(c);
                start++;
            }
            return val;
        }

        if(start == end)
            goto error;

        if(source.substr(start, end - start) == u"Infinity")
        {
            return positive ? Number::PositiveInfinity()->data() : Number::NegativeInfinity()->data();
        }

        while(start < end)
        {
            char16_t c = source[start];
            if(!character::IsDecimalDigit(c))
                break;
            val = val * 10 + character::Digit(c);
            start++;
        }
        if(start == end)
            return positive ? val : -val;

        if(source[start] == u'.')
        {
            start++;
            double frac = 0.1;
            while(start < end)
            {
                char16_t c = source[start];
                if(!character::IsDecimalDigit(c))
                {
                    break;
                }
                val += frac * character::Digit(c);
                frac /= 10;
                start++;
            }
            if(start == end)
                return positive ? val : -val;
        }

        if(source[start] == u'e' || source[start] == u'E')
        {
            start++;
            double exp = 0;
            bool exp_positive = true;
            if(start == end)
                goto error;
            if(source[start] == u'-')
            {
                exp_positive = false;
                start++;
            }
            else if(source[start] == u'-')
            {
                start++;
            }
            if(start == end)
                goto error;

            while(start < end)
            {
                char16_t c = source[start];
                if(!character::IsDecimalDigit(c))
                {
                    break;
                }
                exp = exp * 10 + character::Digit(c);
                start++;
            }
            if(start == end)
            {
                val = val * pow(10, exp);
                return positive ? val : -val;
            }
        }
    error:
        return nan("");
    }

    double StringToNumber(String* str)
    {
        return StringToNumber(str->data());
    }

    double ToNumber(Error* e, JSValue* input)
    {
        assert(input->IsLanguageType());
        switch(input->type())
        {
            case JSValue::JS_UNDEFINED:
                return nan("");
            case JSValue::JS_NULL:
                return 0.0;
            case JSValue::JS_BOOL:
                return static_cast<Bool*>(input)->data() ? 1.0 : 0.0;
            case JSValue::JS_NUMBER:
                return static_cast<Number*>(input)->data();
            case JSValue::JS_STRING:
                return StringToNumber(static_cast<String*>(input));
            case JSValue::JS_OBJECT:
            {
                JSValue* prim_value = ToPrimitive(e, input, u"Number");
                if(!e->IsOk())
                    return 0.0;
                return ToNumber(e, prim_value);
            }
            default:
                assert(false);
        }
    }

    double ToInteger(Error* e, JSValue* input)
    {
        double num = ToNumber(e, input);
        if(!e->IsOk())
            return 0.0;
        if(isnan(num))
        {
            return 0.0;
        }
        if(isinf(num) || num == 0)
        {
            return num;
        }
        return num > 0 ? floor(abs(num)) : -(floor(abs(-num)));
    }

    double ToInt32(Error* e, JSValue* input)
    {
        double num = ToNumber(e, input);
        if(!e->IsOk())
            return 0;
        if(isnan(num) || isinf(num) || num == 0)
        {
            return 0.0;
        }
        double pos_int = num > 0 ? floor(abs(num)) : -(floor(abs(-num)));
        double int32_bit = fmod(pos_int, pow(2, 32));
        if(int32_bit < 0)
            int32_bit += pow(2, 32);

        if(int32_bit > pow(2, 31))
        {
            return int32_bit - pow(2, 32);
        }
        else
        {
            return int32_bit;
        }
    }

    double ToUint(Error* e, JSValue* input, char bits)
    {
        double num = ToNumber(e, input);
        if(!e->IsOk())
            return 0.0;
        if(isnan(num) || isinf(num) || num == 0)
        {
            return 0.0;
        }
        double pos_int = num > 0 ? floor(abs(num)) : -(floor(abs(-num)));
        double int_bit = fmod(pos_int, pow(2, bits));
        if(int_bit < 0)
            int_bit += pow(2, bits);
        return int_bit;
    }

    double ToUint32(Error* e, JSValue* input)
    {
        return ToUint(e, input, 32);
    }

    double ToUint16(Error* e, JSValue* input)
    {
        return ToUint(e, input, 16);
    }

    std::u16string NumberToString(double m)
    {
        // TODO(zhuzilin) Figure out how to solve the large number error.
        if(m == 0)
            return String::Zero()->data();
        std::u16string sign = u"";
        if(m < 0)
        {
            m = -m;
            sign = u"-";
        }
        // the fraction digits, e.g. 1.23's frac_digit = 2, 4200's = -2
        int frac_digit = 0;
        // the total digits, e.g. 1.23's k = 3, 4200's k = 2
        int k = 0;
        // n - k = -frac_digit
        int n = 0;
        double tmp, tmp_m;
        while(modf(m, &tmp) != 0)
        {
            frac_digit++;
            m *= 10;
        }
        while(fmod(m, 10) < 1e-6)
        {
            frac_digit--;
            m /= 10;
        }
        double s = m;
        while(m > 0.5)
        {
            k++;
            m /= 10;
            modf(m, &tmp);
            m = tmp;
        }
        n = k - frac_digit;
        std::u16string res = u"";
        if(k <= n && n <= 21)
        {
            while(s > 0.5)
            {
                res += u'0' + int(fmod(s, 10));
                s /= 10;
                modf(s, &tmp);
                s = tmp;
            }
            reverse(res.begin(), res.end());
            res += std::u16string(n - k, u'0');
            return sign + res;
        }
        if(0 < n && n <= 21)
        {
            for(size_t i = 0; i < k; i++)
            {
                res += u'0' + int(fmod(s, 10));
                if(i + 1 == k - n)
                {
                    res += u'.';
                }
                s /= 10;
                modf(s, &tmp);
                s = tmp;
            }
            reverse(res.begin(), res.end());
            return sign + res;
        }
        if(-6 < n && n <= 0)
        {
            for(size_t i = 0; i < k; i++)
            {
                res += u'0' + int(fmod(s, 10));
                s /= 10;
                modf(s, &tmp);
                s = tmp;
            }
            reverse(res.begin(), res.end());
            res = u"0." + std::u16string(-n, u'0') + res;
            return sign + res;
        }
        if(k == 1)
        {
            res += u'0' + int(s);
            res += u"e";
            if(n - 1 > 0)
            {
                res += u"+" + NumberToString(n - 1);
            }
            else
            {
                res += u"-" + NumberToString(1 - n);
            }
            return sign + res;
        }
        for(size_t i = 0; i < k; i++)
        {
            res += u'0' + int(fmod(s, 10));
            if(i + 1 == k - 1)
            {
                res += u'.';
            }
            s /= 10;
            modf(s, &tmp);
            s = tmp;
        }
        res += u"e";
        if(n - 1 > 0)
        {
            res += u"+" + NumberToString(n - 1);
        }
        else
        {
            res += u"-" + NumberToString(1 - n);
        }
        return sign + res;
    }

    std::u16string NumberToString(Number* num)
    {
        if(num->IsNaN())
            return String::NaN()->data();
        if(num->IsInfinity())
            return String::Infinity()->data();
        return NumberToString(num->data());
    }

    std::u16string ToString(Error* e, JSValue* input)
    {
        assert(input->IsLanguageType());
        switch(input->type())
        {
            case JSValue::JS_UNDEFINED:
                return String::Undefined()->data();
            case JSValue::JS_NULL:
                return String::Null()->data();
            case JSValue::JS_BOOL:
                return static_cast<Bool*>(input)->data() ? u"true" : u"false";
            case JSValue::JS_NUMBER:
                return NumberToString(static_cast<Number*>(input));
            case JSValue::JS_STRING:
                return static_cast<String*>(input)->data();
            case JSValue::JS_OBJECT:
            {
                JSValue* prim_value = ToPrimitive(e, input, u"String");
                if(!e->IsOk())
                    return u"";
                return ToString(e, prim_value);
            }
            default:
                assert(false);
        }
    }

    JSObject* ToObject(Error* e, JSValue* input)
    {
        assert(input->IsLanguageType());
        switch(input->type())
        {
            case JSValue::JS_UNDEFINED:
            case JSValue::JS_NULL:
                *e = *Error::TypeError(u"Cannot convert undefined or null to object");
                return nullptr;
            case JSValue::JS_BOOL:
                return new BoolObject(input);
            case JSValue::JS_NUMBER:
                return new NumberObject(input);
            case JSValue::JS_STRING:
                return new StringObject(input);
            case JSValue::JS_OBJECT:
                return static_cast<JSObject*>(input);
            default:
                assert(false);
        }

    }// namespace es

    // 11.8.5 The Abstract Relational Comparison Algorithm
    // x < y
    JSValue* LessThan(Error* e, JSValue* x, JSValue* y, bool left_first = true)
    {
        JSValue *px, *py;
        if(left_first)
        {
            px = ToPrimitive(e, x, u"Number");
            if(!e->IsOk())
                return Undefined::Instance();
            py = ToPrimitive(e, y, u"Number");
            if(!e->IsOk())
                return Undefined::Instance();
        }
        else
        {
            py = ToPrimitive(e, y, u"Number");
            if(!e->IsOk())
                return Undefined::Instance();
            px = ToPrimitive(e, x, u"Number");
            if(!e->IsOk())
                return Undefined::Instance();
        }
        if(!px->IsString() && !py->IsString())
        {// 3
            double nx = ToNumber(e, px);
            if(!e->IsOk())
                return Undefined::Instance();
            double ny = ToNumber(e, py);
            if(!e->IsOk())
                return Undefined::Instance();
            if(isnan(nx) || isnan(ny))
                return Undefined::Instance();
            if(nx == ny)// this includes +0 vs -0
                return Bool::False();
            if(isinf(nx) && !signbit(nx))// nx = +inf
                return Bool::False();
            if(isinf(ny) && !signbit(ny))// ny = +inf
                return Bool::True();
            if(isinf(ny) && signbit(ny))// ny = -inf
                return Bool::False();
            if(isinf(nx) && signbit(nx))// nx = -inf
                return Bool::True();
            return Bool::Wrap(nx < ny);
        }
        else
        {// 4
            std::u16string sx = ToString(e, px);
            if(!e->IsOk())
                return Undefined::Instance();
            std::u16string sy = ToString(e, py);
            if(!e->IsOk())
                return Undefined::Instance();
            return Bool::Wrap(sx < sy);
        }
    }

    // 11.9.3 The Abstract Equality Comparison Algorithm
    // x == y
    bool Equal(Error* e, JSValue* x, JSValue* y)
    {
        if(x->type() == y->type())
        {
            if(x->IsUndefined())
            {
                return true;
            }
            else if(x->IsNull())
            {
                return true;
            }
            else if(x->IsNumber())
            {
                Number* numx = static_cast<Number*>(x);
                Number* numy = static_cast<Number*>(y);
                if(numx->IsNaN() || numy->IsNaN())
                    return false;
                return numx->data() == numy->data();
            }
            else if(x->IsString())
            {
                String* sx = static_cast<String*>(x);
                String* sy = static_cast<String*>(y);
                return sx->data() == sy->data();
            }
            return x == y;
        }
        if(x->IsNull() && y->IsUndefined())
        {// 2
            return true;
        }
        else if(x->IsUndefined() && y->IsNull())
        {// 3
            return true;
        }
        else if(x->IsNumber() && y->IsString())
        {// 4
            double numy = ToNumber(e, y);
            if(!e->IsOk())
                return false;
            return Equal(e, x, new Number(numy));
        }
        else if(x->IsString() && y->IsNumber())
        {// 5
            double numx = ToNumber(e, x);
            if(!e->IsOk())
                return false;
            return Equal(e, new Number(numx), y);
        }
        else if(x->IsBool())
        {// 6
            double numx = ToNumber(e, x);
            if(!e->IsOk())
                return false;
            return Equal(e, new Number(numx), y);
        }
        else if(y->IsBool())
        {// 7
            double numy = ToNumber(e, x);
            if(!e->IsOk())
                return false;
            return Equal(e, x, new Number(numy));
        }
        else if(x->IsNumber() || x->IsString())
        {// 8
            JSValue* primy = ToPrimitive(e, y, u"");
            if(!e->IsOk())
                return false;
            return Equal(e, x, primy);
        }
        else if(x->IsObject() && (y->IsNumber() || y->IsString()))
        {// 9
            JSValue* primx = ToPrimitive(e, y, u"");
            if(!e->IsOk())
                return false;
            return Equal(e, primx, y);
        }
        return false;
    }

    // 11.9.6 The Strict Equality Comparison Algorithm
    // x === y
    bool StrictEqual(Error* e, JSValue* x, JSValue* y)
    {
        if(x->type() != y->type())
            return false;
        switch(x->type())
        {
            case JSValue::JS_UNDEFINED:
                return true;
            case JSValue::JS_NULL:
                return true;
            case JSValue::JS_NUMBER:
            {
                Number* num_x = static_cast<Number*>(x);
                Number* num_y = static_cast<Number*>(y);
                if(num_x->IsNaN() || num_y->IsNaN())
                    return false;
                double dx = num_x->data();
                double dy = num_y->data();
                return dx == dy;
            }
            case JSValue::JS_STRING:
            {
                String* str_x = static_cast<String*>(x);
                String* str_y = static_cast<String*>(y);
                return str_x->data() == str_y->data();
            }
            case JSValue::JS_BOOL:
            {
                Bool* b_x = static_cast<Bool*>(x);
                Bool* b_y = static_cast<Bool*>(y);
                return b_x->data() == b_y->data();
            }
            default:
                return x == y;
        }
    }

    Completion EvalProgram(AST* ast);

    Completion EvalStatement(AST* ast);
    Completion EvalStatementList(const std::vector<AST*>& statements);
    Completion EvalBlockStatement(AST* ast);
    std::u16string EvalVarDecl(Error* e, AST* ast);
    Completion EvalVarStatement(AST* ast);
    Completion EvalIfStatement(AST* ast);
    Completion EvalForStatement(AST* ast);
    Completion EvalForInStatement(AST* ast);
    Completion EvalDoWhileStatement(AST* ast);
    Completion EvalWhileStatement(AST* ast);
    Completion EvalContinueStatement(AST* ast);
    Completion EvalBreakStatement(AST* ast);
    Completion EvalReturnStatement(AST* ast);
    Completion EvalLabelledStatement(AST* ast);
    Completion EvalWithStatement(AST* ast);
    Completion EvalSwitchStatement(AST* ast);
    Completion EvalThrowStatement(AST* ast);
    Completion EvalTryStatement(AST* ast);
    Completion EvalExpressionStatement(AST* ast);

    JSValue* EvalExpression(Error* e, AST* ast);
    JSValue* EvalPrimaryExpression(Error* e, AST* ast);
    Reference* EvalIdentifier(AST* ast);
    Number* EvalNumber(AST* ast);
    String* EvalString(AST* ast);
    Object* EvalObject(Error* e, AST* ast);
    ArrayObject* EvalArray(Error* e, AST* ast);
    JSValue* EvalUnaryOperator(Error* e, AST* ast);
    JSValue* EvalBinaryExpression(Error* e, AST* ast);
    JSValue* EvalBinaryExpression(Error* e, const std::u16string& op, AST* lval, AST* rval);
    JSValue* EvalBinaryExpression(Error* e, const std::u16string& op, JSValue* lval, JSValue* rval);
    JSValue* EvalArithmeticOperator(Error* e, const std::u16string& op, JSValue* lval, JSValue* rval);
    JSValue* EvalAddOperator(Error* e, JSValue* lval, JSValue* rval);
    JSValue* EvalBitwiseShiftOperator(Error* e, const std::u16string& op, JSValue* lval, JSValue* rval);
    JSValue* EvalRelationalOperator(Error* e, const std::u16string& op, JSValue* lval, JSValue* rval);
    JSValue* EvalEqualityOperator(Error* e, const std::u16string& op, JSValue* lval, JSValue* rval);
    JSValue* EvalBitwiseOperator(Error* e, const std::u16string& op, JSValue* lval, JSValue* rval);
    JSValue* EvalLogicalOperator(Error* e, const std::u16string& op, AST* lhs, AST* rhs);
    JSValue* EvalSimpleAssignment(Error* e, JSValue* lref, JSValue* rval);
    JSValue* EvalCompoundAssignment(Error* e, const std::u16string& op, JSValue* lref, JSValue* rval);
    JSValue* EvalTripleConditionExpression(Error* e, AST* ast);
    JSValue* EvalAssignmentExpression(Error* e, AST* ast);
    JSValue* EvalLeftHandSideExpression(Error* e, AST* ast);
    std::vector<JSValue*> EvalArgumentsList(Error* e, Arguments* ast);
    JSValue* EvalCallExpression(Error* e, JSValue* ref, const std::vector<JSValue*>& arg_list);
    JSValue* EvalIndexExpression(Error* e, JSValue* base_ref, const std::u16string& identifier_name, ValueGuard& guard);
    JSValue* EvalIndexExpression(Error* e, JSValue* base_ref, AST* expr, ValueGuard& guard);
    JSValue* EvalExpressionList(Error* e, AST* ast);

    Reference* IdentifierResolution(const std::u16string& name);

    Completion EvalProgram(AST* ast)
    {
        assert(ast->type() == AST::AST_PROGRAM || ast->type() == AST::AST_FUNC_BODY);
        auto prog = static_cast<ProgramOrFunctionBody*>(ast);
        auto statements = prog->statements();
        // 12.9 considered syntactically incorrect if it contains
        //      a return statement that is not within a FunctionBody.
        if(ast->type() != AST::AST_FUNC_BODY)
        {
            for(auto stmt : statements)
            {
                if(stmt->type() == AST::AST_STMT_RETURN)
                {
                    return Completion(Completion::THROW, new ErrorObject(Error::SyntaxError()), u"");
                }
            }
        }

        Completion head_result;
        if(statements.size() == 0)
            return Completion(Completion::NORMAL, nullptr, u"");
        for(auto stmt : prog->statements())
        {
            if(head_result.IsAbruptCompletion())
                break;
            Completion tail_result = EvalStatement(stmt);
            if(tail_result.IsThrow())
                return tail_result;
            head_result = Completion(tail_result.type, tail_result.value == nullptr ? head_result.value : tail_result.value,
                                     tail_result.target);
        }
        return head_result;
    }

    Completion EvalStatement(AST* ast)
    {
        switch(ast->type())
        {
            case AST::AST_STMT_BLOCK:
                return EvalBlockStatement(ast);
            case AST::AST_STMT_VAR:
                return EvalVarStatement(ast);
            case AST::AST_STMT_EMPTY:
                return Completion(Completion::NORMAL, nullptr, u"");
            case AST::AST_STMT_IF:
                return EvalIfStatement(ast);
            case AST::AST_STMT_DO_WHILE:
                return EvalDoWhileStatement(ast);
            case AST::AST_STMT_WHILE:
                return EvalWhileStatement(ast);
            case AST::AST_STMT_FOR:
                return EvalForStatement(ast);
            case AST::AST_STMT_FOR_IN:
                return EvalForInStatement(ast);
            case AST::AST_STMT_CONTINUE:
                return EvalContinueStatement(ast);
            case AST::AST_STMT_BREAK:
                return EvalBreakStatement(ast);
            case AST::AST_STMT_RETURN:
                return EvalReturnStatement(ast);
            case AST::AST_STMT_WITH:
                return EvalWithStatement(ast);
            case AST::AST_STMT_LABEL:
                return EvalLabelledStatement(ast);
            case AST::AST_STMT_SWITCH:
                return EvalSwitchStatement(ast);
            case AST::AST_STMT_THROW:
                return EvalThrowStatement(ast);
            case AST::AST_STMT_TRY:
                return EvalTryStatement(ast);
            case AST::AST_STMT_DEBUG:
                return Completion(Completion::NORMAL, nullptr, u"");
            default:
                return EvalExpressionStatement(ast);
        }
    }

    Completion EvalStatementList(const std::vector<AST*>& statements)
    {
        Completion sl;
        for(auto stmt : statements)
        {
            Completion s = EvalStatement(stmt);
            if(s.IsThrow())
                return s;
            sl = Completion(s.type, s.value == nullptr ? sl.value : s.value, s.target);
            if(sl.IsAbruptCompletion())
                return sl;
        }
        return sl;
    }

    Completion EvalBlockStatement(AST* ast)
    {
        assert(ast->type() == AST::AST_STMT_BLOCK);
        Block* block = static_cast<Block*>(ast);
        return EvalStatementList(block->statements());
    }

    std::u16string EvalVarDecl(Error* e, AST* ast)
    {
        assert(ast->type() == AST::AST_STMT_VAR_DECL);
        VarDecl* decl = static_cast<VarDecl*>(ast);
        if(decl->init() == nullptr)
            return decl->ident();
        JSValue* lhs = IdentifierResolution(decl->ident());
        JSValue* rhs = EvalAssignmentExpression(e, decl->init());
        if(!e->IsOk())
            return decl->ident();
        JSValue* value = GetValue(e, rhs);
        if(!e->IsOk())
            return decl->ident();
        PutValue(e, lhs, value);
        if(!e->IsOk())
            return decl->ident();
        return decl->ident();
    }

    Completion EvalVarStatement(AST* ast)
    {
        assert(ast->type() == AST::AST_STMT_VAR);
        Error* e = Error::Ok();
        VarStmt* var_stmt = static_cast<VarStmt*>(ast);
        for(VarDecl* decl : var_stmt->decls())
        {
            if(decl->init() == nullptr)
                continue;
            EvalVarDecl(e, decl);
            if(!e->IsOk())
                goto error;
        }
        return Completion(Completion::NORMAL, nullptr, u"");
    error:
        return Completion(Completion::THROW, new ErrorObject(e), u"");
    }

    Completion EvalIfStatement(AST* ast)
    {
        assert(ast->type() == AST::AST_STMT_IF);
        Error* e = Error::Ok();
        If* if_stmt = static_cast<If*>(ast);
        JSValue* expr_ref = EvalExpression(e, if_stmt->cond());
        if(!e->IsOk())
            return Completion(Completion::THROW, new ErrorObject(e), u"");
        JSValue* expr = GetValue(e, expr_ref);
        if(!e->IsOk())
            return Completion(Completion::THROW, new ErrorObject(e), u"");
        if(ToBoolean(expr))
        {
            return EvalStatement(if_stmt->if_block());
        }
        else if(if_stmt->else_block() != nullptr)
        {
            return EvalStatement(if_stmt->else_block());
        }
        return Completion(Completion::NORMAL, nullptr, u"");
    }

    // 12.6.1 The do-while Statement
    Completion EvalDoWhileStatement(AST* ast)
    {
        assert(ast->type() == AST::AST_STMT_DO_WHILE);
        Error* e = Error::Ok();
        RuntimeContext::TopContext()->EnterIteration();
        DoWhile* loop_stmt = static_cast<DoWhile*>(ast);
        JSValue* V = nullptr;
        JSValue* expr_ref;
        JSValue* val;
        Completion stmt;
        bool has_label;
        while(true)
        {
            stmt = EvalStatement(loop_stmt->stmt());
            if(stmt.value != nullptr)// 3.b
                V = stmt.value;
            has_label = stmt.target == ast->label() || stmt.target == u"";
            if(stmt.type != Completion::CONTINUE || !has_label)
            {
                if(stmt.type == Completion::BREAK && has_label)
                {
                    RuntimeContext::TopContext()->ExitIteration();
                    return Completion(Completion::NORMAL, V, u"");
                }
                if(stmt.IsAbruptCompletion())
                {
                    RuntimeContext::TopContext()->ExitIteration();
                    return stmt;
                }
            }

            expr_ref = EvalExpression(e, loop_stmt->expr());
            if(!e->IsOk())
                goto error;
            val = GetValue(e, expr_ref);
            if(!e->IsOk())
                goto error;
            if(!ToBoolean(val))
                break;
        }
        RuntimeContext::TopContext()->ExitIteration();
        return Completion(Completion::NORMAL, V, u"");
    error:
        RuntimeContext::TopContext()->ExitIteration();
        return Completion(Completion::THROW, new ErrorObject(e), u"");
    }

    // 12.6.2 The while Statement
    Completion EvalWhileStatement(AST* ast)
    {
        assert(ast->type() == AST::AST_STMT_WHILE);
        Error* e = Error::Ok();
        RuntimeContext::TopContext()->EnterIteration();
        WhileOrWith* loop_stmt = static_cast<WhileOrWith*>(ast);
        JSValue* V = nullptr;
        JSValue* expr_ref;
        JSValue* val;
        Completion stmt;
        bool has_label;
        while(true)
        {
            expr_ref = EvalExpression(e, loop_stmt->expr());
            if(!e->IsOk())
                goto error;
            val = GetValue(e, expr_ref);
            if(!e->IsOk())
                goto error;
            if(!ToBoolean(val))
                break;

            stmt = EvalStatement(loop_stmt->stmt());
            if(stmt.value != nullptr)// 3.b
                V = stmt.value;
            has_label = stmt.target == ast->label() || stmt.target == u"";
            if(stmt.type != Completion::CONTINUE || !has_label)
            {
                if(stmt.type == Completion::BREAK && has_label)
                {
                    RuntimeContext::TopContext()->ExitIteration();
                    return Completion(Completion::NORMAL, V, u"");
                }
                if(stmt.IsAbruptCompletion())
                {
                    RuntimeContext::TopContext()->ExitIteration();
                    return stmt;
                }
            }
        }
        RuntimeContext::TopContext()->ExitIteration();
        return Completion(Completion::NORMAL, V, u"");
    error:
        RuntimeContext::TopContext()->ExitIteration();
        return Completion(Completion::THROW, new ErrorObject(e), u"");
    }

    // 12.6.3 The for Statement
    Completion EvalForStatement(AST* ast)
    {
        assert(ast->type() == AST::AST_STMT_FOR);
        Error* e = Error::Ok();
        RuntimeContext::TopContext()->EnterIteration();
        For* for_stmt = static_cast<For*>(ast);
        JSValue* V = nullptr;
        Completion stmt;
        bool has_label;
        for(auto expr : for_stmt->expr0s())
        {
            if(expr->type() == AST::AST_STMT_VAR_DECL)
            {
                EvalVarDecl(e, expr);
                if(!e->IsOk())
                    goto error;
            }
            else
            {
                JSValue* expr_ref = EvalExpression(e, expr);
                if(!e->IsOk())
                    goto error;
                GetValue(e, expr_ref);
                if(!e->IsOk())
                    goto error;
            }
        }
        while(true)
        {
            if(for_stmt->expr1() != nullptr)
            {
                JSValue* test_expr_ref = EvalExpression(e, for_stmt->expr1());
                if(!e->IsOk())
                    goto error;
                JSValue* test_value = GetValue(e, test_expr_ref);
                if(!e->IsOk())
                    goto error;
                if(!ToBoolean(test_value))
                    break;
            }

            stmt = EvalStatement(for_stmt->statement());
            if(stmt.value != nullptr)// 3.b
                V = stmt.value;
            has_label = stmt.target == ast->label() || stmt.target == u"";
            if(stmt.type != Completion::CONTINUE || !has_label)
            {
                if(stmt.type == Completion::BREAK && has_label)
                {
                    RuntimeContext::TopContext()->ExitIteration();
                    return Completion(Completion::NORMAL, V, u"");
                }
                if(stmt.IsAbruptCompletion())
                {
                    RuntimeContext::TopContext()->ExitIteration();
                    return stmt;
                }
            }

            if(for_stmt->expr2() != nullptr)
            {
                JSValue* inc_expr_ref = EvalExpression(e, for_stmt->expr2());
                if(!e->IsOk())
                    goto error;
                GetValue(e, inc_expr_ref);
                if(!e->IsOk())
                    goto error;
            }
        }
        RuntimeContext::TopContext()->ExitIteration();
        return Completion(Completion::NORMAL, V, u"");
    error:
        RuntimeContext::TopContext()->ExitIteration();
        return Completion(Completion::THROW, new ErrorObject(e), u"");
    }

    // 12.6.4 The for-in Statement
    Completion EvalForInStatement(AST* ast)
    {
        assert(ast->type() == AST::AST_STMT_FOR_IN);
        Error* e = Error::Ok();
        RuntimeContext::TopContext()->EnterIteration();
        ForIn* for_in_stmt = static_cast<ForIn*>(ast);
        JSObject* obj;
        JSValue* expr_ref;
        JSValue* expr_val;
        Completion stmt;
        bool has_label;
        JSValue* V = nullptr;
        if(for_in_stmt->expr0()->type() == AST::AST_STMT_VAR_DECL)
        {
            VarDecl* decl = static_cast<VarDecl*>(for_in_stmt->expr0());
            std::u16string var_name = EvalVarDecl(e, decl);
            if(!e->IsOk())
                goto error;
            expr_ref = EvalExpression(e, for_in_stmt->expr1());
            if(!e->IsOk())
                goto error;
            expr_val = GetValue(e, expr_ref);
            if(!e->IsOk())
                goto error;
            if(expr_val->IsUndefined() || expr_val->IsNull())
            {
                RuntimeContext::TopContext()->ExitIteration();
                return Completion(Completion::NORMAL, nullptr, u"");
            }
            obj = ToObject(e, expr_val);
            if(!e->IsOk())
                goto error;

            for(auto pair : obj->AllEnumerableProperties())
            {
                String* P = new String(pair.first);
                Reference* var_ref = IdentifierResolution(var_name);
                PutValue(e, var_ref, P);
                if(!e->IsOk())
                    goto error;

                stmt = EvalStatement(for_in_stmt->statement());
                if(stmt.value != nullptr)
                    V = stmt.value;
                has_label = stmt.target == ast->label() || stmt.target == u"";
                if(stmt.type != Completion::CONTINUE || !has_label)
                {
                    if(stmt.type == Completion::BREAK && has_label)
                    {
                        RuntimeContext::TopContext()->ExitIteration();
                        return Completion(Completion::NORMAL, V, u"");
                    }
                    if(stmt.IsAbruptCompletion())
                    {
                        RuntimeContext::TopContext()->ExitIteration();
                        return stmt;
                    }
                }
            }
        }
        else
        {
            expr_ref = EvalExpression(e, for_in_stmt->expr1());
            if(!e->IsOk())
                goto error;
            expr_ref = EvalExpression(e, for_in_stmt->expr1());
            if(!e->IsOk())
                goto error;
            expr_val = GetValue(e, expr_ref);
            if(!e->IsOk())
                goto error;
            if(expr_val->IsUndefined() || expr_val->IsNull())
            {
                RuntimeContext::TopContext()->ExitIteration();
                return Completion(Completion::NORMAL, nullptr, u"");
            }
            obj = ToObject(e, expr_val);
            for(auto pair : obj->AllEnumerableProperties())
            {
                String* P = new String(pair.first);
                JSValue* lhs_ref = EvalExpression(e, for_in_stmt->expr0());
                if(!e->IsOk())
                    goto error;
                PutValue(e, lhs_ref, P);
                if(!e->IsOk())
                    goto error;

                stmt = EvalStatement(for_in_stmt->statement());
                if(stmt.value != nullptr)
                    V = stmt.value;
                has_label = stmt.target == ast->label() || stmt.target == u"";
                if(stmt.type != Completion::CONTINUE || !has_label)
                {
                    if(stmt.type == Completion::BREAK && has_label)
                    {
                        RuntimeContext::TopContext()->ExitIteration();
                        return Completion(Completion::NORMAL, V, u"");
                    }
                    if(stmt.IsAbruptCompletion())
                    {
                        RuntimeContext::TopContext()->ExitIteration();
                        return stmt;
                    }
                }
            }
        }
        RuntimeContext::TopContext()->ExitIteration();
        return Completion(Completion::NORMAL, V, u"");
    error:
        RuntimeContext::TopContext()->ExitIteration();
        return Completion(Completion::THROW, new ErrorObject(e), u"");
    }

    Completion EvalContinueStatement(AST* ast)
    {
        assert(ast->type() == AST::AST_STMT_CONTINUE);
        Error* e = Error::Ok();
        if(!RuntimeContext::TopContext()->InIteration())
        {
            *e = *Error::SyntaxError();
            return Completion(Completion::THROW, new ErrorObject(e), u"");
        }
        ContinueOrBreak* stmt = static_cast<ContinueOrBreak*>(ast);
        return Completion(Completion::CONTINUE, nullptr, stmt->ident());
    }

    Completion EvalBreakStatement(AST* ast)
    {
        assert(ast->type() == AST::AST_STMT_BREAK);
        Error* e = Error::Ok();
        if(!RuntimeContext::TopContext()->InIteration())
        {
            *e = *Error::SyntaxError();
            return Completion(Completion::THROW, new ErrorObject(e), u"");
        }
        ContinueOrBreak* stmt = static_cast<ContinueOrBreak*>(ast);
        return Completion(Completion::BREAK, nullptr, stmt->ident());
    }

    Completion EvalReturnStatement(AST* ast)
    {
        assert(ast->type() == AST::AST_STMT_RETURN);
        Error* e = Error::Ok();
        Return* return_stmt = static_cast<Return*>(ast);
        if(return_stmt->expr() == nullptr)
        {
            return Completion(Completion::RETURN, Undefined::Instance(), u"");
        }
        auto exp_ref = EvalExpression(e, return_stmt->expr());
        return Completion(Completion::RETURN, GetValue(e, exp_ref), u"");
    }

    Completion EvalLabelledStatement(AST* ast)
    {
        assert(ast->type() == AST::AST_STMT_LABEL);
        LabelledStmt* label_stmt = static_cast<LabelledStmt*>(ast);
        label_stmt->statement()->SetLabel(label_stmt->label());
        Completion R = EvalStatement(label_stmt->statement());
        if(R.type == Completion::BREAK && R.target == label_stmt->label())
        {
            return Completion(Completion::NORMAL, R.value, u"");
        }
        return R;
    }

    // 12.10 The with Statement
    Completion EvalWithStatement(AST* ast)
    {
        assert(ast->type() == AST::AST_STMT_WITH);
        if(RuntimeContext::TopContext()->strict())
        {
            return Completion(Completion::THROW,
                              new ErrorObject(Error::SyntaxError(u"cannot have with statement in strict mode")), u"");
        }
        Error* e = Error::Ok();
        WhileOrWith* with_stmt = static_cast<WhileOrWith*>(ast);
        JSValue* ref = EvalExpression(e, with_stmt->expr());
        if(!e->IsOk())
            return Completion(Completion::THROW, new ErrorObject(e), u"");
        JSValue* val = GetValue(e, ref);
        if(!e->IsOk())
            return Completion(Completion::THROW, new ErrorObject(e), u"");
        JSObject* obj = ToObject(e, val);
        if(!e->IsOk())
            return Completion(Completion::THROW, new ErrorObject(e), u"");
        LexicalEnvironment* old_env = RuntimeContext::TopLexicalEnv();
        LexicalEnvironment* new_env = LexicalEnvironment::NewObjectEnvironment(obj, old_env, true);
        RuntimeContext::TopContext()->SetLexicalEnv(new_env);
        Completion C = EvalStatement(with_stmt->stmt());
        RuntimeContext::TopContext()->SetLexicalEnv(old_env);
        return C;
    }

    JSValue* EvalCaseClause(Error* e, Switch::CaseClause C)
    {
        JSValue* exp_ref = EvalExpression(e, C.expr);
        if(!e->IsOk())
            return nullptr;
        return GetValue(e, exp_ref);
    }

    Completion EvalCaseBlock(Switch* switch_stmt, JSValue* input)
    {
        Error* e = Error::Ok();
        JSValue* V = nullptr;
        bool found = false;
        for(auto C : switch_stmt->before_default_case_clauses())
        {
            if(!found)
            {// 5.a
                JSValue* clause_selector = EvalCaseClause(e, C);
                bool b = StrictEqual(e, input, clause_selector);
                if(!e->IsOk())
                    return Completion(Completion::THROW, new ErrorObject(e), u"");
                if(b)
                    found = true;
            }
            if(found)
            {// 5.b
                Completion R = EvalStatementList(C.stmts);
                if(R.value != nullptr)
                    V = R.value;
                if(R.IsAbruptCompletion())
                    return Completion(R.type, V, R.target);
            }
        }
        bool found_in_b = false;
        size_t i;
        for(i = 0; !found_in_b && i < switch_stmt->after_default_case_clauses().size(); i++)
        {
            auto C = switch_stmt->after_default_case_clauses()[i];
            JSValue* clause_selector = EvalCaseClause(e, C);
            bool b = StrictEqual(e, input, clause_selector);
            if(!e->IsOk())
                return Completion(Completion::THROW, new ErrorObject(e), u"");
            if(b)
            {
                found_in_b = true;
                Completion R = EvalStatementList(C.stmts);
                if(R.value != nullptr)
                    V = R.value;
                if(R.IsAbruptCompletion())
                    return Completion(R.type, V, R.target);
            }
        }
        if(!found_in_b && switch_stmt->has_default_clause())
        {// 8
            Completion R = EvalStatementList(switch_stmt->default_clause().stmts);
            if(R.value != nullptr)
                V = R.value;
            if(R.IsAbruptCompletion())
                return Completion(R.type, V, R.target);
        }
        for(i = 0; i < switch_stmt->after_default_case_clauses().size(); i++)
        {
            auto C = switch_stmt->after_default_case_clauses()[i];
            JSValue* clause_selector = EvalCaseClause(e, C);
            Completion R = EvalStatementList(C.stmts);
            if(R.value != nullptr)
                V = R.value;
            if(R.IsAbruptCompletion())
                return Completion(R.type, V, R.target);
        }
        return Completion(Completion::NORMAL, V, u"");
    }

    // 12.11 The switch Statement
    Completion EvalSwitchStatement(AST* ast)
    {
        assert(ast->type() == AST::AST_STMT_SWITCH);
        Error* e = Error::Ok();
        Switch* switch_stmt = static_cast<Switch*>(ast);
        JSValue* expr_ref = EvalExpression(e, switch_stmt->expr());
        if(!e->IsOk())
            return Completion(Completion::THROW, new ErrorObject(e), u"");
        Completion R = EvalCaseBlock(switch_stmt, expr_ref);
        if(R.IsThrow())
            return R;
        bool has_label = ast->label() == R.target;
        if(R.type == Completion::BREAK && has_label)
            return Completion(Completion::NORMAL, R.value, u"");
        return R;
    }

    // 12.13 The throw Statement
    Completion EvalThrowStatement(AST* ast)
    {
        assert(ast->type() == AST::AST_STMT_THROW);
        Error* e = Error::Ok();
        Throw* throw_stmt = static_cast<Throw*>(ast);
        JSValue* exp_ref = EvalExpression(e, throw_stmt->expr());
        if(!e->Ok())
            return Completion(Completion::THROW, new ErrorObject(e), u"");
        JSValue* val = GetValue(e, exp_ref);
        if(!e->Ok())
            return Completion(Completion::THROW, new ErrorObject(e), u"");
        return Completion(Completion::THROW, val, u"");
    }

    Completion EvalCatch(Try* try_stmt, Completion C)
    {
        // NOTE(zhuzilin) Don't gc these two env, during this function.
        Error* e = Error::Ok();
        LexicalEnvironment* old_env = RuntimeContext::TopLexicalEnv();
        LexicalEnvironment* catch_env = LexicalEnvironment::NewDeclarativeEnvironment(old_env);
        catch_env->env_rec()->CreateMutableBinding(e, try_stmt->catch_ident(), false);// 4
        if(!e->IsOk())
        {
            return Completion(Completion::THROW, new ErrorObject(e), u"");
        }
        // NOTE(zhuzilin) The spec say to send C instead of C.value.
        // However, I think it should be send C.value...
        catch_env->env_rec()->SetMutableBinding(e, try_stmt->catch_ident(), C.value, false);// 5
        if(!e->IsOk())
        {
            return Completion(Completion::THROW, new ErrorObject(e), u"");
        }
        RuntimeContext::TopContext()->SetLexicalEnv(catch_env);
        Completion B = EvalBlockStatement(try_stmt->catch_block());
        RuntimeContext::TopContext()->SetLexicalEnv(old_env);
        return B;
    }

    Completion EvalTryStatement(AST* ast)
    {
        assert(ast->type() == AST::AST_STMT_TRY);
        Error* e = Error::Ok();
        Try* try_stmt = static_cast<Try*>(ast);
        Completion B = EvalBlockStatement(try_stmt->try_block());
        if(try_stmt->finally_block() == nullptr)
        {// try Block Catch
            if(B.type != Completion::THROW)
                return B;
            return EvalCatch(try_stmt, B);
        }
        else if(try_stmt->catch_block() == nullptr)
        {// try Block Finally
            Completion F = EvalBlockStatement(try_stmt->finally_block());
            if(F.type == Completion::NORMAL)
                return B;
            return F;
        }
        else
        {// try Block Catch Finally
            Completion C = B;
            if(B.type == Completion::THROW)
            {
                C = EvalCatch(try_stmt, B);
            }
            Completion F = EvalBlockStatement(try_stmt->finally_block());
            if(F.type == Completion::NORMAL)
                return C;
            return F;
        }
    }

    Completion EvalExpressionStatement(AST* ast)
    {
        Error* e = Error::Ok();
        JSValue* val = EvalExpression(e, ast);
        if(!e->IsOk())
            return Completion(Completion::THROW, new ErrorObject(e), u"");
        return Completion(Completion::NORMAL, val, u"");
    }

    JSValue* EvalExpression(Error* e, AST* ast)
    {
        assert(ast->type() <= AST::AST_EXPR || ast->type() == AST::AST_FUNC);
        JSValue* val;
        switch(ast->type())
        {
            case AST::AST_EXPR_THIS:
            case AST::AST_EXPR_IDENT:
            case AST::AST_EXPR_NULL:
            case AST::AST_EXPR_BOOL:
            case AST::AST_EXPR_NUMBER:
            case AST::AST_EXPR_STRING:
            case AST::AST_EXPR_OBJ:
            case AST::AST_EXPR_ARRAY:
            case AST::AST_EXPR_PAREN:
                val = EvalPrimaryExpression(e, ast);
                break;
            case AST::AST_EXPR_UNARY:
                val = EvalUnaryOperator(e, ast);
                break;
            case AST::AST_EXPR_BINARY:
                val = EvalBinaryExpression(e, ast);
                break;
            case AST::AST_EXPR_TRIPLE:
                val = EvalTripleConditionExpression(e, ast);
                break;
            case AST::AST_EXPR_LHS:
                val = EvalLeftHandSideExpression(e, ast);
                break;
            case AST::AST_EXPR:
                val = EvalExpressionList(e, ast);
            case AST::AST_FUNC:
                val = EvalFunction(e, ast);
                break;
            default:
                assert(false);
        }
        if(!e->IsOk())
            return nullptr;
        return val;
    }

    JSValue* EvalPrimaryExpression(Error* e, AST* ast)
    {
        JSValue* val;
        switch(ast->type())
        {
            case AST::AST_EXPR_THIS:
                val = RuntimeContext::TopContext()->this_binding();
                break;
            case AST::AST_EXPR_IDENT:
                val = EvalIdentifier(ast);
                break;
            case AST::AST_EXPR_NULL:
                val = Null::Instance();
                break;
            case AST::AST_EXPR_BOOL:
                val = ast->source() == u"true" ? Bool::True() : Bool::False();
                break;
            case AST::AST_EXPR_NUMBER:
                val = EvalNumber(ast);
                break;
            case AST::AST_EXPR_STRING:
                val = EvalString(ast);
                break;
            case AST::AST_EXPR_OBJ:
                val = EvalObject(e, ast);
                break;
            case AST::AST_EXPR_ARRAY:
                val = EvalArray(e, ast);
                break;
            case AST::AST_EXPR_PAREN:
                val = EvalExpression(e, static_cast<Paren*>(ast)->expr());
                break;
            default:
                std::cout << "Not primary expression, type " << ast->type() << std::endl;
                assert(false);
        }
        return val;
    }

    Reference* IdentifierResolution(const std::u16string& name)
    {
        // 10.3.1 Identifier Resolution
        LexicalEnvironment* env = RuntimeContext::TopLexicalEnv();
        bool strict = RuntimeContext::TopContext()->strict();
        return env->GetIdentifierReference(name, strict);
    }

    Reference* EvalIdentifier(AST* ast)
    {
        assert(ast->type() == AST::AST_EXPR_IDENT);
        return IdentifierResolution(ast->source());
    }

    Number* EvalNumber(const std::u16string& source)
    {
        double val = 0;
        double frac = 1;
        size_t pos = 0;
        bool dot = false;
        while(pos < source.size())
        {
            char16_t c = source[pos];
            switch(c)
            {
                case u'.':
                    dot = true;
                    break;
                case u'e':
                case u'E':
                {
                    double exp = 0;
                    bool sign = true;
                    pos++;// skip e/E
                    if(c == u'-')
                    {
                        sign = false;
                        pos++;// skip -
                    }
                    else if(c == u'+')
                    {
                        sign = true;
                        pos++;// skip +;
                    }
                    while(pos < source.size())
                    {
                        exp *= 10;
                        exp += character::Digit(c);
                    }
                    if(!sign)
                        exp = -exp;
                    return new Number(val * pow(10.0, exp));
                }
                case u'x':
                case u'X':
                {
                    assert(val == 0);
                    pos++;
                    while(pos < source.size())
                    {
                        c = source[pos];
                        val *= 16;
                        val += character::Digit(c);
                        pos++;
                    }
                    return new Number(val);
                }
                default:
                    if(dot)
                    {
                        frac /= 10;
                        val += character::Digit(c) * frac;
                    }
                    else
                    {
                        val *= 10;
                        val += character::Digit(c);
                    }
            }
            pos++;
        }
        return new Number(val);
    }

    Number* EvalNumber(AST* ast)
    {
        assert(ast->type() == AST::AST_EXPR_NUMBER);
        auto source = ast->source();
        return EvalNumber(source);
    }
    
    String* EvalString(const std::u16string& isource)
    {
        auto source = isource.substr(1, isource.size() - 2);
        size_t pos = 0;
        std::vector<std::u16string> vals;
        while(pos < source.size())
        {
            char16_t c = source[pos];
            switch(c)
            {
                case u'\\':
                {
                    pos++;
                    c = source[pos];
                    switch(c)
                    {
                        case u'b':
                            pos++;
                            vals.emplace_back(u"\b");
                            break;
                        case u't':
                            pos++;
                            vals.emplace_back(u"\t");
                            break;
                        case u'n':
                            pos++;
                            vals.emplace_back(u"\n");
                            break;
                        case u'v':
                            pos++;
                            vals.emplace_back(u"\v");
                            break;
                        case u'f':
                            pos++;
                            vals.emplace_back(u"\f");
                            break;
                        case u'r':
                            pos++;
                            vals.emplace_back(u"\r");
                            break;
                        case u'x':
                        {
                            pos++;// skip 'x'
                            char16_t hex = 0;
                            for(size_t i = 0; i < 2; i++)
                            {
                                hex *= 16;
                                hex += character::Digit(source[pos]);
                                pos++;
                            }
                            vals.emplace_back(std::u16string(1, hex));
                            break;
                        }
                        case u'u':
                        {
                            pos++;// skip 'u'
                            char16_t hex = 0;
                            for(size_t i = 0; i < 4; i++)
                            {
                                hex *= 16;
                                hex += character::Digit(source[pos]);
                                pos++;
                            }
                            vals.emplace_back(std::u16string(1, hex));
                            break;
                        }
                        default:
                            c = source[pos];
                            if(character::IsLineTerminator(c))
                            {
                                pos++;
                                continue;
                            }
                            pos++;
                            vals.emplace_back(std::u16string(1, c));
                    }
                    break;
                }
                default:
                {
                    size_t start = pos;
                    while(true)
                    {
                        if(pos == source.size() || source[pos] == u'\\')
                            break;
                        pos++;
                    }
                    size_t end = pos;
                    auto substr = source.substr(start, end - start);
                    vals.emplace_back(std::u16string(substr.data(), substr.size()));
                }
            }
        }
        if(vals.size() == 0)
        {
            return String::Empty();
        }
        else if(vals.size() == 1)
        {
            return new String(vals[0]);
        }
        return new String(StrCat(vals));
    }

    String* EvalString(AST* ast)
    {
        assert(ast->type() == AST::AST_EXPR_STRING);
        auto source = ast->source();
        return EvalString(source);
    }

    std::u16string EvalPropertyName(Error* e, Token token)
    {
        switch(token.type())
        {
            case Token::TK_IDENT:
            case Token::TK_KEYWORD:
            case Token::TK_FUTURE:
                return token.source();
            case Token::TK_NUMBER:
                return ToString(e, EvalNumber(token.source()));
            case Token::TK_STRING:
                return ToString(e, EvalString(token.source()));
            default:
                assert(false);
        }
    }

    Object* EvalObject(Error* e, AST* ast)
    {
        assert(ast->type() == AST::AST_EXPR_OBJ);
        ObjectLiteral* obj_ast = static_cast<ObjectLiteral*>(ast);
        bool strict = RuntimeContext::TopContext()->strict();
        Object* obj = new Object();
        // PropertyName : AssignmentExpression
        for(auto property : obj_ast->properties())
        {
            std::u16string prop_name = EvalPropertyName(e, property.key);
            PropertyDescriptor* desc = new PropertyDescriptor();
            switch(property.type)
            {
                case ObjectLiteral::Property::NORMAL:
                {
                    JSValue* expr_value = EvalAssignmentExpression(e, property.value);
                    JSValue* prop_value = GetValue(e, expr_value);
                    desc->SetDataDescriptor(prop_value, true, true, true);
                    break;
                }
                default:
                {
                    assert(property.value->type() == AST::AST_FUNC);
                    Function* func_ast = static_cast<Function*>(property.value);
                    bool strict_func = static_cast<ProgramOrFunctionBody*>(func_ast->body())->strict();
                    if(strict || strict_func)
                    {
                        for(auto name : func_ast->params())
                        {
                            if(name == u"eval" || name == u"arguments")
                            {
                                *e = *Error::SyntaxError();
                                return nullptr;
                            }
                        }
                    }
                    FunctionObject* closure
                    = new FunctionObject(func_ast->params(), func_ast->body(), RuntimeContext::TopLexicalEnv());
                    if(property.type == ObjectLiteral::Property::GET)
                    {
                        desc->SetGet(closure);
                    }
                    else
                    {
                        desc->SetSet(closure);
                    }
                    desc->SetEnumerable(true);
                    desc->SetConfigurable(true);
                    break;
                }
            }
            auto previous = obj->GetOwnProperty(prop_name);// 3
            if(!previous->IsUndefined())
            {// 4
                PropertyDescriptor* previous_desc = static_cast<PropertyDescriptor*>(previous);
                if(strict && previous_desc->IsDataDescriptor() && desc->IsDataDescriptor())
                {// 4.a
                    *e = *Error::SyntaxError();
                    return nullptr;
                }
                if(previous_desc->IsDataDescriptor() && desc->IsAccessorDescriptor() ||// 4.b
                   previous_desc->IsAccessorDescriptor() && desc->IsDataDescriptor())
                {// 4.c
                    *e = *Error::SyntaxError();
                    return nullptr;
                }
                if(previous_desc->IsAccessorDescriptor() && desc->IsAccessorDescriptor() &&// 4.d
                   (previous_desc->HasGet() && desc->HasGet() || previous_desc->HasSet() && desc->HasSet()))
                {
                    *e = *Error::SyntaxError();
                    return nullptr;
                }
            }
            obj->DefineOwnProperty(e, prop_name, desc, false);
        }
        return obj;
    }

    ArrayObject* EvalArray(Error* e, AST* ast)
    {
        assert(ast->type() == AST::AST_EXPR_ARRAY);
        ArrayLiteral* array_ast = static_cast<ArrayLiteral*>(ast);

        ArrayObject* arr = new ArrayObject(array_ast->length());
        for(auto pair : array_ast->elements())
        {
            JSValue* init_result = EvalAssignmentExpression(e, pair.second);
            if(!e->IsOk())
                return nullptr;
            arr->AddValueProperty(NumberToString(pair.first), init_result, true, true, true);
        }
        return arr;
    }

    JSValue* EvalAssignmentExpression(Error* e, AST* ast)
    {
        return EvalExpression(e, ast);
    }

    JSValue* EvalUnaryOperator(Error* e, AST* ast)
    {
        assert(ast->type() == AST::AST_EXPR_UNARY);
        Unary* u = static_cast<Unary*>(ast);

        JSValue* expr = EvalExpression(e, u->node());
        if(!e->IsOk())
            return nullptr;
        std::u16string op = u->op().source();

        if(op == u"++" || op == u"--")
        {// a++, ++a, a--, --a
            if(expr->IsReference())
            {
                Reference* ref = static_cast<Reference*>(expr);
                if(ref->IsStrictReference() && ref->GetBase()->IsEnvironmentRecord()
                   && (ref->GetReferencedName() == u"eval" || ref->GetReferencedName() == u"arguments"))
                {
                    *e = *Error::SyntaxError();
                    return nullptr;
                }
            }
            JSValue* old_val = GetValue(e, expr);
            if(!e->IsOk())
                return nullptr;
            double num = ToNumber(e, old_val);
            if(!e->IsOk())
                return nullptr;
            JSValue* new_value;
            if(op == u"++")
            {
                new_value = new Number(num + 1);
            }
            else
            {
                new_value = new Number(num - 1);
            }
            PutValue(e, expr, new_value);
            if(!e->IsOk())
                return nullptr;
            if(u->prefix())
            {
                return new_value;
            }
            else
            {
                return old_val;
            }
        }
        else if(op == u"delete")
        {// 11.4.1 The delete Operator
            if(!expr->IsReference())// 2
                return Bool::True();
            Reference* ref = static_cast<Reference*>(expr);
            if(ref->IsUnresolvableReference())
            {// 3
                if(ref->IsStrictReference())
                {
                    *e = *Error::SyntaxError();
                    return Bool::False();
                }
                return Bool::True();
            }
            if(ref->IsPropertyReference())
            {// 4
                JSObject* obj = ToObject(e, ref->GetBase());
                if(!e->IsOk())
                    return nullptr;
                return Bool::Wrap(obj->Delete(e, ref->GetReferencedName(), ref->IsStrictReference()));
            }
            else
            {
                if(ref->IsStrictReference())
                {
                    *e = *Error::SyntaxError();
                    return Bool::False();
                }
                EnvironmentRecord* bindings = static_cast<EnvironmentRecord*>(ref->GetBase());
                return Bool::Wrap(bindings->DeleteBinding(e, ref->GetReferencedName()));
            }
        }
        else if(op == u"typeof")
        {
            if(expr->IsReference())
            {
                Reference* ref = static_cast<Reference*>(expr);
                if(ref->IsUnresolvableReference())
                    return String::Undefined();
            }
            JSValue* val = GetValue(e, expr);
            if(!e->IsOk())
                return nullptr;
            switch(val->type())
            {
                case JSValue::JS_UNDEFINED:
                    return String::Undefined();
                case JSValue::JS_NULL:
                    return new String(u"object");
                case JSValue::JS_NUMBER:
                    return new String(u"number");
                case JSValue::JS_STRING:
                    return new String(u"string");
                default:
                    if(val->IsCallable())
                        return new String(u"function");
                    return new String(u"object");
            }
        }
        else
        {// +, -, ~, !, void
            JSValue* val = GetValue(e, expr);
            if(!e->IsOk())
                return nullptr;

            if(op == u"+")
            {
                double num = ToNumber(e, val);
                if(!e->IsOk())
                    return nullptr;
                return new Number(num);
            }
            else if(op == u"-")
            {
                double num = ToNumber(e, val);
                if(!e->IsOk())
                    return nullptr;
                if(isnan(num))
                    return Number::NaN();
                return new Number(-num);
            }
            else if(op == u"~")
            {
                int32_t num = ToInt32(e, val);
                if(!e->IsOk())
                    return nullptr;
                return new Number(~num);
            }
            else if(op == u"!")
            {
                bool b = ToBoolean(val);
                return Bool::Wrap(!b);
            }
            else if(op == u"void")
            {
                return Undefined::Instance();
            }
        }
        assert(false);
    }

    JSValue* EvalBinaryExpression(Error* e, AST* ast)
    {
        assert(ast->type() == AST::AST_EXPR_BINARY);
        Binary* b = static_cast<Binary*>(ast);
        return EvalBinaryExpression(e, b->op(), b->lhs(), b->rhs());
    }

    JSValue* EvalBinaryExpression(Error* e, const std::u16string& op, AST* lhs, AST* rhs)
    {
        // && and || are different, as there are not &&= and ||=
        if(op == u"&&" || op == u"||")
        {
            return EvalLogicalOperator(e, op, lhs, rhs);
        }
        if(op == u"=" || op == u"*=" || op == u"/=" || op == u"%=" || op == u"+=" || op == u"-=" || op == u"<<="
           || op == u">>=" || op == u">>>=" || op == u"&=" || op == u"^=" || op == u"|=")
        {
            JSValue* lref = EvalLeftHandSideExpression(e, lhs);
            if(!e->IsOk())
                return nullptr;
            // TODO(zhuzilin) The compound assignment should do lval = GetValue(lref)
            // here. Check if changing the order will have any influence.
            JSValue* rref = EvalExpression(e, rhs);
            if(!e->IsOk())
                return nullptr;
            JSValue* rval = GetValue(e, rref);
            if(!e->IsOk())
                return nullptr;
            if(op == u"=")
            {
                return EvalSimpleAssignment(e, lref, rval);
            }
            else
            {
                return EvalCompoundAssignment(e, op, lref, rval);
            }
        }

        JSValue* lref = EvalExpression(e, lhs);
        if(!e->IsOk())
            return nullptr;
        JSValue* lval = GetValue(e, lref);
        if(!e->IsOk())
            return nullptr;
        JSValue* rref = EvalExpression(e, rhs);
        if(!e->IsOk())
            return nullptr;
        JSValue* rval = GetValue(e, rref);
        if(!e->IsOk())
            return nullptr;
        return EvalBinaryExpression(e, op, lval, rval);
    }

    JSValue* EvalBinaryExpression(Error* e, const std::u16string& op, JSValue* lval, JSValue* rval)
    {
        if(op == u"*" || op == u"/" || op == u"%" || op == u"-")
        {
            return EvalArithmeticOperator(e, op, lval, rval);
        }
        else if(op == u"+")
        {
            return EvalAddOperator(e, lval, rval);
        }
        else if(op == u"<<" || op == u">>" || op == u">>>")
        {
            return EvalBitwiseShiftOperator(e, op, lval, rval);
        }
        else if(op == u"<" || op == u">" || op == u"<=" || op == u">=" || op == u"instanceof" || op == u"in")
        {
            return EvalRelationalOperator(e, op, lval, rval);
        }
        else if(op == u"==" || op == u"!=" || op == u"===" || op == u"!==")
        {
            return EvalEqualityOperator(e, op, lval, rval);
        }
        else if(op == u"&" || op == u"^" || op == u"|")
        {
            return EvalBitwiseOperator(e, op, lval, rval);
        }
        assert(false);
    }

    // 11.5 Multiplicative Operators
    JSValue* EvalArithmeticOperator(Error* e, const std::u16string& op, JSValue* lval, JSValue* rval)
    {
        double lnum = ToNumber(e, lval);
        if(!e->IsOk())
            return nullptr;
        double rnum = ToNumber(e, rval);
        if(!e->IsOk())
            return nullptr;
        switch(op[0])
        {
            case u'*':
                return new Number(lnum * rnum);
            case u'/':
                return new Number(lnum / rnum);
            case u'%':
                return new Number(fmod(lnum, rnum));
            case u'-':
                return new Number(lnum - rnum);
            default:
                assert(false);
        }
    }

    // 11.6 Additive Operators
    JSValue* EvalAddOperator(Error* e, JSValue* lval, JSValue* rval)
    {
        JSValue* lprim = ToPrimitive(e, lval, u"");
        if(!e->IsOk())
            return nullptr;
        JSValue* rprim = ToPrimitive(e, rval, u"");
        if(!e->IsOk())
            return nullptr;

        if(lprim->IsString() || rprim->IsString())
        {
            std::u16string lstr = ToString(e, lprim);
            if(!e->IsOk())
                return nullptr;
            std::u16string rstr = ToString(e, rprim);
            if(!e->IsOk())
                return nullptr;
            return new String(lstr + rstr);
        }

        double lnum = ToNumber(e, lprim);
        if(!e->IsOk())
            return nullptr;
        double rnum = ToNumber(e, rprim);
        if(!e->IsOk())
            return nullptr;
        return new Number(lnum + rnum);
    }

    // 11.7 Bitwise Shift Operators
    JSValue* EvalBitwiseShiftOperator(Error* e, const std::u16string& op, JSValue* lval, JSValue* rval)
    {
        int32_t lnum = ToInt32(e, lval);
        if(!e->IsOk())
            return nullptr;
        uint32_t rnum = ToUint32(e, rval);
        if(!e->IsOk())
            return nullptr;
        uint32_t shift_count = rnum & 0x1F;
        if(op == u"<<")
        {
            return new Number(lnum << shift_count);
        }
        else if(op == u">>")
        {
            return new Number(lnum >> shift_count);
        }
        else if(op == u">>>")
        {
            uint32_t lnum = ToUint32(e, lval);
            return new Number(lnum >> rnum);
        }
        assert(false);
    }

    // 11.8 Relational Operators
    JSValue* EvalRelationalOperator(Error* e, const std::u16string& op, JSValue* lval, JSValue* rval)
    {
        if(op == u"<")
        {
            JSValue* r = LessThan(e, lval, rval);
            if(!e->IsOk())
                return nullptr;
            return r->IsUndefined() ? Bool::False() : r;
        }
        else if(op == u">")
        {
            JSValue* r = LessThan(e, rval, lval);
            if(!e->IsOk())
                return nullptr;
            return r->IsUndefined() ? Bool::False() : r;
        }
        else if(op == u"<=")
        {
            JSValue* r = LessThan(e, rval, lval);
            if(!e->IsOk())
                return nullptr;
            if(r->IsUndefined())
                return Bool::True();
            return Bool::Wrap(!static_cast<Bool*>(r)->data());
        }
        else if(op == u">=")
        {
            JSValue* r = LessThan(e, lval, rval);
            if(!e->IsOk())
                return nullptr;
            if(r->IsUndefined())
                return Bool::True();
            return Bool::Wrap(!static_cast<Bool*>(r)->data());
        }
        else if(op == u"instanceof")
        {
            if(!rval->IsObject())
            {
                *e = *Error::TypeError(u"Right-hand side of 'instanceof' is not an object");
                return nullptr;
            }
            if(!rval->IsCallable())
            {
                *e = *Error::TypeError(u"Right-hand side of 'instanceof' is not callable");
                return nullptr;
            }
            JSObject* obj = static_cast<JSObject*>(rval);
            return Bool::Wrap(obj->HasInstance(e, lval));
        }
        else if(op == u"in")
        {
            if(!rval->IsObject())
            {
                *e = *Error::TypeError(u"in called on non-object");
                return nullptr;
            }
            JSObject* obj = static_cast<JSObject*>(rval);
            return Bool::Wrap(obj->HasProperty(ToString(e, lval)));
        }
        assert(false);
    }

    // 11.9 Equality Operators
    JSValue* EvalEqualityOperator(Error* e, const std::u16string& op, JSValue* lval, JSValue* rval)
    {
        if(op == u"==")
        {
            return Bool::Wrap(Equal(e, lval, rval));
        }
        else if(op == u"!=")
        {
            return Bool::Wrap(!Equal(e, lval, rval));
        }
        else if(op == u"===")
        {
            return Bool::Wrap(StrictEqual(e, lval, rval));
        }
        else if(op == u"!==")
        {
            return Bool::Wrap(!StrictEqual(e, lval, rval));
        }
        assert(false);
    }

    // 11.10 Binary Bitwise Operators
    JSValue* EvalBitwiseOperator(Error* e, const std::u16string& op, JSValue* lval, JSValue* rval)
    {
        int32_t lnum = ToInt32(e, lval);
        if(!e->IsOk())
            return nullptr;
        int32_t rnum = ToInt32(e, rval);
        if(!e->IsOk())
            return nullptr;
        switch(op[0])
        {
            case u'&':
                return new Number(lnum & rnum);
            case u'^':
                return new Number(lnum ^ rnum);
            case u'|':
                return new Number(lnum | rnum);
            default:
                assert(false);
        }
    }

    // 11.11 Binary Logical Operators
    JSValue* EvalLogicalOperator(Error* e, const std::u16string& op, AST* lhs, AST* rhs)
    {
        JSValue* lref = EvalExpression(e, lhs);
        if(!e->IsOk())
            return nullptr;
        JSValue* lval = GetValue(e, lref);
        if(!e->IsOk())
            return nullptr;
        if(op == u"&&" && !ToBoolean(lval) || op == u"||" && ToBoolean(lval))
            return lval;
        JSValue* rref = EvalExpression(e, rhs);
        if(!e->IsOk())
            return nullptr;
        JSValue* rval = GetValue(e, rref);
        if(!e->IsOk())
            return nullptr;
        return rval;
    }

    // 11.13.1 Simple Assignment ( = )
    JSValue* EvalSimpleAssignment(Error* e, JSValue* lref, JSValue* rval)
    {
        if(lref->type() == JSValue::JS_REF)
        {
            Reference* ref = static_cast<Reference*>(lref);
            // NOTE in 11.13.1.
            // TODO(zhuzilin) not sure how to implement the type error part of the note.
            if(ref->IsStrictReference() && ref->IsUnresolvableReference())
            {
                *e = *Error::ReferenceError(ref->GetReferencedName() + u" is not defined");
                return nullptr;
            }
            if(ref->IsStrictReference() && ref->GetBase()->type() == JSValue::JS_ENV_REC
               && (ref->GetReferencedName() == u"eval" || ref->GetReferencedName() == u"arguments"))
            {
                *e = *Error::SyntaxError();
                return nullptr;
            }
        }
        PutValue(e, lref, rval);
        if(!e->IsOk())
            return nullptr;
        return rval;
    }

    // 11.13.2 Compound Assignment ( op= )
    JSValue* EvalCompoundAssignment(Error* e, const std::u16string& op, JSValue* lref, JSValue* rval)
    {
        std::u16string calc_op = op.substr(0, op.size() - 1);
        JSValue* lval = GetValue(e, lref);
        if(!e->IsOk())
            return nullptr;
        JSValue* r = EvalBinaryExpression(e, calc_op, lval, rval);
        if(!e->IsOk())
            return nullptr;
        return EvalSimpleAssignment(e, lref, r);
    }

    // 11.12 Conditional Operator ( ? : )
    JSValue* EvalTripleConditionExpression(Error* e, AST* ast)
    {
        assert(ast->type() == AST::AST_EXPR_TRIPLE);
        TripleCondition* t = static_cast<TripleCondition*>(ast);
        JSValue* lref = EvalExpression(e, t->cond());
        if(!e->IsOk())
            return nullptr;
        JSValue* lval = GetValue(e, lref);
        if(!e->IsOk())
            return nullptr;
        if(ToBoolean(lval))
        {
            JSValue* true_ref = EvalAssignmentExpression(e, t->true_expr());
            if(!e->IsOk())
                return nullptr;
            return GetValue(e, true_ref);
        }
        else
        {
            JSValue* false_ref = EvalAssignmentExpression(e, t->false_expr());
            if(!e->IsOk())
                return nullptr;
            return GetValue(e, false_ref);
        }
    }

    JSValue* EvalLeftHandSideExpression(Error* e, AST* ast)
    {
        assert(ast->type() == AST::AST_EXPR_LHS);
        LHS* lhs = static_cast<LHS*>(ast);

        ValueGuard guard;
        JSValue* base = EvalExpression(e, lhs->base());
        if(!e->IsOk())
            return nullptr;

        size_t new_count = lhs->new_count();
        for(auto pair : lhs->order())
        {
            switch(pair.second)
            {
                case LHS::PostfixType::CALL:
                {
                    auto args = lhs->args_list()[pair.first];
                    auto arg_list = EvalArgumentsList(e, args);
                    if(!e->IsOk())
                        return nullptr;
                    if(new_count > 0)
                    {
                        base = GetValue(e, base);
                        if(!e->IsOk())
                            return nullptr;
                        if(!base->IsConstructor())
                        {
                            *e = *Error::TypeError(u"base value is not a constructor");
                            return nullptr;
                        }
                        JSObject* constructor = static_cast<JSObject*>(base);
                        base = constructor->Construct(e, arg_list);
                        if(!e->IsOk())
                            return nullptr;
                        new_count--;
                    }
                    else
                    {
                        base = EvalCallExpression(e, base, arg_list);
                        if(!e->IsOk())
                            return nullptr;
                    }
                    break;
                }
                case LHS::PostfixType::INDEX:
                {
                    auto index = lhs->index_list()[pair.first];
                    base = EvalIndexExpression(e, base, index, guard);
                    if(!e->IsOk())
                        return nullptr;
                    break;
                }
                case LHS::PostfixType::PROP:
                {
                    auto prop = lhs->prop_name_list()[pair.first];
                    base = EvalIndexExpression(e, base, prop, guard);
                    if(!e->IsOk())
                        return nullptr;
                    break;
                }
                default:
                    assert(false);
            }
        }
        while(new_count > 0)
        {
            base = GetValue(e, base);
            if(!e->IsOk())
                return nullptr;
            if(!base->IsConstructor())
            {
                *e = *Error::TypeError(u"base value is not a constructor");
                return nullptr;
            }
            JSObject* constructor = static_cast<JSObject*>(base);
            base = constructor->Construct(e, {});
            if(!e->IsOk())
                return nullptr;
            new_count--;
        }
        return base;
    }

    std::vector<JSValue*> EvalArgumentsList(Error* e, Arguments* ast)
    {
        std::vector<JSValue*> arg_list;
        for(AST* ast : ast->args())
        {
            JSValue* ref = EvalExpression(e, ast);
            if(!e->IsOk())
                return {};
            JSValue* arg = GetValue(e, ref);
            if(!e->IsOk())
                return {};
            arg_list.emplace_back(arg);
        }
        return arg_list;
    }

    // 11.2.3
    JSValue* EvalCallExpression(Error* e, JSValue* ref, const std::vector<JSValue*>& arg_list)
    {
        JSValue* val = GetValue(e, ref);
        if(!e->IsOk())
            return nullptr;
        if(!val->IsObject())
        {// 4
            *e = *Error::TypeError(u"is not a function");
            return nullptr;
        }
        auto obj = static_cast<JSObject*>(val);
        if(!obj->IsCallable())
        {// 5
            *e = *Error::TypeError(u"is not a function");
            return nullptr;
        }
        JSValue* this_value;
        if(ref->IsReference())
        {
            Reference* r = static_cast<Reference*>(ref);
            JSValue* base = r->GetBase();
            if(r->IsPropertyReference())
            {
                this_value = base;
            }
            else
            {
                assert(base->IsEnvironmentRecord());
                auto env_rec = static_cast<EnvironmentRecord*>(base);
                this_value = env_rec->ImplicitThisValue();
            }
        }
        else
        {
            this_value = Undefined::Instance();
        }
        // indirect
        if(ref->IsReference() && static_cast<Reference*>(ref)->GetReferencedName() == u"eval")
        {
            DirectEvalGuard guard;
            return obj->Call(e, this_value, arg_list);
        }
        else
        {
            return obj->Call(e, this_value, arg_list);
        }
    }

    // 11.2.1 Property Accessors
    JSValue* EvalIndexExpression(Error* e, JSValue* base_ref, const std::u16string& identifier_name, ValueGuard& guard)
    {
        JSValue* base_value = GetValue(e, base_ref);
        if(!e->IsOk())
            return nullptr;
        guard.AddValue(base_value);
        base_value->CheckObjectCoercible(e);
        if(!e->IsOk())
            return nullptr;
        bool strict = RuntimeContext::TopContext()->strict();
        return new Reference(base_value, identifier_name, strict);
    }

    JSValue* EvalIndexExpression(Error* e, JSValue* base_ref, AST* expr, ValueGuard& guard)
    {
        JSValue* property_name_ref = EvalExpression(e, expr);
        if(!e->IsOk())
            return nullptr;
        JSValue* property_name_value = GetValue(e, property_name_ref);
        if(!e->IsOk())
            return nullptr;
        std::u16string property_name_str = ToString(e, property_name_value);
        if(!e->IsOk())
            return nullptr;
        return EvalIndexExpression(e, base_ref, property_name_str, guard);
    }

    JSValue* EvalExpressionList(Error* e, AST* ast)
    {
        assert(ast->type() == AST::AST_EXPR);
        Expression* exprs = static_cast<Expression*>(ast);
        assert(exprs->elements().size() > 0);
        JSValue* val;
        for(AST* expr : exprs->elements())
        {
            JSValue* ref = EvalAssignmentExpression(e, expr);
            if(!e->IsOk())
                return nullptr;
            val = GetValue(e, ref);
            if(!e->IsOk())
                return nullptr;
        }
        return val;
    }

}// namespace es
