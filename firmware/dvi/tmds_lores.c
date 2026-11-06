/*
MIT License

Copyright (c) 2024 Thorsten Brehm
Copyright (c) 2026 Michael Neil

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include "tmds.h"
#include "config/config.h"
#include "util/dmacopy.h"

// TMDS symbols for LORES RGB colors - using the "double pixel" trick
// (each symbol covers two pixels and is encoded with a perfect 'bit balance').
uint32_t tmds_lorescolor[3*16];

// default: initial A2DVI color palette...
// gray1 != gray2
const uint32_t __in_flash("chr_rom") tmds_lores_default[3*16] =
{
    // R        G        B
    0x7fd00, 0x7fd00, 0x7fd00, // black
    0x886de, 0x7e107, 0xa2277, // magenta
    0x465e6, 0x465e6, 0x826f6, // darkblue
    0x85ee8, 0x44dec, 0xbfe00, // purple
    0x7fd00, 0x5fd80, 0x7fd00, // darkgreen
    0x5fd80, 0x5fd80, 0x5fd80, // gray1 (dark gray)
    0x411fb, 0x5819f, 0xbfe00, // mediumblue
    0xb3233, 0xb3233, 0xbfe00, // lightblue
    0x73133, 0x73133, 0x7fd00, // brown
    0x812fb, 0x9829f, 0x7fd00, // orange
    0x6fd40, 0x6fd40, 0x6fd40, // gray2 (light gray)
    0xbfe00, 0x5e187, 0x826f6, // pink
    0x45de8, 0x84eec, 0x7fd00, // green
    0x866e6, 0x866e6, 0x425f6, // yellow
    0x485de, 0xbe207, 0x62177, // aqua
    0xbfe00, 0xbfe00, 0xbfe00, // white
};

// Apple IIe original
// based on a color analysis by Linards Ticmanis (TeaRex)
// gray1==gray2
const uint32_t __in_flash("chr_rom") tmds_lores_original[3*16] =
{
    // R        G        B
    0x7fd00, 0x7fd00, 0x7fd00, // black
    0xb7e20, 0x7d909, 0x71d38, // magenta
    0x71d38, 0x445ee, 0xbce0c, // darkblue
    0x862e7, 0x4f9c1, 0xbfe00, // purple
    0x421f7, 0x9e686, 0x70d3c, // darkgreen
    0x6017f, 0x6017f, 0x6017f, // gray1 (dark gray)
    0xae247, 0x5c58e, 0xbfe00, // mediumblue
    0xb0a3d, 0x882df, 0xbfe00, // lightblue
    0x4e1c7, 0x77123, 0x7fd00, // brown
    0xb8a1d, 0x9ee84, 0x7fd00, // orange
    0x6017f, 0x6017f, 0x6017f, // gray2 (light gray)
    0x80efc, 0x8bed0, 0x8fac1, // pink
    0xafa41, 0xbba11, 0x7fd00, // green
    0x8fac1, 0x862e7, 0x7fd00, // yellow
    0xa3a71, 0x822f7, 0xb0a3d, // aqua
    0x816fa, 0x816fa, 0x816fa, // white
};

// improved: Apple IIe colors of an Apple IIgs in IIe emulation mode
// based on a color analysis by Linards Ticmanis (TeaRex)
// gray1!=gray2
const uint32_t __in_flash("chr_rom") tmds_lores_improved[3*16] =
{
    // R        G        B
    0x7fd00, 0x7fd00, 0x7fd00, // black
    0xb8e1c, 0x97ea0, 0x7053e, // magenta
    0x7ed04, 0x7c10f, 0x58d9c, // darkblue
    0x87ae1, 0x4f9c1, 0x6815f, // purple
    0x42df4, 0x6057e, 0x441ef, // darkgreen
    0x77921, 0x77921, 0x77921, // gray1 (dark gray)
    0x7a117, 0x906be, 0xbfe00, // mediumblue
    0x9f283, 0x8eec4, 0xbfe00, // lightblue
    0x5c18f, 0x48ddc, 0x7fd00, // brown
    0xbf203, 0xa1e78, 0x7fd00, // orange
    0x8eec4, 0x8eec4, 0x8eec4, // gray2 (light gray)
    0x80efc, 0x67163, 0x63971, // pink
    0x4fdc0, 0xb821f, 0x7fd00, // green
    0x802ff, 0x802ff, 0x7fd00, // yellow
    0x9de88, 0xbfa01, 0x67961, // aqua
    0xbfe00, 0xbfe00, 0xbfe00, // white
};

/*
    Orginal RGB values for the TMDS encodings
    // tmds_lores_default values:
    static List<(int, int, int)> tmds_lores_default_rbg = new List<(int, int, int)>
    {
        // R        G        B
       (0x00, 0x00, 0x00), // black
       (0x9D, 0x09, 0x66), // magenta
       (0x2A, 0x2A, 0xE5), // darkblue
       (0xC7, 0x34, 0xFF), // purple
       (0x00, 0x80, 0x00), // darkgreen
       (0x80, 0x80, 0x80), // gray1 (dark gray)
       (0x0D, 0xA1, 0xFF), // mediumblue
       (0xAA, 0xAA, 0xFF), // lightblue
       (0x55, 0x55, 0x00), // brown
       (0xF2, 0x5E, 0x00), // orange
       (0xC0, 0xC0, 0xC0), // gray2 (light gray)
       (0xFF, 0x89, 0xE5), // pink
       (0x38, 0xCB, 0x00), // green
       (0xD5, 0xD5, 0x1A), // yellow
       (0x62, 0xF6, 0x99), // aqua
       (0xFF, 0xFF, 0xFF), // white
    };

    // -------------------------

    // tmds_lores_original values:
    static List<(int, int, int)> tmds_lores_original_rgb = new List<(int, int, int)>
    {
        // R        G        B
       (0x00, 0x00, 0x00), // black
       (0x9F, 0x1B, 0x48), // magenta
       (0x48, 0x32, 0xEB), // darkblue
       (0xD6, 0x43, 0xFF), // purple
       (0x19, 0x75, 0x44), // darkgreen
       (0x81, 0x81, 0x81), // gray1 (dark gray)
       (0x36, 0x92, 0xFF), // mediumblue
       (0xB8, 0x9E, 0xFF), // lightblue
       (0x49, 0x65, 0x00), // brown
       (0xD8, 0x73, 0x00), // orange
       (0x81, 0x81, 0x81), // gray2 (light gray)
       (0xFB, 0x8F, 0xBC), // pink
       (0x3C, 0xCC, 0x00), // green
       (0xBC, 0xD6, 0x00), // yellow
       (0x6C, 0xE6, 0xB8), // aqua
       (0xF1, 0xF1, 0xF1), // white
    };

    // -------------------------

    // tmds_lores_improved values:
    static List<(int, int, int)> tmds_lores_improved_rgb = new List<(int, int, int)>
    {
        // R        G        B
       (0x00, 0x00, 0x00), // black
       (0xDB, 0x1F, 0x42), // magenta
       (0x0C, 0x11, 0xA4), // darkblue
       (0xDC, 0x43, 0xE1), // purple
       (0x1C, 0x82, 0x31), // darkgreen
       (0x63, 0x63, 0x63), // gray1 (dark gray)
       (0x39, 0x3D, 0xFF), // mediumblue
       (0x7A, 0xB3, 0xFF), // lightblue
       (0x91, 0x64, 0x00), // brown
       (0xFA, 0x77, 0x00), // orange
       (0xB3, 0xB3, 0xB3), // gray2 (light gray)
       (0xFB, 0xA5, 0x93), // pink
       (0x40, 0xDE, 0x00), // green
       (0xFE, 0xFE, 0x00), // yellow
       (0x67, 0xFC, 0xA3), // aqua
       (0xFF, 0xFF, 0xFF), // white
    };

    // -------------------------
*/

void DELAYED_COPY_CODE(tmds_color_load_lores)(uint color_style)
{
    const uint32_t* pSource = tmds_lores_default;

    switch(color_style)
    {
        case 1:
            pSource = tmds_lores_original;
            break;
        case 2:
            pSource = tmds_lores_improved;
            break;
        case 0: // fall-through
        default:
            break;
    }
    memcpy32(tmds_lorescolor, pSource, sizeof(tmds_lorescolor));
}

// LORES single pixel palette TMDS symbols: (for 80 col)
//  3 (RGB) double pixels
//  4 rows for 2-bits of FG and BG color
//  Base = ((((bg * 16) + fg) * 4) * 3) + (color_mode * (3*16*16*4))
//  36 KB
// LORES single pixel palette TMDS symbols:
uint32_t tmds_lores_single_pixel[3 * 16 * 16 * 4 * 3] =
{
    // R        G        B
    // tmds_lores_default
   0x7FD00, 0x7FD00, 0x7FD00, // 00 bg black bg black
   0x7FD00, 0x7FD00, 0x7FD00, // 10 fg black bg black
   0x7FD00, 0x7FD00, 0x7FD00, // 01 bg black fg black
   0x7FD00, 0x7FD00, 0x7FD00, // 11 fg black fg black
   0x7FD00, 0x7FD00, 0x7FD00, // 00 bg black bg black
   0xC02DE, 0x7FD07, 0xC0277, // 10 fg magenta bg black
   0x37900, 0x7E100, 0x1DD00, // 01 bg black fg magenta
   0x886DE, 0x7E107, 0xA2277, // 11 fg magenta fg magenta
   0x7FD00, 0x7FD00, 0x7FD00, // 00 bg black bg black
   0xC01E6, 0xC01E6, 0xC02F6, // 10 fg darkblue bg black
   0xF9900, 0xF9900, 0x3D900, // 01 bg black fg darkblue
   0x465E6, 0x465E6, 0x826F6, // 11 fg darkblue fg darkblue
   0x7FD00, 0x7FD00, 0x7FD00, // 00 bg black bg black
   0x7FEE8, 0xC01EC, 0x7FE00, // 10 fg purple bg black
   0x85D00, 0xFB100, 0xBFD00, // 01 bg black fg purple
   0x85EE8, 0x44DEC, 0xBFE00, // 11 fg purple fg purple
   0x7FD00, 0x7FD00, 0x7FD00, // 00 bg black bg black
   0x7FD00, 0x7FD80, 0x7FD00, // 10 fg darkgreen bg black
   0x7FD00, 0x5FD00, 0x7FD00, // 01 bg black fg darkgreen
   0x7FD00, 0x5FD80, 0x7FD00, // 11 fg darkgreen fg darkgreen
   0x7FD00, 0x7FD00, 0x7FD00, // 00 bg black bg black
   0x7FD80, 0x7FD80, 0x7FD80, // 10 fg gray1 (dark gray) bg black
   0x5FD00, 0x5FD00, 0x5FD00, // 01 bg black fg gray1 (dark gray)
   0x5FD80, 0x5FD80, 0x5FD80, // 11 fg gray1 (dark gray) fg gray1 (dark gray)
   0x7FD00, 0x7FD00, 0x7FD00, // 00 bg black bg black
   0xC01FB, 0xC019F, 0x7FE00, // 10 fg mediumblue bg black
   0xFED00, 0xE7D00, 0xBFD00, // 01 bg black fg mediumblue
   0x411FB, 0x5819F, 0xBFE00, // 11 fg mediumblue fg mediumblue
   0x7FD00, 0x7FD00, 0x7FD00, // 00 bg black bg black
   0x7FE33, 0x7FE33, 0x7FE00, // 10 fg lightblue bg black
   0xB3100, 0xB3100, 0xBFD00, // 01 bg black fg lightblue
   0xB3233, 0xB3233, 0xBFE00, // 11 fg lightblue fg lightblue
   0x7FD00, 0x7FD00, 0x7FD00, // 00 bg black bg black
   0x7FD33, 0x7FD33, 0x7FD00, // 10 fg brown bg black
   0x73100, 0x73100, 0x7FD00, // 01 bg black fg brown
   0x73133, 0x73133, 0x7FD00, // 11 fg brown fg brown
   0x7FD00, 0x7FD00, 0x7FD00, // 00 bg black bg black
   0xC02FB, 0xC029F, 0x7FD00, // 10 fg orange bg black
   0x3ED00, 0x27D00, 0x7FD00, // 01 bg black fg orange
   0x812FB, 0x9829F, 0x7FD00, // 11 fg orange fg orange
   0x7FD00, 0x7FD00, 0x7FD00, // 00 bg black bg black
   0x7FD40, 0x7FD40, 0x7FD40, // 10 fg gray2 (light gray) bg black
   0x6FD00, 0x6FD00, 0x6FD00, // 01 bg black fg gray2 (light gray)
   0x6FD40, 0x6FD40, 0x6FD40, // 11 fg gray2 (light gray) fg gray2 (light gray)
   0x7FD00, 0x7FD00, 0x7FD00, // 00 bg black bg black
   0x7FE00, 0x7FD87, 0xC02F6, // 10 fg pink bg black
   0xBFD00, 0x5E100, 0x3D900, // 01 bg black fg pink
   0xBFE00, 0x5E187, 0x826F6, // 11 fg pink fg pink
   0x7FD00, 0x7FD00, 0x7FD00, // 00 bg black bg black
   0x7FDE8, 0xC02EC, 0x7FD00, // 10 fg green bg black
   0x45D00, 0x3B100, 0x7FD00, // 01 bg black fg green
   0x45DE8, 0x84EEC, 0x7FD00, // 11 fg green fg green
   0x7FD00, 0x7FD00, 0x7FD00, // 00 bg black bg black
   0xC02E6, 0xC02E6, 0xC01F6, // 10 fg yellow bg black
   0x39900, 0x39900, 0xFD900, // 01 bg black fg yellow
   0x866E6, 0x866E6, 0x425F6, // 11 fg yellow fg yellow
   0x7FD00, 0x7FD00, 0x7FD00, // 00 bg black bg black
   0xC01DE, 0x7FE07, 0xC0177, // 10 fg aqua bg black
   0xF7900, 0xBE100, 0xDDD00, // 01 bg black fg aqua
   0x485DE, 0xBE207, 0x62177, // 11 fg aqua fg aqua
   0x7FD00, 0x7FD00, 0x7FD00, // 00 bg black bg black
   0x7FE00, 0x7FE00, 0x7FE00, // 10 fg white bg black
   0xBFD00, 0xBFD00, 0xBFD00, // 01 bg black fg white
   0xBFE00, 0xBFE00, 0xBFE00, // 11 fg white fg white
   0x886DE, 0x7E107, 0xA2277, // 00 bg magenta bg magenta
   0x37900, 0x7E100, 0x1DD00, // 10 fg black bg magenta
   0xC02DE, 0x7FD07, 0xC0277, // 01 bg magenta fg black
   0x7FD00, 0x7FD00, 0x7FD00, // 11 fg black fg black
   0x886DE, 0x7E107, 0xA2277, // 00 bg magenta bg magenta
   0x886DE, 0x7E107, 0xA2277, // 10 fg magenta bg magenta
   0x886DE, 0x7E107, 0xA2277, // 01 bg magenta fg magenta
   0x886DE, 0x7E107, 0xA2277, // 11 fg magenta fg magenta
   0x886DE, 0x7E107, 0xA2277, // 00 bg magenta bg magenta
   0x885E6, 0xC1DE6, 0xA22F6, // 10 fg darkblue bg magenta
   0x466DE, 0xF9907, 0x82677, // 01 bg magenta fg darkblue
   0x465E6, 0x465E6, 0x826F6, // 11 fg darkblue fg darkblue
   0x886DE, 0x7E107, 0xA2277, // 00 bg magenta bg magenta
   0x886E8, 0xC1DEC, 0x1DE00, // 10 fg purple bg magenta
   0x85EDE, 0xFB107, 0x00277, // 01 bg magenta fg purple
   0x85EE8, 0x44DEC, 0xBFE00, // 11 fg purple fg purple
   0x886DE, 0x7E107, 0xA2277, // 00 bg magenta bg magenta
   0x37900, 0x7E180, 0x1DD00, // 10 fg darkgreen bg magenta
   0xC02DE, 0x5FD07, 0xC0277, // 01 bg magenta fg darkgreen
   0x7FD00, 0x5FD80, 0x7FD00, // 11 fg darkgreen fg darkgreen
   0x886DE, 0x7E107, 0xA2277, // 00 bg magenta bg magenta
   0x37980, 0x7E180, 0x1DD80, // 10 fg gray1 (dark gray) bg magenta
   0xE02DE, 0x5FD07, 0xE0277, // 01 bg magenta fg gray1 (dark gray)
   0x5FD80, 0x5FD80, 0x5FD80, // 11 fg gray1 (dark gray) fg gray1 (dark gray)
   0x886DE, 0x7E107, 0xA2277, // 00 bg magenta bg magenta
   0x885FB, 0xC1D9F, 0x1DE00, // 10 fg mediumblue bg magenta
   0x412DE, 0xE7D07, 0x00277, // 01 bg magenta fg mediumblue
   0x411FB, 0x5819F, 0xBFE00, // 11 fg mediumblue fg mediumblue
   0x886DE, 0x7E107, 0xA2277, // 00 bg magenta bg magenta
   0x88633, 0x7E233, 0x1DE00, // 10 fg lightblue bg magenta
   0xB32DE, 0xB3107, 0x00277, // 01 bg magenta fg lightblue
   0xB3233, 0xB3233, 0xBFE00, // 11 fg lightblue fg lightblue
   0x886DE, 0x7E107, 0xA2277, // 00 bg magenta bg magenta
   0x88533, 0x7E133, 0x1DD00, // 10 fg brown bg magenta
   0x732DE, 0x73107, 0xC0277, // 01 bg magenta fg brown
   0x73133, 0x73133, 0x7FD00, // 11 fg brown fg brown
   0x886DE, 0x7E107, 0xA2277, // 00 bg magenta bg magenta
   0x886FB, 0xC1E9F, 0x1DD00, // 10 fg orange bg magenta
   0x812DE, 0x27D07, 0xC0277, // 01 bg magenta fg orange
   0x812FB, 0x9829F, 0x7FD00, // 11 fg orange fg orange
   0x886DE, 0x7E107, 0xA2277, // 00 bg magenta bg magenta
   0x37940, 0x7E140, 0x1DD40, // 10 fg gray2 (light gray) bg magenta
   0xD02DE, 0x6FD07, 0xD0277, // 01 bg magenta fg gray2 (light gray)
   0x6FD40, 0x6FD40, 0x6FD40, // 11 fg gray2 (light gray) fg gray2 (light gray)
   0x886DE, 0x7E107, 0xA2277, // 00 bg magenta bg magenta
   0x37A00, 0x7E187, 0xA22F6, // 10 fg pink bg magenta
   0x002DE, 0x5E107, 0x82677, // 01 bg magenta fg pink
   0xBFE00, 0x5E187, 0x826F6, // 11 fg pink fg pink
   0x886DE, 0x7E107, 0xA2277, // 00 bg magenta bg magenta
   0x885E8, 0xC1EEC, 0x1DD00, // 10 fg green bg magenta
   0x45EDE, 0x3B107, 0xC0277, // 01 bg magenta fg green
   0x45DE8, 0x84EEC, 0x7FD00, // 11 fg green fg green
   0x886DE, 0x7E107, 0xA2277, // 00 bg magenta bg magenta
   0x886E6, 0xC1EE6, 0xA21F6, // 10 fg yellow bg magenta
   0x866DE, 0x39907, 0x42677, // 01 bg magenta fg yellow
   0x866E6, 0x866E6, 0x425F6, // 11 fg yellow fg yellow
   0x886DE, 0x7E107, 0xA2277, // 00 bg magenta bg magenta
   0x885DE, 0x7E207, 0xA2177, // 10 fg aqua bg magenta
   0x486DE, 0xBE107, 0x62277, // 01 bg magenta fg aqua
   0x485DE, 0xBE207, 0x62177, // 11 fg aqua fg aqua
   0x886DE, 0x7E107, 0xA2277, // 00 bg magenta bg magenta
   0x37A00, 0x7E200, 0x1DE00, // 10 fg white bg magenta
   0x002DE, 0xBFD07, 0x00277, // 01 bg magenta fg white
   0xBFE00, 0xBFE00, 0xBFE00, // 11 fg white fg white
   0x465E6, 0x465E6, 0x826F6, // 00 bg darkblue bg darkblue
   0xF9900, 0xF9900, 0x3D900, // 10 fg black bg darkblue
   0xC01E6, 0xC01E6, 0xC02F6, // 01 bg darkblue fg black
   0x7FD00, 0x7FD00, 0x7FD00, // 11 fg black fg black
   0x465E6, 0x465E6, 0x826F6, // 00 bg darkblue bg darkblue
   0x466DE, 0xF9907, 0x82677, // 10 fg magenta bg darkblue
   0x885E6, 0xC1DE6, 0xA22F6, // 01 bg darkblue fg magenta
   0x886DE, 0x7E107, 0xA2277, // 11 fg magenta fg magenta
   0x465E6, 0x465E6, 0x826F6, // 00 bg darkblue bg darkblue
   0x465E6, 0x465E6, 0x826F6, // 10 fg darkblue bg darkblue
   0x465E6, 0x465E6, 0x826F6, // 01 bg darkblue fg darkblue
   0x465E6, 0x465E6, 0x826F6, // 11 fg darkblue fg darkblue
   0x465E6, 0x465E6, 0x826F6, // 00 bg darkblue bg darkblue
   0x466E8, 0x465EC, 0x3DA00, // 10 fg purple bg darkblue
   0x85DE6, 0x44DE6, 0x002F6, // 01 bg darkblue fg purple
   0x85EE8, 0x44DEC, 0xBFE00, // 11 fg purple fg purple
   0x465E6, 0x465E6, 0x826F6, // 00 bg darkblue bg darkblue
   0xF9900, 0xF9980, 0x3D900, // 10 fg darkgreen bg darkblue
   0xC01E6, 0xE01E6, 0xC02F6, // 01 bg darkblue fg darkgreen
   0x7FD00, 0x5FD80, 0x7FD00, // 11 fg darkgreen fg darkgreen
   0x465E6, 0x465E6, 0x826F6, // 00 bg darkblue bg darkblue
   0xF9980, 0xF9980, 0x3D980, // 10 fg gray1 (dark gray) bg darkblue
   0xE01E6, 0xE01E6, 0xE02F6, // 01 bg darkblue fg gray1 (dark gray)
   0x5FD80, 0x5FD80, 0x5FD80, // 11 fg gray1 (dark gray) fg gray1 (dark gray)
   0x465E6, 0x465E6, 0x826F6, // 00 bg darkblue bg darkblue
   0x465FB, 0x4659F, 0x3DA00, // 10 fg mediumblue bg darkblue
   0x411E6, 0x581E6, 0x002F6, // 01 bg darkblue fg mediumblue
   0x411FB, 0x5819F, 0xBFE00, // 11 fg mediumblue fg mediumblue
   0x465E6, 0x465E6, 0x826F6, // 00 bg darkblue bg darkblue
   0x46633, 0x46633, 0x3DA00, // 10 fg lightblue bg darkblue
   0xB31E6, 0xB31E6, 0x002F6, // 01 bg darkblue fg lightblue
   0xB3233, 0xB3233, 0xBFE00, // 11 fg lightblue fg lightblue
   0x465E6, 0x465E6, 0x826F6, // 00 bg darkblue bg darkblue
   0x46533, 0x46533, 0x3D900, // 10 fg brown bg darkblue
   0x731E6, 0x731E6, 0xC02F6, // 01 bg darkblue fg brown
   0x73133, 0x73133, 0x7FD00, // 11 fg brown fg brown
   0x465E6, 0x465E6, 0x826F6, // 00 bg darkblue bg darkblue
   0x466FB, 0x4669F, 0x3D900, // 10 fg orange bg darkblue
   0x811E6, 0x981E6, 0xC02F6, // 01 bg darkblue fg orange
   0x812FB, 0x9829F, 0x7FD00, // 11 fg orange fg orange
   0x465E6, 0x465E6, 0x826F6, // 00 bg darkblue bg darkblue
   0xF9940, 0xF9940, 0x3D940, // 10 fg gray2 (light gray) bg darkblue
   0xD01E6, 0xD01E6, 0xD02F6, // 01 bg darkblue fg gray2 (light gray)
   0x6FD40, 0x6FD40, 0x6FD40, // 11 fg gray2 (light gray) fg gray2 (light gray)
   0x465E6, 0x465E6, 0x826F6, // 00 bg darkblue bg darkblue
   0xF9A00, 0x46587, 0x826F6, // 10 fg pink bg darkblue
   0x001E6, 0x5E1E6, 0x826F6, // 01 bg darkblue fg pink
   0xBFE00, 0x5E187, 0x826F6, // 11 fg pink fg pink
   0x465E6, 0x465E6, 0x826F6, // 00 bg darkblue bg darkblue
   0x465E8, 0x466EC, 0x3D900, // 10 fg green bg darkblue
   0x45DE6, 0x84DE6, 0xC02F6, // 01 bg darkblue fg green
   0x45DE8, 0x84EEC, 0x7FD00, // 11 fg green fg green
   0x465E6, 0x465E6, 0x826F6, // 00 bg darkblue bg darkblue
   0x466E6, 0x466E6, 0x825F6, // 10 fg yellow bg darkblue
   0x865E6, 0x865E6, 0x426F6, // 01 bg darkblue fg yellow
   0x866E6, 0x866E6, 0x425F6, // 11 fg yellow fg yellow
   0x465E6, 0x465E6, 0x826F6, // 00 bg darkblue bg darkblue
   0x465DE, 0xF9A07, 0x82577, // 10 fg aqua bg darkblue
   0x485E6, 0x01DE6, 0x622F6, // 01 bg darkblue fg aqua
   0x485DE, 0xBE207, 0x62177, // 11 fg aqua fg aqua
   0x465E6, 0x465E6, 0x826F6, // 00 bg darkblue bg darkblue
   0xF9A00, 0xF9A00, 0x3DA00, // 10 fg white bg darkblue
   0x001E6, 0x001E6, 0x002F6, // 01 bg darkblue fg white
   0xBFE00, 0xBFE00, 0xBFE00, // 11 fg white fg white
   0x85EE8, 0x44DEC, 0xBFE00, // 00 bg purple bg purple
   0x85D00, 0xFB100, 0xBFD00, // 10 fg black bg purple
   0x7FEE8, 0xC01EC, 0x7FE00, // 01 bg purple fg black
   0x7FD00, 0x7FD00, 0x7FD00, // 11 fg black fg black
   0x85EE8, 0x44DEC, 0xBFE00, // 00 bg purple bg purple
   0x85EDE, 0xFB107, 0x00277, // 10 fg magenta bg purple
   0x886E8, 0xC1DEC, 0x1DE00, // 01 bg purple fg magenta
   0x886DE, 0x7E107, 0xA2277, // 11 fg magenta fg magenta
   0x85EE8, 0x44DEC, 0xBFE00, // 00 bg purple bg purple
   0x85DE6, 0x44DE6, 0x002F6, // 10 fg darkblue bg purple
   0x466E8, 0x465EC, 0x3DA00, // 01 bg purple fg darkblue
   0x465E6, 0x465E6, 0x826F6, // 11 fg darkblue fg darkblue
   0x85EE8, 0x44DEC, 0xBFE00, // 00 bg purple bg purple
   0x85EE8, 0x44DEC, 0xBFE00, // 10 fg purple bg purple
   0x85EE8, 0x44DEC, 0xBFE00, // 01 bg purple fg purple
   0x85EE8, 0x44DEC, 0xBFE00, // 11 fg purple fg purple
   0x85EE8, 0x44DEC, 0xBFE00, // 00 bg purple bg purple
   0x85D00, 0xFB180, 0xBFD00, // 10 fg darkgreen bg purple
   0x7FEE8, 0xE01EC, 0x7FE00, // 01 bg purple fg darkgreen
   0x7FD00, 0x5FD80, 0x7FD00, // 11 fg darkgreen fg darkgreen
   0x85EE8, 0x44DEC, 0xBFE00, // 00 bg purple bg purple
   0x85D80, 0xFB180, 0xBFD80, // 10 fg gray1 (dark gray) bg purple
   0x5FEE8, 0xE01EC, 0x5FE00, // 01 bg purple fg gray1 (dark gray)
   0x5FD80, 0x5FD80, 0x5FD80, // 11 fg gray1 (dark gray) fg gray1 (dark gray)
   0x85EE8, 0x44DEC, 0xBFE00, // 00 bg purple bg purple
   0x85DFB, 0x44D9F, 0xBFE00, // 10 fg mediumblue bg purple
   0x412E8, 0x581EC, 0xBFE00, // 01 bg purple fg mediumblue
   0x411FB, 0x5819F, 0xBFE00, // 11 fg mediumblue fg mediumblue
   0x85EE8, 0x44DEC, 0xBFE00, // 00 bg purple bg purple
   0x85E33, 0x44E33, 0xBFE00, // 10 fg lightblue bg purple
   0xB32E8, 0xB31EC, 0xBFE00, // 01 bg purple fg lightblue
   0xB3233, 0xB3233, 0xBFE00, // 11 fg lightblue fg lightblue
   0x85EE8, 0x44DEC, 0xBFE00, // 00 bg purple bg purple
   0x85D33, 0x44D33, 0xBFD00, // 10 fg brown bg purple
   0x732E8, 0x731EC, 0x7FE00, // 01 bg purple fg brown
   0x73133, 0x73133, 0x7FD00, // 11 fg brown fg brown
   0x85EE8, 0x44DEC, 0xBFE00, // 00 bg purple bg purple
   0x85EFB, 0x44E9F, 0xBFD00, // 10 fg orange bg purple
   0x812E8, 0x981EC, 0x7FE00, // 01 bg purple fg orange
   0x812FB, 0x9829F, 0x7FD00, // 11 fg orange fg orange
   0x85EE8, 0x44DEC, 0xBFE00, // 00 bg purple bg purple
   0x85D40, 0xFB140, 0xBFD40, // 10 fg gray2 (light gray) bg purple
   0x6FEE8, 0xD01EC, 0x6FE00, // 01 bg purple fg gray2 (light gray)
   0x6FD40, 0x6FD40, 0x6FD40, // 11 fg gray2 (light gray) fg gray2 (light gray)
   0x85EE8, 0x44DEC, 0xBFE00, // 00 bg purple bg purple
   0x85E00, 0x44D87, 0x002F6, // 10 fg pink bg purple
   0xBFEE8, 0x5E1EC, 0x3DA00, // 01 bg purple fg pink
   0xBFE00, 0x5E187, 0x826F6, // 11 fg pink fg pink
   0x85EE8, 0x44DEC, 0xBFE00, // 00 bg purple bg purple
   0x85DE8, 0x44EEC, 0xBFD00, // 10 fg green bg purple
   0x45EE8, 0x84DEC, 0x7FE00, // 01 bg purple fg green
   0x45DE8, 0x84EEC, 0x7FD00, // 11 fg green fg green
   0x85EE8, 0x44DEC, 0xBFE00, // 00 bg purple bg purple
   0x85EE6, 0x44EE6, 0x001F6, // 10 fg yellow bg purple
   0x866E8, 0x865EC, 0xFDA00, // 01 bg purple fg yellow
   0x866E6, 0x866E6, 0x425F6, // 11 fg yellow fg yellow
   0x85EE8, 0x44DEC, 0xBFE00, // 00 bg purple bg purple
   0x85DDE, 0xFB207, 0x00177, // 10 fg aqua bg purple
   0x486E8, 0x01DEC, 0xDDE00, // 01 bg purple fg aqua
   0x485DE, 0xBE207, 0x62177, // 11 fg aqua fg aqua
   0x85EE8, 0x44DEC, 0xBFE00, // 00 bg purple bg purple
   0x85E00, 0xFB200, 0xBFE00, // 10 fg white bg purple
   0xBFEE8, 0x001EC, 0xBFE00, // 01 bg purple fg white
   0xBFE00, 0xBFE00, 0xBFE00, // 11 fg white fg white
   0x7FD00, 0x5FD80, 0x7FD00, // 00 bg darkgreen bg darkgreen
   0x7FD00, 0x5FD00, 0x7FD00, // 10 fg black bg darkgreen
   0x7FD00, 0x7FD80, 0x7FD00, // 01 bg darkgreen fg black
   0x7FD00, 0x7FD00, 0x7FD00, // 11 fg black fg black
   0x7FD00, 0x5FD80, 0x7FD00, // 00 bg darkgreen bg darkgreen
   0xC02DE, 0x5FD07, 0xC0277, // 10 fg magenta bg darkgreen
   0x37900, 0x7E180, 0x1DD00, // 01 bg darkgreen fg magenta
   0x886DE, 0x7E107, 0xA2277, // 11 fg magenta fg magenta
   0x7FD00, 0x5FD80, 0x7FD00, // 00 bg darkgreen bg darkgreen
   0xC01E6, 0xE01E6, 0xC02F6, // 10 fg darkblue bg darkgreen
   0xF9900, 0xF9980, 0x3D900, // 01 bg darkgreen fg darkblue
   0x465E6, 0x465E6, 0x826F6, // 11 fg darkblue fg darkblue
   0x7FD00, 0x5FD80, 0x7FD00, // 00 bg darkgreen bg darkgreen
   0x7FEE8, 0xE01EC, 0x7FE00, // 10 fg purple bg darkgreen
   0x85D00, 0xFB180, 0xBFD00, // 01 bg darkgreen fg purple
   0x85EE8, 0x44DEC, 0xBFE00, // 11 fg purple fg purple
   0x7FD00, 0x5FD80, 0x7FD00, // 00 bg darkgreen bg darkgreen
   0x7FD00, 0x5FD80, 0x7FD00, // 10 fg darkgreen bg darkgreen
   0x7FD00, 0x5FD80, 0x7FD00, // 01 bg darkgreen fg darkgreen
   0x7FD00, 0x5FD80, 0x7FD00, // 11 fg darkgreen fg darkgreen
   0x7FD00, 0x5FD80, 0x7FD00, // 00 bg darkgreen bg darkgreen
   0x7FD80, 0x5FD80, 0x7FD80, // 10 fg gray1 (dark gray) bg darkgreen
   0x5FD00, 0x5FD80, 0x5FD00, // 01 bg darkgreen fg gray1 (dark gray)
   0x5FD80, 0x5FD80, 0x5FD80, // 11 fg gray1 (dark gray) fg gray1 (dark gray)
   0x7FD00, 0x5FD80, 0x7FD00, // 00 bg darkgreen bg darkgreen
   0xC01FB, 0xE019F, 0x7FE00, // 10 fg mediumblue bg darkgreen
   0xFED00, 0xE7D80, 0xBFD00, // 01 bg darkgreen fg mediumblue
   0x411FB, 0x5819F, 0xBFE00, // 11 fg mediumblue fg mediumblue
   0x7FD00, 0x5FD80, 0x7FD00, // 00 bg darkgreen bg darkgreen
   0x7FE33, 0x5FE33, 0x7FE00, // 10 fg lightblue bg darkgreen
   0xB3100, 0xB3180, 0xBFD00, // 01 bg darkgreen fg lightblue
   0xB3233, 0xB3233, 0xBFE00, // 11 fg lightblue fg lightblue
   0x7FD00, 0x5FD80, 0x7FD00, // 00 bg darkgreen bg darkgreen
   0x7FD33, 0x5FD33, 0x7FD00, // 10 fg brown bg darkgreen
   0x73100, 0x73180, 0x7FD00, // 01 bg darkgreen fg brown
   0x73133, 0x73133, 0x7FD00, // 11 fg brown fg brown
   0x7FD00, 0x5FD80, 0x7FD00, // 00 bg darkgreen bg darkgreen
   0xC02FB, 0xE029F, 0x7FD00, // 10 fg orange bg darkgreen
   0x3ED00, 0x27D80, 0x7FD00, // 01 bg darkgreen fg orange
   0x812FB, 0x9829F, 0x7FD00, // 11 fg orange fg orange
   0x7FD00, 0x5FD80, 0x7FD00, // 00 bg darkgreen bg darkgreen
   0x7FD40, 0x5FD40, 0x7FD40, // 10 fg gray2 (light gray) bg darkgreen
   0x6FD00, 0x6FD80, 0x6FD00, // 01 bg darkgreen fg gray2 (light gray)
   0x6FD40, 0x6FD40, 0x6FD40, // 11 fg gray2 (light gray) fg gray2 (light gray)
   0x7FD00, 0x5FD80, 0x7FD00, // 00 bg darkgreen bg darkgreen
   0x7FE00, 0x5FD87, 0xC02F6, // 10 fg pink bg darkgreen
   0xBFD00, 0x5E180, 0x3D900, // 01 bg darkgreen fg pink
   0xBFE00, 0x5E187, 0x826F6, // 11 fg pink fg pink
   0x7FD00, 0x5FD80, 0x7FD00, // 00 bg darkgreen bg darkgreen
   0x7FDE8, 0xE02EC, 0x7FD00, // 10 fg green bg darkgreen
   0x45D00, 0x3B180, 0x7FD00, // 01 bg darkgreen fg green
   0x45DE8, 0x84EEC, 0x7FD00, // 11 fg green fg green
   0x7FD00, 0x5FD80, 0x7FD00, // 00 bg darkgreen bg darkgreen
   0xC02E6, 0xE02E6, 0xC01F6, // 10 fg yellow bg darkgreen
   0x39900, 0x39980, 0xFD900, // 01 bg darkgreen fg yellow
   0x866E6, 0x866E6, 0x425F6, // 11 fg yellow fg yellow
   0x7FD00, 0x5FD80, 0x7FD00, // 00 bg darkgreen bg darkgreen
   0xC01DE, 0x5FE07, 0xC0177, // 10 fg aqua bg darkgreen
   0xF7900, 0xBE180, 0xDDD00, // 01 bg darkgreen fg aqua
   0x485DE, 0xBE207, 0x62177, // 11 fg aqua fg aqua
   0x7FD00, 0x5FD80, 0x7FD00, // 00 bg darkgreen bg darkgreen
   0x7FE00, 0x5FE00, 0x7FE00, // 10 fg white bg darkgreen
   0xBFD00, 0xBFD80, 0xBFD00, // 01 bg darkgreen fg white
   0xBFE00, 0xBFE00, 0xBFE00, // 11 fg white fg white
   0x5FD80, 0x5FD80, 0x5FD80, // 00 bg gray1 (dark gray) bg gray1 (dark gray)
   0x5FD00, 0x5FD00, 0x5FD00, // 10 fg black bg gray1 (dark gray)
   0x7FD80, 0x7FD80, 0x7FD80, // 01 bg gray1 (dark gray) fg black
   0x7FD00, 0x7FD00, 0x7FD00, // 11 fg black fg black
   0x5FD80, 0x5FD80, 0x5FD80, // 00 bg gray1 (dark gray) bg gray1 (dark gray)
   0xE02DE, 0x5FD07, 0xE0277, // 10 fg magenta bg gray1 (dark gray)
   0x37980, 0x7E180, 0x1DD80, // 01 bg gray1 (dark gray) fg magenta
   0x886DE, 0x7E107, 0xA2277, // 11 fg magenta fg magenta
   0x5FD80, 0x5FD80, 0x5FD80, // 00 bg gray1 (dark gray) bg gray1 (dark gray)
   0xE01E6, 0xE01E6, 0xE02F6, // 10 fg darkblue bg gray1 (dark gray)
   0xF9980, 0xF9980, 0x3D980, // 01 bg gray1 (dark gray) fg darkblue
   0x465E6, 0x465E6, 0x826F6, // 11 fg darkblue fg darkblue
   0x5FD80, 0x5FD80, 0x5FD80, // 00 bg gray1 (dark gray) bg gray1 (dark gray)
   0x5FEE8, 0xE01EC, 0x5FE00, // 10 fg purple bg gray1 (dark gray)
   0x85D80, 0xFB180, 0xBFD80, // 01 bg gray1 (dark gray) fg purple
   0x85EE8, 0x44DEC, 0xBFE00, // 11 fg purple fg purple
   0x5FD80, 0x5FD80, 0x5FD80, // 00 bg gray1 (dark gray) bg gray1 (dark gray)
   0x5FD00, 0x5FD80, 0x5FD00, // 10 fg darkgreen bg gray1 (dark gray)
   0x7FD80, 0x5FD80, 0x7FD80, // 01 bg gray1 (dark gray) fg darkgreen
   0x7FD00, 0x5FD80, 0x7FD00, // 11 fg darkgreen fg darkgreen
   0x5FD80, 0x5FD80, 0x5FD80, // 00 bg gray1 (dark gray) bg gray1 (dark gray)
   0x5FD80, 0x5FD80, 0x5FD80, // 10 fg gray1 (dark gray) bg gray1 (dark gray)
   0x5FD80, 0x5FD80, 0x5FD80, // 01 bg gray1 (dark gray) fg gray1 (dark gray)
   0x5FD80, 0x5FD80, 0x5FD80, // 11 fg gray1 (dark gray) fg gray1 (dark gray)
   0x5FD80, 0x5FD80, 0x5FD80, // 00 bg gray1 (dark gray) bg gray1 (dark gray)
   0xE01FB, 0xE019F, 0x5FE00, // 10 fg mediumblue bg gray1 (dark gray)
   0xFED80, 0xE7D80, 0xBFD80, // 01 bg gray1 (dark gray) fg mediumblue
   0x411FB, 0x5819F, 0xBFE00, // 11 fg mediumblue fg mediumblue
   0x5FD80, 0x5FD80, 0x5FD80, // 00 bg gray1 (dark gray) bg gray1 (dark gray)
   0x5FE33, 0x5FE33, 0x5FE00, // 10 fg lightblue bg gray1 (dark gray)
   0xB3180, 0xB3180, 0xBFD80, // 01 bg gray1 (dark gray) fg lightblue
   0xB3233, 0xB3233, 0xBFE00, // 11 fg lightblue fg lightblue
   0x5FD80, 0x5FD80, 0x5FD80, // 00 bg gray1 (dark gray) bg gray1 (dark gray)
   0x5FD33, 0x5FD33, 0x5FD00, // 10 fg brown bg gray1 (dark gray)
   0x73180, 0x73180, 0x7FD80, // 01 bg gray1 (dark gray) fg brown
   0x73133, 0x73133, 0x7FD00, // 11 fg brown fg brown
   0x5FD80, 0x5FD80, 0x5FD80, // 00 bg gray1 (dark gray) bg gray1 (dark gray)
   0xE02FB, 0xE029F, 0x5FD00, // 10 fg orange bg gray1 (dark gray)
   0x3ED80, 0x27D80, 0x7FD80, // 01 bg gray1 (dark gray) fg orange
   0x812FB, 0x9829F, 0x7FD00, // 11 fg orange fg orange
   0x5FD80, 0x5FD80, 0x5FD80, // 00 bg gray1 (dark gray) bg gray1 (dark gray)
   0x5FD40, 0x5FD40, 0x5FD40, // 10 fg gray2 (light gray) bg gray1 (dark gray)
   0x6FD80, 0x6FD80, 0x6FD80, // 01 bg gray1 (dark gray) fg gray2 (light gray)
   0x6FD40, 0x6FD40, 0x6FD40, // 11 fg gray2 (light gray) fg gray2 (light gray)
   0x5FD80, 0x5FD80, 0x5FD80, // 00 bg gray1 (dark gray) bg gray1 (dark gray)
   0x5FE00, 0x5FD87, 0xE02F6, // 10 fg pink bg gray1 (dark gray)
   0xBFD80, 0x5E180, 0x3D980, // 01 bg gray1 (dark gray) fg pink
   0xBFE00, 0x5E187, 0x826F6, // 11 fg pink fg pink
   0x5FD80, 0x5FD80, 0x5FD80, // 00 bg gray1 (dark gray) bg gray1 (dark gray)
   0x5FDE8, 0xE02EC, 0x5FD00, // 10 fg green bg gray1 (dark gray)
   0x45D80, 0x3B180, 0x7FD80, // 01 bg gray1 (dark gray) fg green
   0x45DE8, 0x84EEC, 0x7FD00, // 11 fg green fg green
   0x5FD80, 0x5FD80, 0x5FD80, // 00 bg gray1 (dark gray) bg gray1 (dark gray)
   0xE02E6, 0xE02E6, 0xE01F6, // 10 fg yellow bg gray1 (dark gray)
   0x39980, 0x39980, 0xFD980, // 01 bg gray1 (dark gray) fg yellow
   0x866E6, 0x866E6, 0x425F6, // 11 fg yellow fg yellow
   0x5FD80, 0x5FD80, 0x5FD80, // 00 bg gray1 (dark gray) bg gray1 (dark gray)
   0xE01DE, 0x5FE07, 0xE0177, // 10 fg aqua bg gray1 (dark gray)
   0xF7980, 0xBE180, 0xDDD80, // 01 bg gray1 (dark gray) fg aqua
   0x485DE, 0xBE207, 0x62177, // 11 fg aqua fg aqua
   0x5FD80, 0x5FD80, 0x5FD80, // 00 bg gray1 (dark gray) bg gray1 (dark gray)
   0x5FE00, 0x5FE00, 0x5FE00, // 10 fg white bg gray1 (dark gray)
   0xBFD80, 0xBFD80, 0xBFD80, // 01 bg gray1 (dark gray) fg white
   0xBFE00, 0xBFE00, 0xBFE00, // 11 fg white fg white
   0x411FB, 0x5819F, 0xBFE00, // 00 bg mediumblue bg mediumblue
   0xFED00, 0xE7D00, 0xBFD00, // 10 fg black bg mediumblue
   0xC01FB, 0xC019F, 0x7FE00, // 01 bg mediumblue fg black
   0x7FD00, 0x7FD00, 0x7FD00, // 11 fg black fg black
   0x411FB, 0x5819F, 0xBFE00, // 00 bg mediumblue bg mediumblue
   0x412DE, 0xE7D07, 0x00277, // 10 fg magenta bg mediumblue
   0x885FB, 0xC1D9F, 0x1DE00, // 01 bg mediumblue fg magenta
   0x886DE, 0x7E107, 0xA2277, // 11 fg magenta fg magenta
   0x411FB, 0x5819F, 0xBFE00, // 00 bg mediumblue bg mediumblue
   0x411E6, 0x581E6, 0x002F6, // 10 fg darkblue bg mediumblue
   0x465FB, 0x4659F, 0x3DA00, // 01 bg mediumblue fg darkblue
   0x465E6, 0x465E6, 0x826F6, // 11 fg darkblue fg darkblue
   0x411FB, 0x5819F, 0xBFE00, // 00 bg mediumblue bg mediumblue
   0x412E8, 0x581EC, 0xBFE00, // 10 fg purple bg mediumblue
   0x85DFB, 0x44D9F, 0xBFE00, // 01 bg mediumblue fg purple
   0x85EE8, 0x44DEC, 0xBFE00, // 11 fg purple fg purple
   0x411FB, 0x5819F, 0xBFE00, // 00 bg mediumblue bg mediumblue
   0xFED00, 0xE7D80, 0xBFD00, // 10 fg darkgreen bg mediumblue
   0xC01FB, 0xE019F, 0x7FE00, // 01 bg mediumblue fg darkgreen
   0x7FD00, 0x5FD80, 0x7FD00, // 11 fg darkgreen fg darkgreen
   0x411FB, 0x5819F, 0xBFE00, // 00 bg mediumblue bg mediumblue
   0xFED80, 0xE7D80, 0xBFD80, // 10 fg gray1 (dark gray) bg mediumblue
   0xE01FB, 0xE019F, 0x5FE00, // 01 bg mediumblue fg gray1 (dark gray)
   0x5FD80, 0x5FD80, 0x5FD80, // 11 fg gray1 (dark gray) fg gray1 (dark gray)
   0x411FB, 0x5819F, 0xBFE00, // 00 bg mediumblue bg mediumblue
   0x411FB, 0x5819F, 0xBFE00, // 10 fg mediumblue bg mediumblue
   0x411FB, 0x5819F, 0xBFE00, // 01 bg mediumblue fg mediumblue
   0x411FB, 0x5819F, 0xBFE00, // 11 fg mediumblue fg mediumblue
   0x411FB, 0x5819F, 0xBFE00, // 00 bg mediumblue bg mediumblue
   0x41233, 0x58233, 0xBFE00, // 10 fg lightblue bg mediumblue
   0xB31FB, 0xB319F, 0xBFE00, // 01 bg mediumblue fg lightblue
   0xB3233, 0xB3233, 0xBFE00, // 11 fg lightblue fg lightblue
   0x411FB, 0x5819F, 0xBFE00, // 00 bg mediumblue bg mediumblue
   0x41133, 0x58133, 0xBFD00, // 10 fg brown bg mediumblue
   0x731FB, 0x7319F, 0x7FE00, // 01 bg mediumblue fg brown
   0x73133, 0x73133, 0x7FD00, // 11 fg brown fg brown
   0x411FB, 0x5819F, 0xBFE00, // 00 bg mediumblue bg mediumblue
   0x412FB, 0x5829F, 0xBFD00, // 10 fg orange bg mediumblue
   0x811FB, 0x9819F, 0x7FE00, // 01 bg mediumblue fg orange
   0x812FB, 0x9829F, 0x7FD00, // 11 fg orange fg orange
   0x411FB, 0x5819F, 0xBFE00, // 00 bg mediumblue bg mediumblue
   0xFED40, 0xE7D40, 0xBFD40, // 10 fg gray2 (light gray) bg mediumblue
   0xD01FB, 0xD019F, 0x6FE00, // 01 bg mediumblue fg gray2 (light gray)
   0x6FD40, 0x6FD40, 0x6FD40, // 11 fg gray2 (light gray) fg gray2 (light gray)
   0x411FB, 0x5819F, 0xBFE00, // 00 bg mediumblue bg mediumblue
   0xFEE00, 0x58187, 0x002F6, // 10 fg pink bg mediumblue
   0x001FB, 0x5E19F, 0x3DA00, // 01 bg mediumblue fg pink
   0xBFE00, 0x5E187, 0x826F6, // 11 fg pink fg pink
   0x411FB, 0x5819F, 0xBFE00, // 00 bg mediumblue bg mediumblue
   0x411E8, 0x582EC, 0xBFD00, // 10 fg green bg mediumblue
   0x45DFB, 0x84D9F, 0x7FE00, // 01 bg mediumblue fg green
   0x45DE8, 0x84EEC, 0x7FD00, // 11 fg green fg green
   0x411FB, 0x5819F, 0xBFE00, // 00 bg mediumblue bg mediumblue
   0x412E6, 0x582E6, 0x001F6, // 10 fg yellow bg mediumblue
   0x865FB, 0x8659F, 0xFDA00, // 01 bg mediumblue fg yellow
   0x866E6, 0x866E6, 0x425F6, // 11 fg yellow fg yellow
   0x411FB, 0x5819F, 0xBFE00, // 00 bg mediumblue bg mediumblue
   0x411DE, 0xE7E07, 0x00177, // 10 fg aqua bg mediumblue
   0x485FB, 0x01D9F, 0xDDE00, // 01 bg mediumblue fg aqua
   0x485DE, 0xBE207, 0x62177, // 11 fg aqua fg aqua
   0x411FB, 0x5819F, 0xBFE00, // 00 bg mediumblue bg mediumblue
   0xFEE00, 0xE7E00, 0xBFE00, // 10 fg white bg mediumblue
   0x001FB, 0x0019F, 0xBFE00, // 01 bg mediumblue fg white
   0xBFE00, 0xBFE00, 0xBFE00, // 11 fg white fg white
   0xB3233, 0xB3233, 0xBFE00, // 00 bg lightblue bg lightblue
   0xB3100, 0xB3100, 0xBFD00, // 10 fg black bg lightblue
   0x7FE33, 0x7FE33, 0x7FE00, // 01 bg lightblue fg black
   0x7FD00, 0x7FD00, 0x7FD00, // 11 fg black fg black
   0xB3233, 0xB3233, 0xBFE00, // 00 bg lightblue bg lightblue
   0xB32DE, 0xB3107, 0x00277, // 10 fg magenta bg lightblue
   0x88633, 0x7E233, 0x1DE00, // 01 bg lightblue fg magenta
   0x886DE, 0x7E107, 0xA2277, // 11 fg magenta fg magenta
   0xB3233, 0xB3233, 0xBFE00, // 00 bg lightblue bg lightblue
   0xB31E6, 0xB31E6, 0x002F6, // 10 fg darkblue bg lightblue
   0x46633, 0x46633, 0x3DA00, // 01 bg lightblue fg darkblue
   0x465E6, 0x465E6, 0x826F6, // 11 fg darkblue fg darkblue
   0xB3233, 0xB3233, 0xBFE00, // 00 bg lightblue bg lightblue
   0xB32E8, 0xB31EC, 0xBFE00, // 10 fg purple bg lightblue
   0x85E33, 0x44E33, 0xBFE00, // 01 bg lightblue fg purple
   0x85EE8, 0x44DEC, 0xBFE00, // 11 fg purple fg purple
   0xB3233, 0xB3233, 0xBFE00, // 00 bg lightblue bg lightblue
   0xB3100, 0xB3180, 0xBFD00, // 10 fg darkgreen bg lightblue
   0x7FE33, 0x5FE33, 0x7FE00, // 01 bg lightblue fg darkgreen
   0x7FD00, 0x5FD80, 0x7FD00, // 11 fg darkgreen fg darkgreen
   0xB3233, 0xB3233, 0xBFE00, // 00 bg lightblue bg lightblue
   0xB3180, 0xB3180, 0xBFD80, // 10 fg gray1 (dark gray) bg lightblue
   0x5FE33, 0x5FE33, 0x5FE00, // 01 bg lightblue fg gray1 (dark gray)
   0x5FD80, 0x5FD80, 0x5FD80, // 11 fg gray1 (dark gray) fg gray1 (dark gray)
   0xB3233, 0xB3233, 0xBFE00, // 00 bg lightblue bg lightblue
   0xB31FB, 0xB319F, 0xBFE00, // 10 fg mediumblue bg lightblue
   0x41233, 0x58233, 0xBFE00, // 01 bg lightblue fg mediumblue
   0x411FB, 0x5819F, 0xBFE00, // 11 fg mediumblue fg mediumblue
   0xB3233, 0xB3233, 0xBFE00, // 00 bg lightblue bg lightblue
   0xB3233, 0xB3233, 0xBFE00, // 10 fg lightblue bg lightblue
   0xB3233, 0xB3233, 0xBFE00, // 01 bg lightblue fg lightblue
   0xB3233, 0xB3233, 0xBFE00, // 11 fg lightblue fg lightblue
   0xB3233, 0xB3233, 0xBFE00, // 00 bg lightblue bg lightblue
   0xB3133, 0xB3133, 0xBFD00, // 10 fg brown bg lightblue
   0x73233, 0x73233, 0x7FE00, // 01 bg lightblue fg brown
   0x73133, 0x73133, 0x7FD00, // 11 fg brown fg brown
   0xB3233, 0xB3233, 0xBFE00, // 00 bg lightblue bg lightblue
   0xB32FB, 0xB329F, 0xBFD00, // 10 fg orange bg lightblue
   0x81233, 0x98233, 0x7FE00, // 01 bg lightblue fg orange
   0x812FB, 0x9829F, 0x7FD00, // 11 fg orange fg orange
   0xB3233, 0xB3233, 0xBFE00, // 00 bg lightblue bg lightblue
   0xB3140, 0xB3140, 0xBFD40, // 10 fg gray2 (light gray) bg lightblue
   0x6FE33, 0x6FE33, 0x6FE00, // 01 bg lightblue fg gray2 (light gray)
   0x6FD40, 0x6FD40, 0x6FD40, // 11 fg gray2 (light gray) fg gray2 (light gray)
   0xB3233, 0xB3233, 0xBFE00, // 00 bg lightblue bg lightblue
   0xB3200, 0xB3187, 0x002F6, // 10 fg pink bg lightblue
   0xBFE33, 0x5E233, 0x3DA00, // 01 bg lightblue fg pink
   0xBFE00, 0x5E187, 0x826F6, // 11 fg pink fg pink
   0xB3233, 0xB3233, 0xBFE00, // 00 bg lightblue bg lightblue
   0xB31E8, 0xB32EC, 0xBFD00, // 10 fg green bg lightblue
   0x45E33, 0x84E33, 0x7FE00, // 01 bg lightblue fg green
   0x45DE8, 0x84EEC, 0x7FD00, // 11 fg green fg green
   0xB3233, 0xB3233, 0xBFE00, // 00 bg lightblue bg lightblue
   0xB32E6, 0xB32E6, 0x001F6, // 10 fg yellow bg lightblue
   0x86633, 0x86633, 0xFDA00, // 01 bg lightblue fg yellow
   0x866E6, 0x866E6, 0x425F6, // 11 fg yellow fg yellow
   0xB3233, 0xB3233, 0xBFE00, // 00 bg lightblue bg lightblue
   0xB31DE, 0xB3207, 0x00177, // 10 fg aqua bg lightblue
   0x48633, 0xBE233, 0xDDE00, // 01 bg lightblue fg aqua
   0x485DE, 0xBE207, 0x62177, // 11 fg aqua fg aqua
   0xB3233, 0xB3233, 0xBFE00, // 00 bg lightblue bg lightblue
   0xB3200, 0xB3200, 0xBFE00, // 10 fg white bg lightblue
   0xBFE33, 0xBFE33, 0xBFE00, // 01 bg lightblue fg white
   0xBFE00, 0xBFE00, 0xBFE00, // 11 fg white fg white
   0x73133, 0x73133, 0x7FD00, // 00 bg brown bg brown
   0x73100, 0x73100, 0x7FD00, // 10 fg black bg brown
   0x7FD33, 0x7FD33, 0x7FD00, // 01 bg brown fg black
   0x7FD00, 0x7FD00, 0x7FD00, // 11 fg black fg black
   0x73133, 0x73133, 0x7FD00, // 00 bg brown bg brown
   0x732DE, 0x73107, 0xC0277, // 10 fg magenta bg brown
   0x88533, 0x7E133, 0x1DD00, // 01 bg brown fg magenta
   0x886DE, 0x7E107, 0xA2277, // 11 fg magenta fg magenta
   0x73133, 0x73133, 0x7FD00, // 00 bg brown bg brown
   0x731E6, 0x731E6, 0xC02F6, // 10 fg darkblue bg brown
   0x46533, 0x46533, 0x3D900, // 01 bg brown fg darkblue
   0x465E6, 0x465E6, 0x826F6, // 11 fg darkblue fg darkblue
   0x73133, 0x73133, 0x7FD00, // 00 bg brown bg brown
   0x732E8, 0x731EC, 0x7FE00, // 10 fg purple bg brown
   0x85D33, 0x44D33, 0xBFD00, // 01 bg brown fg purple
   0x85EE8, 0x44DEC, 0xBFE00, // 11 fg purple fg purple
   0x73133, 0x73133, 0x7FD00, // 00 bg brown bg brown
   0x73100, 0x73180, 0x7FD00, // 10 fg darkgreen bg brown
   0x7FD33, 0x5FD33, 0x7FD00, // 01 bg brown fg darkgreen
   0x7FD00, 0x5FD80, 0x7FD00, // 11 fg darkgreen fg darkgreen
   0x73133, 0x73133, 0x7FD00, // 00 bg brown bg brown
   0x73180, 0x73180, 0x7FD80, // 10 fg gray1 (dark gray) bg brown
   0x5FD33, 0x5FD33, 0x5FD00, // 01 bg brown fg gray1 (dark gray)
   0x5FD80, 0x5FD80, 0x5FD80, // 11 fg gray1 (dark gray) fg gray1 (dark gray)
   0x73133, 0x73133, 0x7FD00, // 00 bg brown bg brown
   0x731FB, 0x7319F, 0x7FE00, // 10 fg mediumblue bg brown
   0x41133, 0x58133, 0xBFD00, // 01 bg brown fg mediumblue
   0x411FB, 0x5819F, 0xBFE00, // 11 fg mediumblue fg mediumblue
   0x73133, 0x73133, 0x7FD00, // 00 bg brown bg brown
   0x73233, 0x73233, 0x7FE00, // 10 fg lightblue bg brown
   0xB3133, 0xB3133, 0xBFD00, // 01 bg brown fg lightblue
   0xB3233, 0xB3233, 0xBFE00, // 11 fg lightblue fg lightblue
   0x73133, 0x73133, 0x7FD00, // 00 bg brown bg brown
   0x73133, 0x73133, 0x7FD00, // 10 fg brown bg brown
   0x73133, 0x73133, 0x7FD00, // 01 bg brown fg brown
   0x73133, 0x73133, 0x7FD00, // 11 fg brown fg brown
   0x73133, 0x73133, 0x7FD00, // 00 bg brown bg brown
   0x732FB, 0x7329F, 0x7FD00, // 10 fg orange bg brown
   0x81133, 0x98133, 0x7FD00, // 01 bg brown fg orange
   0x812FB, 0x9829F, 0x7FD00, // 11 fg orange fg orange
   0x73133, 0x73133, 0x7FD00, // 00 bg brown bg brown
   0x73140, 0x73140, 0x7FD40, // 10 fg gray2 (light gray) bg brown
   0x6FD33, 0x6FD33, 0x6FD00, // 01 bg brown fg gray2 (light gray)
   0x6FD40, 0x6FD40, 0x6FD40, // 11 fg gray2 (light gray) fg gray2 (light gray)
   0x73133, 0x73133, 0x7FD00, // 00 bg brown bg brown
   0x73200, 0x73187, 0xC02F6, // 10 fg pink bg brown
   0xBFD33, 0x5E133, 0x3D900, // 01 bg brown fg pink
   0xBFE00, 0x5E187, 0x826F6, // 11 fg pink fg pink
   0x73133, 0x73133, 0x7FD00, // 00 bg brown bg brown
   0x731E8, 0x732EC, 0x7FD00, // 10 fg green bg brown
   0x45D33, 0x84D33, 0x7FD00, // 01 bg brown fg green
   0x45DE8, 0x84EEC, 0x7FD00, // 11 fg green fg green
   0x73133, 0x73133, 0x7FD00, // 00 bg brown bg brown
   0x732E6, 0x732E6, 0xC01F6, // 10 fg yellow bg brown
   0x86533, 0x86533, 0xFD900, // 01 bg brown fg yellow
   0x866E6, 0x866E6, 0x425F6, // 11 fg yellow fg yellow
   0x73133, 0x73133, 0x7FD00, // 00 bg brown bg brown
   0x731DE, 0x73207, 0xC0177, // 10 fg aqua bg brown
   0x48533, 0xBE133, 0xDDD00, // 01 bg brown fg aqua
   0x485DE, 0xBE207, 0x62177, // 11 fg aqua fg aqua
   0x73133, 0x73133, 0x7FD00, // 00 bg brown bg brown
   0x73200, 0x73200, 0x7FE00, // 10 fg white bg brown
   0xBFD33, 0xBFD33, 0xBFD00, // 01 bg brown fg white
   0xBFE00, 0xBFE00, 0xBFE00, // 11 fg white fg white
   0x812FB, 0x9829F, 0x7FD00, // 00 bg orange bg orange
   0x3ED00, 0x27D00, 0x7FD00, // 10 fg black bg orange
   0xC02FB, 0xC029F, 0x7FD00, // 01 bg orange fg black
   0x7FD00, 0x7FD00, 0x7FD00, // 11 fg black fg black
   0x812FB, 0x9829F, 0x7FD00, // 00 bg orange bg orange
   0x812DE, 0x27D07, 0xC0277, // 10 fg magenta bg orange
   0x886FB, 0xC1E9F, 0x1DD00, // 01 bg orange fg magenta
   0x886DE, 0x7E107, 0xA2277, // 11 fg magenta fg magenta
   0x812FB, 0x9829F, 0x7FD00, // 00 bg orange bg orange
   0x811E6, 0x981E6, 0xC02F6, // 10 fg darkblue bg orange
   0x466FB, 0x4669F, 0x3D900, // 01 bg orange fg darkblue
   0x465E6, 0x465E6, 0x826F6, // 11 fg darkblue fg darkblue
   0x812FB, 0x9829F, 0x7FD00, // 00 bg orange bg orange
   0x812E8, 0x981EC, 0x7FE00, // 10 fg purple bg orange
   0x85EFB, 0x44E9F, 0xBFD00, // 01 bg orange fg purple
   0x85EE8, 0x44DEC, 0xBFE00, // 11 fg purple fg purple
   0x812FB, 0x9829F, 0x7FD00, // 00 bg orange bg orange
   0x3ED00, 0x27D80, 0x7FD00, // 10 fg darkgreen bg orange
   0xC02FB, 0xE029F, 0x7FD00, // 01 bg orange fg darkgreen
   0x7FD00, 0x5FD80, 0x7FD00, // 11 fg darkgreen fg darkgreen
   0x812FB, 0x9829F, 0x7FD00, // 00 bg orange bg orange
   0x3ED80, 0x27D80, 0x7FD80, // 10 fg gray1 (dark gray) bg orange
   0xE02FB, 0xE029F, 0x5FD00, // 01 bg orange fg gray1 (dark gray)
   0x5FD80, 0x5FD80, 0x5FD80, // 11 fg gray1 (dark gray) fg gray1 (dark gray)
   0x812FB, 0x9829F, 0x7FD00, // 00 bg orange bg orange
   0x811FB, 0x9819F, 0x7FE00, // 10 fg mediumblue bg orange
   0x412FB, 0x5829F, 0xBFD00, // 01 bg orange fg mediumblue
   0x411FB, 0x5819F, 0xBFE00, // 11 fg mediumblue fg mediumblue
   0x812FB, 0x9829F, 0x7FD00, // 00 bg orange bg orange
   0x81233, 0x98233, 0x7FE00, // 10 fg lightblue bg orange
   0xB32FB, 0xB329F, 0xBFD00, // 01 bg orange fg lightblue
   0xB3233, 0xB3233, 0xBFE00, // 11 fg lightblue fg lightblue
   0x812FB, 0x9829F, 0x7FD00, // 00 bg orange bg orange
   0x81133, 0x98133, 0x7FD00, // 10 fg brown bg orange
   0x732FB, 0x7329F, 0x7FD00, // 01 bg orange fg brown
   0x73133, 0x73133, 0x7FD00, // 11 fg brown fg brown
   0x812FB, 0x9829F, 0x7FD00, // 00 bg orange bg orange
   0x812FB, 0x9829F, 0x7FD00, // 10 fg orange bg orange
   0x812FB, 0x9829F, 0x7FD00, // 01 bg orange fg orange
   0x812FB, 0x9829F, 0x7FD00, // 11 fg orange fg orange
   0x812FB, 0x9829F, 0x7FD00, // 00 bg orange bg orange
   0x3ED40, 0x27D40, 0x7FD40, // 10 fg gray2 (light gray) bg orange
   0xD02FB, 0xD029F, 0x6FD00, // 01 bg orange fg gray2 (light gray)
   0x6FD40, 0x6FD40, 0x6FD40, // 11 fg gray2 (light gray) fg gray2 (light gray)
   0x812FB, 0x9829F, 0x7FD00, // 00 bg orange bg orange
   0x3EE00, 0x98187, 0xC02F6, // 10 fg pink bg orange
   0x002FB, 0x5E29F, 0x3D900, // 01 bg orange fg pink
   0xBFE00, 0x5E187, 0x826F6, // 11 fg pink fg pink
   0x812FB, 0x9829F, 0x7FD00, // 00 bg orange bg orange
   0x811E8, 0x982EC, 0x7FD00, // 10 fg green bg orange
   0x45EFB, 0x84E9F, 0x7FD00, // 01 bg orange fg green
   0x45DE8, 0x84EEC, 0x7FD00, // 11 fg green fg green
   0x812FB, 0x9829F, 0x7FD00, // 00 bg orange bg orange
   0x812E6, 0x982E6, 0xC01F6, // 10 fg yellow bg orange
   0x866FB, 0x8669F, 0xFD900, // 01 bg orange fg yellow
   0x866E6, 0x866E6, 0x425F6, // 11 fg yellow fg yellow
   0x812FB, 0x9829F, 0x7FD00, // 00 bg orange bg orange
   0x811DE, 0x27E07, 0xC0177, // 10 fg aqua bg orange
   0x486FB, 0x01E9F, 0xDDD00, // 01 bg orange fg aqua
   0x485DE, 0xBE207, 0x62177, // 11 fg aqua fg aqua
   0x812FB, 0x9829F, 0x7FD00, // 00 bg orange bg orange
   0x3EE00, 0x27E00, 0x7FE00, // 10 fg white bg orange
   0x002FB, 0x0029F, 0xBFD00, // 01 bg orange fg white
   0xBFE00, 0xBFE00, 0xBFE00, // 11 fg white fg white
   0x6FD40, 0x6FD40, 0x6FD40, // 00 bg gray2 (light gray) bg gray2 (light gray)
   0x6FD00, 0x6FD00, 0x6FD00, // 10 fg black bg gray2 (light gray)
   0x7FD40, 0x7FD40, 0x7FD40, // 01 bg gray2 (light gray) fg black
   0x7FD00, 0x7FD00, 0x7FD00, // 11 fg black fg black
   0x6FD40, 0x6FD40, 0x6FD40, // 00 bg gray2 (light gray) bg gray2 (light gray)
   0xD02DE, 0x6FD07, 0xD0277, // 10 fg magenta bg gray2 (light gray)
   0x37940, 0x7E140, 0x1DD40, // 01 bg gray2 (light gray) fg magenta
   0x886DE, 0x7E107, 0xA2277, // 11 fg magenta fg magenta
   0x6FD40, 0x6FD40, 0x6FD40, // 00 bg gray2 (light gray) bg gray2 (light gray)
   0xD01E6, 0xD01E6, 0xD02F6, // 10 fg darkblue bg gray2 (light gray)
   0xF9940, 0xF9940, 0x3D940, // 01 bg gray2 (light gray) fg darkblue
   0x465E6, 0x465E6, 0x826F6, // 11 fg darkblue fg darkblue
   0x6FD40, 0x6FD40, 0x6FD40, // 00 bg gray2 (light gray) bg gray2 (light gray)
   0x6FEE8, 0xD01EC, 0x6FE00, // 10 fg purple bg gray2 (light gray)
   0x85D40, 0xFB140, 0xBFD40, // 01 bg gray2 (light gray) fg purple
   0x85EE8, 0x44DEC, 0xBFE00, // 11 fg purple fg purple
   0x6FD40, 0x6FD40, 0x6FD40, // 00 bg gray2 (light gray) bg gray2 (light gray)
   0x6FD00, 0x6FD80, 0x6FD00, // 10 fg darkgreen bg gray2 (light gray)
   0x7FD40, 0x5FD40, 0x7FD40, // 01 bg gray2 (light gray) fg darkgreen
   0x7FD00, 0x5FD80, 0x7FD00, // 11 fg darkgreen fg darkgreen
   0x6FD40, 0x6FD40, 0x6FD40, // 00 bg gray2 (light gray) bg gray2 (light gray)
   0x6FD80, 0x6FD80, 0x6FD80, // 10 fg gray1 (dark gray) bg gray2 (light gray)
   0x5FD40, 0x5FD40, 0x5FD40, // 01 bg gray2 (light gray) fg gray1 (dark gray)
   0x5FD80, 0x5FD80, 0x5FD80, // 11 fg gray1 (dark gray) fg gray1 (dark gray)
   0x6FD40, 0x6FD40, 0x6FD40, // 00 bg gray2 (light gray) bg gray2 (light gray)
   0xD01FB, 0xD019F, 0x6FE00, // 10 fg mediumblue bg gray2 (light gray)
   0xFED40, 0xE7D40, 0xBFD40, // 01 bg gray2 (light gray) fg mediumblue
   0x411FB, 0x5819F, 0xBFE00, // 11 fg mediumblue fg mediumblue
   0x6FD40, 0x6FD40, 0x6FD40, // 00 bg gray2 (light gray) bg gray2 (light gray)
   0x6FE33, 0x6FE33, 0x6FE00, // 10 fg lightblue bg gray2 (light gray)
   0xB3140, 0xB3140, 0xBFD40, // 01 bg gray2 (light gray) fg lightblue
   0xB3233, 0xB3233, 0xBFE00, // 11 fg lightblue fg lightblue
   0x6FD40, 0x6FD40, 0x6FD40, // 00 bg gray2 (light gray) bg gray2 (light gray)
   0x6FD33, 0x6FD33, 0x6FD00, // 10 fg brown bg gray2 (light gray)
   0x73140, 0x73140, 0x7FD40, // 01 bg gray2 (light gray) fg brown
   0x73133, 0x73133, 0x7FD00, // 11 fg brown fg brown
   0x6FD40, 0x6FD40, 0x6FD40, // 00 bg gray2 (light gray) bg gray2 (light gray)
   0xD02FB, 0xD029F, 0x6FD00, // 10 fg orange bg gray2 (light gray)
   0x3ED40, 0x27D40, 0x7FD40, // 01 bg gray2 (light gray) fg orange
   0x812FB, 0x9829F, 0x7FD00, // 11 fg orange fg orange
   0x6FD40, 0x6FD40, 0x6FD40, // 00 bg gray2 (light gray) bg gray2 (light gray)
   0x6FD40, 0x6FD40, 0x6FD40, // 10 fg gray2 (light gray) bg gray2 (light gray)
   0x6FD40, 0x6FD40, 0x6FD40, // 01 bg gray2 (light gray) fg gray2 (light gray)
   0x6FD40, 0x6FD40, 0x6FD40, // 11 fg gray2 (light gray) fg gray2 (light gray)
   0x6FD40, 0x6FD40, 0x6FD40, // 00 bg gray2 (light gray) bg gray2 (light gray)
   0x6FE00, 0x6FD87, 0xD02F6, // 10 fg pink bg gray2 (light gray)
   0xBFD40, 0x5E140, 0x3D940, // 01 bg gray2 (light gray) fg pink
   0xBFE00, 0x5E187, 0x826F6, // 11 fg pink fg pink
   0x6FD40, 0x6FD40, 0x6FD40, // 00 bg gray2 (light gray) bg gray2 (light gray)
   0x6FDE8, 0xD02EC, 0x6FD00, // 10 fg green bg gray2 (light gray)
   0x45D40, 0x3B140, 0x7FD40, // 01 bg gray2 (light gray) fg green
   0x45DE8, 0x84EEC, 0x7FD00, // 11 fg green fg green
   0x6FD40, 0x6FD40, 0x6FD40, // 00 bg gray2 (light gray) bg gray2 (light gray)
   0xD02E6, 0xD02E6, 0xD01F6, // 10 fg yellow bg gray2 (light gray)
   0x39940, 0x39940, 0xFD940, // 01 bg gray2 (light gray) fg yellow
   0x866E6, 0x866E6, 0x425F6, // 11 fg yellow fg yellow
   0x6FD40, 0x6FD40, 0x6FD40, // 00 bg gray2 (light gray) bg gray2 (light gray)
   0xD01DE, 0x6FE07, 0xD0177, // 10 fg aqua bg gray2 (light gray)
   0xF7940, 0xBE140, 0xDDD40, // 01 bg gray2 (light gray) fg aqua
   0x485DE, 0xBE207, 0x62177, // 11 fg aqua fg aqua
   0x6FD40, 0x6FD40, 0x6FD40, // 00 bg gray2 (light gray) bg gray2 (light gray)
   0x6FE00, 0x6FE00, 0x6FE00, // 10 fg white bg gray2 (light gray)
   0xBFD40, 0xBFD40, 0xBFD40, // 01 bg gray2 (light gray) fg white
   0xBFE00, 0xBFE00, 0xBFE00, // 11 fg white fg white
   0xBFE00, 0x5E187, 0x826F6, // 00 bg pink bg pink
   0xBFD00, 0x5E100, 0x3D900, // 10 fg black bg pink
   0x7FE00, 0x7FD87, 0xC02F6, // 01 bg pink fg black
   0x7FD00, 0x7FD00, 0x7FD00, // 11 fg black fg black
   0xBFE00, 0x5E187, 0x826F6, // 00 bg pink bg pink
   0x002DE, 0x5E107, 0x82677, // 10 fg magenta bg pink
   0x37A00, 0x7E187, 0xA22F6, // 01 bg pink fg magenta
   0x886DE, 0x7E107, 0xA2277, // 11 fg magenta fg magenta
   0xBFE00, 0x5E187, 0x826F6, // 00 bg pink bg pink
   0x001E6, 0x5E1E6, 0x826F6, // 10 fg darkblue bg pink
   0xF9A00, 0x46587, 0x826F6, // 01 bg pink fg darkblue
   0x465E6, 0x465E6, 0x826F6, // 11 fg darkblue fg darkblue
   0xBFE00, 0x5E187, 0x826F6, // 00 bg pink bg pink
   0xBFEE8, 0x5E1EC, 0x3DA00, // 10 fg purple bg pink
   0x85E00, 0x44D87, 0x002F6, // 01 bg pink fg purple
   0x85EE8, 0x44DEC, 0xBFE00, // 11 fg purple fg purple
   0xBFE00, 0x5E187, 0x826F6, // 00 bg pink bg pink
   0xBFD00, 0x5E180, 0x3D900, // 10 fg darkgreen bg pink
   0x7FE00, 0x5FD87, 0xC02F6, // 01 bg pink fg darkgreen
   0x7FD00, 0x5FD80, 0x7FD00, // 11 fg darkgreen fg darkgreen
   0xBFE00, 0x5E187, 0x826F6, // 00 bg pink bg pink
   0xBFD80, 0x5E180, 0x3D980, // 10 fg gray1 (dark gray) bg pink
   0x5FE00, 0x5FD87, 0xE02F6, // 01 bg pink fg gray1 (dark gray)
   0x5FD80, 0x5FD80, 0x5FD80, // 11 fg gray1 (dark gray) fg gray1 (dark gray)
   0xBFE00, 0x5E187, 0x826F6, // 00 bg pink bg pink
   0x001FB, 0x5E19F, 0x3DA00, // 10 fg mediumblue bg pink
   0xFEE00, 0x58187, 0x002F6, // 01 bg pink fg mediumblue
   0x411FB, 0x5819F, 0xBFE00, // 11 fg mediumblue fg mediumblue
   0xBFE00, 0x5E187, 0x826F6, // 00 bg pink bg pink
   0xBFE33, 0x5E233, 0x3DA00, // 10 fg lightblue bg pink
   0xB3200, 0xB3187, 0x002F6, // 01 bg pink fg lightblue
   0xB3233, 0xB3233, 0xBFE00, // 11 fg lightblue fg lightblue
   0xBFE00, 0x5E187, 0x826F6, // 00 bg pink bg pink
   0xBFD33, 0x5E133, 0x3D900, // 10 fg brown bg pink
   0x73200, 0x73187, 0xC02F6, // 01 bg pink fg brown
   0x73133, 0x73133, 0x7FD00, // 11 fg brown fg brown
   0xBFE00, 0x5E187, 0x826F6, // 00 bg pink bg pink
   0x002FB, 0x5E29F, 0x3D900, // 10 fg orange bg pink
   0x3EE00, 0x98187, 0xC02F6, // 01 bg pink fg orange
   0x812FB, 0x9829F, 0x7FD00, // 11 fg orange fg orange
   0xBFE00, 0x5E187, 0x826F6, // 00 bg pink bg pink
   0xBFD40, 0x5E140, 0x3D940, // 10 fg gray2 (light gray) bg pink
   0x6FE00, 0x6FD87, 0xD02F6, // 01 bg pink fg gray2 (light gray)
   0x6FD40, 0x6FD40, 0x6FD40, // 11 fg gray2 (light gray) fg gray2 (light gray)
   0xBFE00, 0x5E187, 0x826F6, // 00 bg pink bg pink
   0xBFE00, 0x5E187, 0x826F6, // 10 fg pink bg pink
   0xBFE00, 0x5E187, 0x826F6, // 01 bg pink fg pink
   0xBFE00, 0x5E187, 0x826F6, // 11 fg pink fg pink
   0xBFE00, 0x5E187, 0x826F6, // 00 bg pink bg pink
   0xBFDE8, 0x5E2EC, 0x3D900, // 10 fg green bg pink
   0x45E00, 0x84D87, 0xC02F6, // 01 bg pink fg green
   0x45DE8, 0x84EEC, 0x7FD00, // 11 fg green fg green
   0xBFE00, 0x5E187, 0x826F6, // 00 bg pink bg pink
   0x002E6, 0x5E2E6, 0x825F6, // 10 fg yellow bg pink
   0x39A00, 0x86587, 0x426F6, // 01 bg pink fg yellow
   0x866E6, 0x866E6, 0x425F6, // 11 fg yellow fg yellow
   0xBFE00, 0x5E187, 0x826F6, // 00 bg pink bg pink
   0x001DE, 0x5E207, 0x82577, // 10 fg aqua bg pink
   0xF7A00, 0xBE187, 0x622F6, // 01 bg pink fg aqua
   0x485DE, 0xBE207, 0x62177, // 11 fg aqua fg aqua
   0xBFE00, 0x5E187, 0x826F6, // 00 bg pink bg pink
   0xBFE00, 0x5E200, 0x3DA00, // 10 fg white bg pink
   0xBFE00, 0xBFD87, 0x002F6, // 01 bg pink fg white
   0xBFE00, 0xBFE00, 0xBFE00, // 11 fg white fg white
   0x45DE8, 0x84EEC, 0x7FD00, // 00 bg green bg green
   0x45D00, 0x3B100, 0x7FD00, // 10 fg black bg green
   0x7FDE8, 0xC02EC, 0x7FD00, // 01 bg green fg black
   0x7FD00, 0x7FD00, 0x7FD00, // 11 fg black fg black
   0x45DE8, 0x84EEC, 0x7FD00, // 00 bg green bg green
   0x45EDE, 0x3B107, 0xC0277, // 10 fg magenta bg green
   0x885E8, 0xC1EEC, 0x1DD00, // 01 bg green fg magenta
   0x886DE, 0x7E107, 0xA2277, // 11 fg magenta fg magenta
   0x45DE8, 0x84EEC, 0x7FD00, // 00 bg green bg green
   0x45DE6, 0x84DE6, 0xC02F6, // 10 fg darkblue bg green
   0x465E8, 0x466EC, 0x3D900, // 01 bg green fg darkblue
   0x465E6, 0x465E6, 0x826F6, // 11 fg darkblue fg darkblue
   0x45DE8, 0x84EEC, 0x7FD00, // 00 bg green bg green
   0x45EE8, 0x84DEC, 0x7FE00, // 10 fg purple bg green
   0x85DE8, 0x44EEC, 0xBFD00, // 01 bg green fg purple
   0x85EE8, 0x44DEC, 0xBFE00, // 11 fg purple fg purple
   0x45DE8, 0x84EEC, 0x7FD00, // 00 bg green bg green
   0x45D00, 0x3B180, 0x7FD00, // 10 fg darkgreen bg green
   0x7FDE8, 0xE02EC, 0x7FD00, // 01 bg green fg darkgreen
   0x7FD00, 0x5FD80, 0x7FD00, // 11 fg darkgreen fg darkgreen
   0x45DE8, 0x84EEC, 0x7FD00, // 00 bg green bg green
   0x45D80, 0x3B180, 0x7FD80, // 10 fg gray1 (dark gray) bg green
   0x5FDE8, 0xE02EC, 0x5FD00, // 01 bg green fg gray1 (dark gray)
   0x5FD80, 0x5FD80, 0x5FD80, // 11 fg gray1 (dark gray) fg gray1 (dark gray)
   0x45DE8, 0x84EEC, 0x7FD00, // 00 bg green bg green
   0x45DFB, 0x84D9F, 0x7FE00, // 10 fg mediumblue bg green
   0x411E8, 0x582EC, 0xBFD00, // 01 bg green fg mediumblue
   0x411FB, 0x5819F, 0xBFE00, // 11 fg mediumblue fg mediumblue
   0x45DE8, 0x84EEC, 0x7FD00, // 00 bg green bg green
   0x45E33, 0x84E33, 0x7FE00, // 10 fg lightblue bg green
   0xB31E8, 0xB32EC, 0xBFD00, // 01 bg green fg lightblue
   0xB3233, 0xB3233, 0xBFE00, // 11 fg lightblue fg lightblue
   0x45DE8, 0x84EEC, 0x7FD00, // 00 bg green bg green
   0x45D33, 0x84D33, 0x7FD00, // 10 fg brown bg green
   0x731E8, 0x732EC, 0x7FD00, // 01 bg green fg brown
   0x73133, 0x73133, 0x7FD00, // 11 fg brown fg brown
   0x45DE8, 0x84EEC, 0x7FD00, // 00 bg green bg green
   0x45EFB, 0x84E9F, 0x7FD00, // 10 fg orange bg green
   0x811E8, 0x982EC, 0x7FD00, // 01 bg green fg orange
   0x812FB, 0x9829F, 0x7FD00, // 11 fg orange fg orange
   0x45DE8, 0x84EEC, 0x7FD00, // 00 bg green bg green
   0x45D40, 0x3B140, 0x7FD40, // 10 fg gray2 (light gray) bg green
   0x6FDE8, 0xD02EC, 0x6FD00, // 01 bg green fg gray2 (light gray)
   0x6FD40, 0x6FD40, 0x6FD40, // 11 fg gray2 (light gray) fg gray2 (light gray)
   0x45DE8, 0x84EEC, 0x7FD00, // 00 bg green bg green
   0x45E00, 0x84D87, 0xC02F6, // 10 fg pink bg green
   0xBFDE8, 0x5E2EC, 0x3D900, // 01 bg green fg pink
   0xBFE00, 0x5E187, 0x826F6, // 11 fg pink fg pink
   0x45DE8, 0x84EEC, 0x7FD00, // 00 bg green bg green
   0x45DE8, 0x84EEC, 0x7FD00, // 10 fg green bg green
   0x45DE8, 0x84EEC, 0x7FD00, // 01 bg green fg green
   0x45DE8, 0x84EEC, 0x7FD00, // 11 fg green fg green
   0x45DE8, 0x84EEC, 0x7FD00, // 00 bg green bg green
   0x45EE6, 0x84EE6, 0xC01F6, // 10 fg yellow bg green
   0x865E8, 0x866EC, 0xFD900, // 01 bg green fg yellow
   0x866E6, 0x866E6, 0x425F6, // 11 fg yellow fg yellow
   0x45DE8, 0x84EEC, 0x7FD00, // 00 bg green bg green
   0x45DDE, 0x3B207, 0xC0177, // 10 fg aqua bg green
   0x485E8, 0x01EEC, 0xDDD00, // 01 bg green fg aqua
   0x485DE, 0xBE207, 0x62177, // 11 fg aqua fg aqua
   0x45DE8, 0x84EEC, 0x7FD00, // 00 bg green bg green
   0x45E00, 0x3B200, 0x7FE00, // 10 fg white bg green
   0xBFDE8, 0x002EC, 0xBFD00, // 01 bg green fg white
   0xBFE00, 0xBFE00, 0xBFE00, // 11 fg white fg white
   0x866E6, 0x866E6, 0x425F6, // 00 bg yellow bg yellow
   0x39900, 0x39900, 0xFD900, // 10 fg black bg yellow
   0xC02E6, 0xC02E6, 0xC01F6, // 01 bg yellow fg black
   0x7FD00, 0x7FD00, 0x7FD00, // 11 fg black fg black
   0x866E6, 0x866E6, 0x425F6, // 00 bg yellow bg yellow
   0x866DE, 0x39907, 0x42677, // 10 fg magenta bg yellow
   0x886E6, 0xC1EE6, 0xA21F6, // 01 bg yellow fg magenta
   0x886DE, 0x7E107, 0xA2277, // 11 fg magenta fg magenta
   0x866E6, 0x866E6, 0x425F6, // 00 bg yellow bg yellow
   0x865E6, 0x865E6, 0x426F6, // 10 fg darkblue bg yellow
   0x466E6, 0x466E6, 0x825F6, // 01 bg yellow fg darkblue
   0x465E6, 0x465E6, 0x826F6, // 11 fg darkblue fg darkblue
   0x866E6, 0x866E6, 0x425F6, // 00 bg yellow bg yellow
   0x866E8, 0x865EC, 0xFDA00, // 10 fg purple bg yellow
   0x85EE6, 0x44EE6, 0x001F6, // 01 bg yellow fg purple
   0x85EE8, 0x44DEC, 0xBFE00, // 11 fg purple fg purple
   0x866E6, 0x866E6, 0x425F6, // 00 bg yellow bg yellow
   0x39900, 0x39980, 0xFD900, // 10 fg darkgreen bg yellow
   0xC02E6, 0xE02E6, 0xC01F6, // 01 bg yellow fg darkgreen
   0x7FD00, 0x5FD80, 0x7FD00, // 11 fg darkgreen fg darkgreen
   0x866E6, 0x866E6, 0x425F6, // 00 bg yellow bg yellow
   0x39980, 0x39980, 0xFD980, // 10 fg gray1 (dark gray) bg yellow
   0xE02E6, 0xE02E6, 0xE01F6, // 01 bg yellow fg gray1 (dark gray)
   0x5FD80, 0x5FD80, 0x5FD80, // 11 fg gray1 (dark gray) fg gray1 (dark gray)
   0x866E6, 0x866E6, 0x425F6, // 00 bg yellow bg yellow
   0x865FB, 0x8659F, 0xFDA00, // 10 fg mediumblue bg yellow
   0x412E6, 0x582E6, 0x001F6, // 01 bg yellow fg mediumblue
   0x411FB, 0x5819F, 0xBFE00, // 11 fg mediumblue fg mediumblue
   0x866E6, 0x866E6, 0x425F6, // 00 bg yellow bg yellow
   0x86633, 0x86633, 0xFDA00, // 10 fg lightblue bg yellow
   0xB32E6, 0xB32E6, 0x001F6, // 01 bg yellow fg lightblue
   0xB3233, 0xB3233, 0xBFE00, // 11 fg lightblue fg lightblue
   0x866E6, 0x866E6, 0x425F6, // 00 bg yellow bg yellow
   0x86533, 0x86533, 0xFD900, // 10 fg brown bg yellow
   0x732E6, 0x732E6, 0xC01F6, // 01 bg yellow fg brown
   0x73133, 0x73133, 0x7FD00, // 11 fg brown fg brown
   0x866E6, 0x866E6, 0x425F6, // 00 bg yellow bg yellow
   0x866FB, 0x8669F, 0xFD900, // 10 fg orange bg yellow
   0x812E6, 0x982E6, 0xC01F6, // 01 bg yellow fg orange
   0x812FB, 0x9829F, 0x7FD00, // 11 fg orange fg orange
   0x866E6, 0x866E6, 0x425F6, // 00 bg yellow bg yellow
   0x39940, 0x39940, 0xFD940, // 10 fg gray2 (light gray) bg yellow
   0xD02E6, 0xD02E6, 0xD01F6, // 01 bg yellow fg gray2 (light gray)
   0x6FD40, 0x6FD40, 0x6FD40, // 11 fg gray2 (light gray) fg gray2 (light gray)
   0x866E6, 0x866E6, 0x425F6, // 00 bg yellow bg yellow
   0x39A00, 0x86587, 0x426F6, // 10 fg pink bg yellow
   0x002E6, 0x5E2E6, 0x825F6, // 01 bg yellow fg pink
   0xBFE00, 0x5E187, 0x826F6, // 11 fg pink fg pink
   0x866E6, 0x866E6, 0x425F6, // 00 bg yellow bg yellow
   0x865E8, 0x866EC, 0xFD900, // 10 fg green bg yellow
   0x45EE6, 0x84EE6, 0xC01F6, // 01 bg yellow fg green
   0x45DE8, 0x84EEC, 0x7FD00, // 11 fg green fg green
   0x866E6, 0x866E6, 0x425F6, // 00 bg yellow bg yellow
   0x866E6, 0x866E6, 0x425F6, // 10 fg yellow bg yellow
   0x866E6, 0x866E6, 0x425F6, // 01 bg yellow fg yellow
   0x866E6, 0x866E6, 0x425F6, // 11 fg yellow fg yellow
   0x866E6, 0x866E6, 0x425F6, // 00 bg yellow bg yellow
   0x865DE, 0x39A07, 0x42577, // 10 fg aqua bg yellow
   0x486E6, 0x01EE6, 0x621F6, // 01 bg yellow fg aqua
   0x485DE, 0xBE207, 0x62177, // 11 fg aqua fg aqua
   0x866E6, 0x866E6, 0x425F6, // 00 bg yellow bg yellow
   0x39A00, 0x39A00, 0xFDA00, // 10 fg white bg yellow
   0x002E6, 0x002E6, 0x001F6, // 01 bg yellow fg white
   0xBFE00, 0xBFE00, 0xBFE00, // 11 fg white fg white
   0x485DE, 0xBE207, 0x62177, // 00 bg aqua bg aqua
   0xF7900, 0xBE100, 0xDDD00, // 10 fg black bg aqua
   0xC01DE, 0x7FE07, 0xC0177, // 01 bg aqua fg black
   0x7FD00, 0x7FD00, 0x7FD00, // 11 fg black fg black
   0x485DE, 0xBE207, 0x62177, // 00 bg aqua bg aqua
   0x486DE, 0xBE107, 0x62277, // 10 fg magenta bg aqua
   0x885DE, 0x7E207, 0xA2177, // 01 bg aqua fg magenta
   0x886DE, 0x7E107, 0xA2277, // 11 fg magenta fg magenta
   0x485DE, 0xBE207, 0x62177, // 00 bg aqua bg aqua
   0x485E6, 0x01DE6, 0x622F6, // 10 fg darkblue bg aqua
   0x465DE, 0xF9A07, 0x82577, // 01 bg aqua fg darkblue
   0x465E6, 0x465E6, 0x826F6, // 11 fg darkblue fg darkblue
   0x485DE, 0xBE207, 0x62177, // 00 bg aqua bg aqua
   0x486E8, 0x01DEC, 0xDDE00, // 10 fg purple bg aqua
   0x85DDE, 0xFB207, 0x00177, // 01 bg aqua fg purple
   0x85EE8, 0x44DEC, 0xBFE00, // 11 fg purple fg purple
   0x485DE, 0xBE207, 0x62177, // 00 bg aqua bg aqua
   0xF7900, 0xBE180, 0xDDD00, // 10 fg darkgreen bg aqua
   0xC01DE, 0x5FE07, 0xC0177, // 01 bg aqua fg darkgreen
   0x7FD00, 0x5FD80, 0x7FD00, // 11 fg darkgreen fg darkgreen
   0x485DE, 0xBE207, 0x62177, // 00 bg aqua bg aqua
   0xF7980, 0xBE180, 0xDDD80, // 10 fg gray1 (dark gray) bg aqua
   0xE01DE, 0x5FE07, 0xE0177, // 01 bg aqua fg gray1 (dark gray)
   0x5FD80, 0x5FD80, 0x5FD80, // 11 fg gray1 (dark gray) fg gray1 (dark gray)
   0x485DE, 0xBE207, 0x62177, // 00 bg aqua bg aqua
   0x485FB, 0x01D9F, 0xDDE00, // 10 fg mediumblue bg aqua
   0x411DE, 0xE7E07, 0x00177, // 01 bg aqua fg mediumblue
   0x411FB, 0x5819F, 0xBFE00, // 11 fg mediumblue fg mediumblue
   0x485DE, 0xBE207, 0x62177, // 00 bg aqua bg aqua
   0x48633, 0xBE233, 0xDDE00, // 10 fg lightblue bg aqua
   0xB31DE, 0xB3207, 0x00177, // 01 bg aqua fg lightblue
   0xB3233, 0xB3233, 0xBFE00, // 11 fg lightblue fg lightblue
   0x485DE, 0xBE207, 0x62177, // 00 bg aqua bg aqua
   0x48533, 0xBE133, 0xDDD00, // 10 fg brown bg aqua
   0x731DE, 0x73207, 0xC0177, // 01 bg aqua fg brown
   0x73133, 0x73133, 0x7FD00, // 11 fg brown fg brown
   0x485DE, 0xBE207, 0x62177, // 00 bg aqua bg aqua
   0x486FB, 0x01E9F, 0xDDD00, // 10 fg orange bg aqua
   0x811DE, 0x27E07, 0xC0177, // 01 bg aqua fg orange
   0x812FB, 0x9829F, 0x7FD00, // 11 fg orange fg orange
   0x485DE, 0xBE207, 0x62177, // 00 bg aqua bg aqua
   0xF7940, 0xBE140, 0xDDD40, // 10 fg gray2 (light gray) bg aqua
   0xD01DE, 0x6FE07, 0xD0177, // 01 bg aqua fg gray2 (light gray)
   0x6FD40, 0x6FD40, 0x6FD40, // 11 fg gray2 (light gray) fg gray2 (light gray)
   0x485DE, 0xBE207, 0x62177, // 00 bg aqua bg aqua
   0xF7A00, 0xBE187, 0x622F6, // 10 fg pink bg aqua
   0x001DE, 0x5E207, 0x82577, // 01 bg aqua fg pink
   0xBFE00, 0x5E187, 0x826F6, // 11 fg pink fg pink
   0x485DE, 0xBE207, 0x62177, // 00 bg aqua bg aqua
   0x485E8, 0x01EEC, 0xDDD00, // 10 fg green bg aqua
   0x45DDE, 0x3B207, 0xC0177, // 01 bg aqua fg green
   0x45DE8, 0x84EEC, 0x7FD00, // 11 fg green fg green
   0x485DE, 0xBE207, 0x62177, // 00 bg aqua bg aqua
   0x486E6, 0x01EE6, 0x621F6, // 10 fg yellow bg aqua
   0x865DE, 0x39A07, 0x42577, // 01 bg aqua fg yellow
   0x866E6, 0x866E6, 0x425F6, // 11 fg yellow fg yellow
   0x485DE, 0xBE207, 0x62177, // 00 bg aqua bg aqua
   0x485DE, 0xBE207, 0x62177, // 10 fg aqua bg aqua
   0x485DE, 0xBE207, 0x62177, // 01 bg aqua fg aqua
   0x485DE, 0xBE207, 0x62177, // 11 fg aqua fg aqua
   0x485DE, 0xBE207, 0x62177, // 00 bg aqua bg aqua
   0xF7A00, 0xBE200, 0xDDE00, // 10 fg white bg aqua
   0x001DE, 0xBFE07, 0x00177, // 01 bg aqua fg white
   0xBFE00, 0xBFE00, 0xBFE00, // 11 fg white fg white
   0xBFE00, 0xBFE00, 0xBFE00, // 00 bg white bg white
   0xBFD00, 0xBFD00, 0xBFD00, // 10 fg black bg white
   0x7FE00, 0x7FE00, 0x7FE00, // 01 bg white fg black
   0x7FD00, 0x7FD00, 0x7FD00, // 11 fg black fg black
   0xBFE00, 0xBFE00, 0xBFE00, // 00 bg white bg white
   0x002DE, 0xBFD07, 0x00277, // 10 fg magenta bg white
   0x37A00, 0x7E200, 0x1DE00, // 01 bg white fg magenta
   0x886DE, 0x7E107, 0xA2277, // 11 fg magenta fg magenta
   0xBFE00, 0xBFE00, 0xBFE00, // 00 bg white bg white
   0x001E6, 0x001E6, 0x002F6, // 10 fg darkblue bg white
   0xF9A00, 0xF9A00, 0x3DA00, // 01 bg white fg darkblue
   0x465E6, 0x465E6, 0x826F6, // 11 fg darkblue fg darkblue
   0xBFE00, 0xBFE00, 0xBFE00, // 00 bg white bg white
   0xBFEE8, 0x001EC, 0xBFE00, // 10 fg purple bg white
   0x85E00, 0xFB200, 0xBFE00, // 01 bg white fg purple
   0x85EE8, 0x44DEC, 0xBFE00, // 11 fg purple fg purple
   0xBFE00, 0xBFE00, 0xBFE00, // 00 bg white bg white
   0xBFD00, 0xBFD80, 0xBFD00, // 10 fg darkgreen bg white
   0x7FE00, 0x5FE00, 0x7FE00, // 01 bg white fg darkgreen
   0x7FD00, 0x5FD80, 0x7FD00, // 11 fg darkgreen fg darkgreen
   0xBFE00, 0xBFE00, 0xBFE00, // 00 bg white bg white
   0xBFD80, 0xBFD80, 0xBFD80, // 10 fg gray1 (dark gray) bg white
   0x5FE00, 0x5FE00, 0x5FE00, // 01 bg white fg gray1 (dark gray)
   0x5FD80, 0x5FD80, 0x5FD80, // 11 fg gray1 (dark gray) fg gray1 (dark gray)
   0xBFE00, 0xBFE00, 0xBFE00, // 00 bg white bg white
   0x001FB, 0x0019F, 0xBFE00, // 10 fg mediumblue bg white
   0xFEE00, 0xE7E00, 0xBFE00, // 01 bg white fg mediumblue
   0x411FB, 0x5819F, 0xBFE00, // 11 fg mediumblue fg mediumblue
   0xBFE00, 0xBFE00, 0xBFE00, // 00 bg white bg white
   0xBFE33, 0xBFE33, 0xBFE00, // 10 fg lightblue bg white
   0xB3200, 0xB3200, 0xBFE00, // 01 bg white fg lightblue
   0xB3233, 0xB3233, 0xBFE00, // 11 fg lightblue fg lightblue
   0xBFE00, 0xBFE00, 0xBFE00, // 00 bg white bg white
   0xBFD33, 0xBFD33, 0xBFD00, // 10 fg brown bg white
   0x73200, 0x73200, 0x7FE00, // 01 bg white fg brown
   0x73133, 0x73133, 0x7FD00, // 11 fg brown fg brown
   0xBFE00, 0xBFE00, 0xBFE00, // 00 bg white bg white
   0x002FB, 0x0029F, 0xBFD00, // 10 fg orange bg white
   0x3EE00, 0x27E00, 0x7FE00, // 01 bg white fg orange
   0x812FB, 0x9829F, 0x7FD00, // 11 fg orange fg orange
   0xBFE00, 0xBFE00, 0xBFE00, // 00 bg white bg white
   0xBFD40, 0xBFD40, 0xBFD40, // 10 fg gray2 (light gray) bg white
   0x6FE00, 0x6FE00, 0x6FE00, // 01 bg white fg gray2 (light gray)
   0x6FD40, 0x6FD40, 0x6FD40, // 11 fg gray2 (light gray) fg gray2 (light gray)
   0xBFE00, 0xBFE00, 0xBFE00, // 00 bg white bg white
   0xBFE00, 0xBFD87, 0x002F6, // 10 fg pink bg white
   0xBFE00, 0x5E200, 0x3DA00, // 01 bg white fg pink
   0xBFE00, 0x5E187, 0x826F6, // 11 fg pink fg pink
   0xBFE00, 0xBFE00, 0xBFE00, // 00 bg white bg white
   0xBFDE8, 0x002EC, 0xBFD00, // 10 fg green bg white
   0x45E00, 0x3B200, 0x7FE00, // 01 bg white fg green
   0x45DE8, 0x84EEC, 0x7FD00, // 11 fg green fg green
   0xBFE00, 0xBFE00, 0xBFE00, // 00 bg white bg white
   0x002E6, 0x002E6, 0x001F6, // 10 fg yellow bg white
   0x39A00, 0x39A00, 0xFDA00, // 01 bg white fg yellow
   0x866E6, 0x866E6, 0x425F6, // 11 fg yellow fg yellow
   0xBFE00, 0xBFE00, 0xBFE00, // 00 bg white bg white
   0x001DE, 0xBFE07, 0x00177, // 10 fg aqua bg white
   0xF7A00, 0xBE200, 0xDDE00, // 01 bg white fg aqua
   0x485DE, 0xBE207, 0x62177, // 11 fg aqua fg aqua
   0xBFE00, 0xBFE00, 0xBFE00, // 00 bg white bg white
   0xBFE00, 0xBFE00, 0xBFE00, // 10 fg white bg white
   0xBFE00, 0xBFE00, 0xBFE00, // 01 bg white fg white
   0xBFE00, 0xBFE00, 0xBFE00, // 11 fg white fg white
    // tmds_lores_original
   0x7FD00, 0x7FD00, 0x7FD00, // 00 bg black bg black
   0x7FD00, 0x7FD00, 0x7FD00, // 10 fg black bg black
   0x7FD00, 0x7FD00, 0x7FD00, // 01 bg black fg black
   0x7FD00, 0x7FD00, 0x7FD00, // 11 fg black fg black
   0x7FD00, 0x7FD00, 0x7FD00, // 00 bg black bg black
   0x7FE20, 0x7FD09, 0x7FD38, // 10 fg magenta bg black
   0xB7D00, 0x7D900, 0x71D00, // 01 bg black fg magenta
   0xB7E20, 0x7D909, 0x71D38, // 11 fg magenta fg magenta
   0x7FD00, 0x7FD00, 0x7FD00, // 00 bg black bg black
   0x7FD38, 0xC01EE, 0x7FE0C, // 10 fg darkblue bg black
   0x71D00, 0xFB900, 0xBCD00, // 01 bg black fg darkblue
   0x71D38, 0x445EE, 0xBCE0C, // 11 fg darkblue fg darkblue
   0x7FD00, 0x7FD00, 0x7FD00, // 00 bg black bg black
   0xC02E7, 0x7FDC1, 0x7FE00, // 10 fg purple bg black
   0x39D00, 0x4F900, 0xBFD00, // 01 bg black fg purple
   0x862E7, 0x4F9C1, 0xBFE00, // 11 fg purple fg purple
   0x7FD00, 0x7FD00, 0x7FD00, // 00 bg black bg black
   0xC01F7, 0x7FE86, 0x7FD3C, // 10 fg darkgreen bg black
   0xFDD00, 0x9E500, 0x70D00, // 01 bg black fg darkgreen
   0x421F7, 0x9E686, 0x70D3C, // 11 fg darkgreen fg darkgreen
   0x7FD00, 0x7FD00, 0x7FD00, // 00 bg black bg black
   0xC017F, 0xC017F, 0xC017F, // 10 fg gray1 (dark gray) bg black
   0xDFD00, 0xDFD00, 0xDFD00, // 01 bg black fg gray1 (dark gray)
   0x6017F, 0x6017F, 0x6017F, // 11 fg gray1 (dark gray) fg gray1 (dark gray)
   0x7FD00, 0x7FD00, 0x7FD00, // 00 bg black bg black
   0x7FE47, 0x7FD8E, 0x7FE00, // 10 fg mediumblue bg black
   0xAE100, 0x5C500, 0xBFD00, // 01 bg black fg mediumblue
   0xAE247, 0x5C58E, 0xBFE00, // 11 fg mediumblue fg mediumblue
   0x7FD00, 0x7FD00, 0x7FD00, // 00 bg black bg black
   0xC023D, 0xC02DF, 0x7FE00, // 10 fg lightblue bg black
   0x0F500, 0x37D00, 0xBFD00, // 01 bg black fg lightblue
   0xB0A3D, 0x882DF, 0xBFE00, // 11 fg lightblue fg lightblue
   0x7FD00, 0x7FD00, 0x7FD00, // 00 bg black bg black
   0xC01C7, 0x7FD23, 0x7FD00, // 10 fg brown bg black
   0xF1D00, 0x77100, 0x7FD00, // 01 bg black fg brown
   0x4E1C7, 0x77123, 0x7FD00, // 11 fg brown fg brown
   0x7FD00, 0x7FD00, 0x7FD00, // 00 bg black bg black
   0x7FE1D, 0x7FE84, 0x7FD00, // 10 fg orange bg black
   0xB8900, 0x9ED00, 0x7FD00, // 01 bg black fg orange
   0xB8A1D, 0x9EE84, 0x7FD00, // 11 fg orange fg orange
   0x7FD00, 0x7FD00, 0x7FD00, // 00 bg black bg black
   0xC017F, 0xC017F, 0xC017F, // 10 fg gray2 (light gray) bg black
   0xDFD00, 0xDFD00, 0xDFD00, // 01 bg black fg gray2 (light gray)
   0x6017F, 0x6017F, 0x6017F, // 11 fg gray2 (light gray) fg gray2 (light gray)
   0x7FD00, 0x7FD00, 0x7FD00, // 00 bg black bg black
   0xC02FC, 0x7FED0, 0x7FEC1, // 10 fg pink bg black
   0x3F100, 0x8BD00, 0x8F900, // 01 bg black fg pink
   0x80EFC, 0x8BED0, 0x8FAC1, // 11 fg pink fg pink
   0x7FD00, 0x7FD00, 0x7FD00, // 00 bg black bg black
   0x7FE41, 0x7FE11, 0x7FD00, // 10 fg green bg black
   0xAF900, 0xBB900, 0x7FD00, // 01 bg black fg green
   0xAFA41, 0xBBA11, 0x7FD00, // 11 fg green fg green
   0x7FD00, 0x7FD00, 0x7FD00, // 00 bg black bg black
   0x7FEC1, 0xC02E7, 0x7FD00, // 10 fg yellow bg black
   0x8F900, 0x39D00, 0x7FD00, // 01 bg black fg yellow
   0x8FAC1, 0x862E7, 0x7FD00, // 11 fg yellow fg yellow
   0x7FD00, 0x7FD00, 0x7FD00, // 00 bg black bg black
   0x7FE71, 0xC02F7, 0xC023D, // 10 fg aqua bg black
   0xA3900, 0x3DD00, 0x0F500, // 01 bg black fg aqua
   0xA3A71, 0x822F7, 0xB0A3D, // 11 fg aqua fg aqua
   0x7FD00, 0x7FD00, 0x7FD00, // 00 bg black bg black
   0xC02FA, 0xC02FA, 0xC02FA, // 10 fg white bg black
   0x3E900, 0x3E900, 0x3E900, // 01 bg black fg white
   0x816FA, 0x816FA, 0x816FA, // 11 fg white fg white
   0xB7E20, 0x7D909, 0x71D38, // 00 bg magenta bg magenta
   0xB7D00, 0x7D900, 0x71D00, // 10 fg black bg magenta
   0x7FE20, 0x7FD09, 0x7FD38, // 01 bg magenta fg black
   0x7FD00, 0x7FD00, 0x7FD00, // 11 fg black fg black
   0xB7E20, 0x7D909, 0x71D38, // 00 bg magenta bg magenta
   0xB7E20, 0x7D909, 0x71D38, // 10 fg magenta bg magenta
   0xB7E20, 0x7D909, 0x71D38, // 01 bg magenta fg magenta
   0xB7E20, 0x7D909, 0x71D38, // 11 fg magenta fg magenta
   0xB7E20, 0x7D909, 0x71D38, // 00 bg magenta bg magenta
   0xB7D38, 0xC25EE, 0x71E0C, // 10 fg darkblue bg magenta
   0x71E20, 0xFB909, 0xBCD38, // 01 bg magenta fg darkblue
   0x71D38, 0x445EE, 0xBCE0C, // 11 fg darkblue fg darkblue
   0xB7E20, 0x7D909, 0x71D38, // 00 bg magenta bg magenta
   0x082E7, 0x7D9C1, 0x71E00, // 10 fg purple bg magenta
   0x39E20, 0x4F909, 0xBFD38, // 01 bg magenta fg purple
   0x862E7, 0x4F9C1, 0xBFE00, // 11 fg purple fg purple
   0xB7E20, 0x7D909, 0x71D38, // 00 bg magenta bg magenta
   0x081F7, 0x7DA86, 0x71D3C, // 10 fg darkgreen bg magenta
   0xFDE20, 0x9E509, 0x70D38, // 01 bg magenta fg darkgreen
   0x421F7, 0x9E686, 0x70D3C, // 11 fg darkgreen fg darkgreen
   0xB7E20, 0x7D909, 0x71D38, // 00 bg magenta bg magenta
   0x0817F, 0xC257F, 0xCE17F, // 10 fg gray1 (dark gray) bg magenta
   0xDFE20, 0xDFD09, 0xDFD38, // 01 bg magenta fg gray1 (dark gray)
   0x6017F, 0x6017F, 0x6017F, // 11 fg gray1 (dark gray) fg gray1 (dark gray)
   0xB7E20, 0x7D909, 0x71D38, // 00 bg magenta bg magenta
   0xB7E47, 0x7D98E, 0x71E00, // 10 fg mediumblue bg magenta
   0xAE220, 0x5C509, 0xBFD38, // 01 bg magenta fg mediumblue
   0xAE247, 0x5C58E, 0xBFE00, // 11 fg mediumblue fg mediumblue
   0xB7E20, 0x7D909, 0x71D38, // 00 bg magenta bg magenta
   0x0823D, 0xC26DF, 0x71E00, // 10 fg lightblue bg magenta
   0x0F620, 0x37D09, 0xBFD38, // 01 bg magenta fg lightblue
   0xB0A3D, 0x882DF, 0xBFE00, // 11 fg lightblue fg lightblue
   0xB7E20, 0x7D909, 0x71D38, // 00 bg magenta bg magenta
   0x081C7, 0x7D923, 0x71D00, // 10 fg brown bg magenta
   0xF1E20, 0x77109, 0x7FD38, // 01 bg magenta fg brown
   0x4E1C7, 0x77123, 0x7FD00, // 11 fg brown fg brown
   0xB7E20, 0x7D909, 0x71D38, // 00 bg magenta bg magenta
   0xB7E1D, 0x7DA84, 0x71D00, // 10 fg orange bg magenta
   0xB8A20, 0x9ED09, 0x7FD38, // 01 bg magenta fg orange
   0xB8A1D, 0x9EE84, 0x7FD00, // 11 fg orange fg orange
   0xB7E20, 0x7D909, 0x71D38, // 00 bg magenta bg magenta
   0x0817F, 0xC257F, 0xCE17F, // 10 fg gray2 (light gray) bg magenta
   0xDFE20, 0xDFD09, 0xDFD38, // 01 bg magenta fg gray2 (light gray)
   0x6017F, 0x6017F, 0x6017F, // 11 fg gray2 (light gray) fg gray2 (light gray)
   0xB7E20, 0x7D909, 0x71D38, // 00 bg magenta bg magenta
   0x082FC, 0x7DAD0, 0x71EC1, // 10 fg pink bg magenta
   0x3F220, 0x8BD09, 0x8F938, // 01 bg magenta fg pink
   0x80EFC, 0x8BED0, 0x8FAC1, // 11 fg pink fg pink
   0xB7E20, 0x7D909, 0x71D38, // 00 bg magenta bg magenta
   0xB7E41, 0x7DA11, 0x71D00, // 10 fg green bg magenta
   0xAFA20, 0xBB909, 0x7FD38, // 01 bg magenta fg green
   0xAFA41, 0xBBA11, 0x7FD00, // 11 fg green fg green
   0xB7E20, 0x7D909, 0x71D38, // 00 bg magenta bg magenta
   0xB7EC1, 0xC26E7, 0x71D00, // 10 fg yellow bg magenta
   0x8FA20, 0x39D09, 0x7FD38, // 01 bg magenta fg yellow
   0x8FAC1, 0x862E7, 0x7FD00, // 11 fg yellow fg yellow
   0xB7E20, 0x7D909, 0x71D38, // 00 bg magenta bg magenta
   0xB7E71, 0xC26F7, 0xCE23D, // 10 fg aqua bg magenta
   0xA3A20, 0x3DD09, 0x0F538, // 01 bg magenta fg aqua
   0xA3A71, 0x822F7, 0xB0A3D, // 11 fg aqua fg aqua
   0xB7E20, 0x7D909, 0x71D38, // 00 bg magenta bg magenta
   0x082FA, 0xC26FA, 0xCE2FA, // 10 fg white bg magenta
   0x3EA20, 0x3E909, 0x3E938, // 01 bg magenta fg white
   0x816FA, 0x816FA, 0x816FA, // 11 fg white fg white
   0x71D38, 0x445EE, 0xBCE0C, // 00 bg darkblue bg darkblue
   0x71D00, 0xFB900, 0xBCD00, // 10 fg black bg darkblue
   0x7FD38, 0xC01EE, 0x7FE0C, // 01 bg darkblue fg black
   0x7FD00, 0x7FD00, 0x7FD00, // 11 fg black fg black
   0x71D38, 0x445EE, 0xBCE0C, // 00 bg darkblue bg darkblue
   0x71E20, 0xFB909, 0xBCD38, // 10 fg magenta bg darkblue
   0xB7D38, 0xC25EE, 0x71E0C, // 01 bg darkblue fg magenta
   0xB7E20, 0x7D909, 0x71D38, // 11 fg magenta fg magenta
   0x71D38, 0x445EE, 0xBCE0C, // 00 bg darkblue bg darkblue
   0x71D38, 0x445EE, 0xBCE0C, // 10 fg darkblue bg darkblue
   0x71D38, 0x445EE, 0xBCE0C, // 01 bg darkblue fg darkblue
   0x71D38, 0x445EE, 0xBCE0C, // 11 fg darkblue fg darkblue
   0x71D38, 0x445EE, 0xBCE0C, // 00 bg darkblue bg darkblue
   0xCE2E7, 0xFB9C1, 0xBCE00, // 10 fg purple bg darkblue
   0x39D38, 0xF05EE, 0xBFE0C, // 01 bg darkblue fg purple
   0x862E7, 0x4F9C1, 0xBFE00, // 11 fg purple fg purple
   0x71D38, 0x445EE, 0xBCE0C, // 00 bg darkblue bg darkblue
   0xCE1F7, 0xFBA86, 0xBCD3C, // 10 fg darkgreen bg darkblue
   0xFDD38, 0x219EE, 0x70E0C, // 01 bg darkblue fg darkgreen
   0x421F7, 0x9E686, 0x70D3C, // 11 fg darkgreen fg darkgreen
   0x71D38, 0x445EE, 0xBCE0C, // 00 bg darkblue bg darkblue
   0xCE17F, 0x4457F, 0x0317F, // 10 fg gray1 (dark gray) bg darkblue
   0xDFD38, 0x601EE, 0xDFE0C, // 01 bg darkblue fg gray1 (dark gray)
   0x6017F, 0x6017F, 0x6017F, // 11 fg gray1 (dark gray) fg gray1 (dark gray)
   0x71D38, 0x445EE, 0xBCE0C, // 00 bg darkblue bg darkblue
   0x71E47, 0x4458E, 0xBCE00, // 10 fg mediumblue bg darkblue
   0xAE138, 0x5C5EE, 0xBFE0C, // 01 bg darkblue fg mediumblue
   0xAE247, 0x5C58E, 0xBFE00, // 11 fg mediumblue fg mediumblue
   0x71D38, 0x445EE, 0xBCE0C, // 00 bg darkblue bg darkblue
   0xCE23D, 0x446DF, 0xBCE00, // 10 fg lightblue bg darkblue
   0x0F538, 0x881EE, 0xBFE0C, // 01 bg darkblue fg lightblue
   0xB0A3D, 0x882DF, 0xBFE00, // 11 fg lightblue fg lightblue
   0x71D38, 0x445EE, 0xBCE0C, // 00 bg darkblue bg darkblue
   0xCE1C7, 0xFB923, 0xBCD00, // 10 fg brown bg darkblue
   0xF1D38, 0xC8DEE, 0x7FE0C, // 01 bg darkblue fg brown
   0x4E1C7, 0x77123, 0x7FD00, // 11 fg brown fg brown
   0x71D38, 0x445EE, 0xBCE0C, // 00 bg darkblue bg darkblue
   0x71E1D, 0xFBA84, 0xBCD00, // 10 fg orange bg darkblue
   0xB8938, 0x211EE, 0x7FE0C, // 01 bg darkblue fg orange
   0xB8A1D, 0x9EE84, 0x7FD00, // 11 fg orange fg orange
   0x71D38, 0x445EE, 0xBCE0C, // 00 bg darkblue bg darkblue
   0xCE17F, 0x4457F, 0x0317F, // 10 fg gray2 (light gray) bg darkblue
   0xDFD38, 0x601EE, 0xDFE0C, // 01 bg darkblue fg gray2 (light gray)
   0x6017F, 0x6017F, 0x6017F, // 11 fg gray2 (light gray) fg gray2 (light gray)
   0x71D38, 0x445EE, 0xBCE0C, // 00 bg darkblue bg darkblue
   0xCE2FC, 0xFBAD0, 0xBCEC1, // 10 fg pink bg darkblue
   0x3F138, 0x341EE, 0x8FA0C, // 01 bg darkblue fg pink
   0x80EFC, 0x8BED0, 0x8FAC1, // 11 fg pink fg pink
   0x71D38, 0x445EE, 0xBCE0C, // 00 bg darkblue bg darkblue
   0x71E41, 0xFBA11, 0xBCD00, // 10 fg green bg darkblue
   0xAF938, 0x045EE, 0x7FE0C, // 01 bg darkblue fg green
   0xAFA41, 0xBBA11, 0x7FD00, // 11 fg green fg green
   0x71D38, 0x445EE, 0xBCE0C, // 00 bg darkblue bg darkblue
   0x71EC1, 0x446E7, 0xBCD00, // 10 fg yellow bg darkblue
   0x8F938, 0x861EE, 0x7FE0C, // 01 bg darkblue fg yellow
   0x8FAC1, 0x862E7, 0x7FD00, // 11 fg yellow fg yellow
   0x71D38, 0x445EE, 0xBCE0C, // 00 bg darkblue bg darkblue
   0x71E71, 0x446F7, 0x0323D, // 10 fg aqua bg darkblue
   0xA3938, 0x821EE, 0x0F60C, // 01 bg darkblue fg aqua
   0xA3A71, 0x822F7, 0xB0A3D, // 11 fg aqua fg aqua
   0x71D38, 0x445EE, 0xBCE0C, // 00 bg darkblue bg darkblue
   0xCE2FA, 0x446FA, 0x032FA, // 10 fg white bg darkblue
   0x3E938, 0x815EE, 0x3EA0C, // 01 bg darkblue fg white
   0x816FA, 0x816FA, 0x816FA, // 11 fg white fg white
   0x862E7, 0x4F9C1, 0xBFE00, // 00 bg purple bg purple
   0x39D00, 0x4F900, 0xBFD00, // 10 fg black bg purple
   0xC02E7, 0x7FDC1, 0x7FE00, // 01 bg purple fg black
   0x7FD00, 0x7FD00, 0x7FD00, // 11 fg black fg black
   0x862E7, 0x4F9C1, 0xBFE00, // 00 bg purple bg purple
   0x39E20, 0x4F909, 0xBFD38, // 10 fg magenta bg purple
   0x082E7, 0x7D9C1, 0x71E00, // 01 bg purple fg magenta
   0xB7E20, 0x7D909, 0x71D38, // 11 fg magenta fg magenta
   0x862E7, 0x4F9C1, 0xBFE00, // 00 bg purple bg purple
   0x39D38, 0xF05EE, 0xBFE0C, // 10 fg darkblue bg purple
   0xCE2E7, 0xFB9C1, 0xBCE00, // 01 bg purple fg darkblue
   0x71D38, 0x445EE, 0xBCE0C, // 11 fg darkblue fg darkblue
   0x862E7, 0x4F9C1, 0xBFE00, // 00 bg purple bg purple
   0x862E7, 0x4F9C1, 0xBFE00, // 10 fg purple bg purple
   0x862E7, 0x4F9C1, 0xBFE00, // 01 bg purple fg purple
   0x862E7, 0x4F9C1, 0xBFE00, // 11 fg purple fg purple
   0x862E7, 0x4F9C1, 0xBFE00, // 00 bg purple bg purple
   0x861F7, 0x4FA86, 0xBFD3C, // 10 fg darkgreen bg purple
   0x422E7, 0x9E5C1, 0x70E00, // 01 bg purple fg darkgreen
   0x421F7, 0x9E686, 0x70D3C, // 11 fg darkgreen fg darkgreen
   0x862E7, 0x4F9C1, 0xBFE00, // 00 bg purple bg purple
   0x8617F, 0xF057F, 0x0017F, // 10 fg gray1 (dark gray) bg purple
   0x602E7, 0xDFDC1, 0xDFE00, // 01 bg purple fg gray1 (dark gray)
   0x6017F, 0x6017F, 0x6017F, // 11 fg gray1 (dark gray) fg gray1 (dark gray)
   0x862E7, 0x4F9C1, 0xBFE00, // 00 bg purple bg purple
   0x86247, 0x4F98E, 0xBFE00, // 10 fg mediumblue bg purple
   0xAE2E7, 0x5C5C1, 0xBFE00, // 01 bg purple fg mediumblue
   0xAE247, 0x5C58E, 0xBFE00, // 11 fg mediumblue fg mediumblue
   0x862E7, 0x4F9C1, 0xBFE00, // 00 bg purple bg purple
   0x8623D, 0xF06DF, 0xBFE00, // 10 fg lightblue bg purple
   0xB0AE7, 0x37DC1, 0xBFE00, // 01 bg purple fg lightblue
   0xB0A3D, 0x882DF, 0xBFE00, // 11 fg lightblue fg lightblue
   0x862E7, 0x4F9C1, 0xBFE00, // 00 bg purple bg purple
   0x861C7, 0x4F923, 0xBFD00, // 10 fg brown bg purple
   0x4E2E7, 0x771C1, 0x7FE00, // 01 bg purple fg brown
   0x4E1C7, 0x77123, 0x7FD00, // 11 fg brown fg brown
   0x862E7, 0x4F9C1, 0xBFE00, // 00 bg purple bg purple
   0x8621D, 0x4FA84, 0xBFD00, // 10 fg orange bg purple
   0xB8AE7, 0x9EDC1, 0x7FE00, // 01 bg purple fg orange
   0xB8A1D, 0x9EE84, 0x7FD00, // 11 fg orange fg orange
   0x862E7, 0x4F9C1, 0xBFE00, // 00 bg purple bg purple
   0x8617F, 0xF057F, 0x0017F, // 10 fg gray2 (light gray) bg purple
   0x602E7, 0xDFDC1, 0xDFE00, // 01 bg purple fg gray2 (light gray)
   0x6017F, 0x6017F, 0x6017F, // 11 fg gray2 (light gray) fg gray2 (light gray)
   0x862E7, 0x4F9C1, 0xBFE00, // 00 bg purple bg purple
   0x862FC, 0x4FAD0, 0xBFEC1, // 10 fg pink bg purple
   0x80EE7, 0x8BDC1, 0x8FA00, // 01 bg purple fg pink
   0x80EFC, 0x8BED0, 0x8FAC1, // 11 fg pink fg pink
   0x862E7, 0x4F9C1, 0xBFE00, // 00 bg purple bg purple
   0x39E41, 0x4FA11, 0xBFD00, // 10 fg green bg purple
   0x106E7, 0xBB9C1, 0x7FE00, // 01 bg purple fg green
   0xAFA41, 0xBBA11, 0x7FD00, // 11 fg green fg green
   0x862E7, 0x4F9C1, 0xBFE00, // 00 bg purple bg purple
   0x39EC1, 0xF06E7, 0xBFD00, // 10 fg yellow bg purple
   0x306E7, 0x39DC1, 0x7FE00, // 01 bg purple fg yellow
   0x8FAC1, 0x862E7, 0x7FD00, // 11 fg yellow fg yellow
   0x862E7, 0x4F9C1, 0xBFE00, // 00 bg purple bg purple
   0x86271, 0xF06F7, 0x0023D, // 10 fg aqua bg purple
   0xA3AE7, 0x3DDC1, 0x0F600, // 01 bg purple fg aqua
   0xA3A71, 0x822F7, 0xB0A3D, // 11 fg aqua fg aqua
   0x862E7, 0x4F9C1, 0xBFE00, // 00 bg purple bg purple
   0x862FA, 0xF06FA, 0x002FA, // 10 fg white bg purple
   0x816E7, 0x3E9C1, 0x3EA00, // 01 bg purple fg white
   0x816FA, 0x816FA, 0x816FA, // 11 fg white fg white
   0x421F7, 0x9E686, 0x70D3C, // 00 bg darkgreen bg darkgreen
   0xFDD00, 0x9E500, 0x70D00, // 10 fg black bg darkgreen
   0xC01F7, 0x7FE86, 0x7FD3C, // 01 bg darkgreen fg black
   0x7FD00, 0x7FD00, 0x7FD00, // 11 fg black fg black
   0x421F7, 0x9E686, 0x70D3C, // 00 bg darkgreen bg darkgreen
   0xFDE20, 0x9E509, 0x70D38, // 10 fg magenta bg darkgreen
   0x081F7, 0x7DA86, 0x71D3C, // 01 bg darkgreen fg magenta
   0xB7E20, 0x7D909, 0x71D38, // 11 fg magenta fg magenta
   0x421F7, 0x9E686, 0x70D3C, // 00 bg darkgreen bg darkgreen
   0xFDD38, 0x219EE, 0x70E0C, // 10 fg darkblue bg darkgreen
   0xCE1F7, 0xFBA86, 0xBCD3C, // 01 bg darkgreen fg darkblue
   0x71D38, 0x445EE, 0xBCE0C, // 11 fg darkblue fg darkblue
   0x421F7, 0x9E686, 0x70D3C, // 00 bg darkgreen bg darkgreen
   0x422E7, 0x9E5C1, 0x70E00, // 10 fg purple bg darkgreen
   0x861F7, 0x4FA86, 0xBFD3C, // 01 bg darkgreen fg purple
   0x862E7, 0x4F9C1, 0xBFE00, // 11 fg purple fg purple
   0x421F7, 0x9E686, 0x70D3C, // 00 bg darkgreen bg darkgreen
   0x421F7, 0x9E686, 0x70D3C, // 10 fg darkgreen bg darkgreen
   0x421F7, 0x9E686, 0x70D3C, // 01 bg darkgreen fg darkgreen
   0x421F7, 0x9E686, 0x70D3C, // 11 fg darkgreen fg darkgreen
   0x421F7, 0x9E686, 0x70D3C, // 00 bg darkgreen bg darkgreen
   0x4217F, 0x2197F, 0x70D7F, // 10 fg gray1 (dark gray) bg darkgreen
   0x601F7, 0xDFE86, 0x6013C, // 01 bg darkgreen fg gray1 (dark gray)
   0x6017F, 0x6017F, 0x6017F, // 11 fg gray1 (dark gray) fg gray1 (dark gray)
   0x421F7, 0x9E686, 0x70D3C, // 00 bg darkgreen bg darkgreen
   0x42247, 0x9E58E, 0x70E00, // 10 fg mediumblue bg darkgreen
   0xAE1F7, 0x5C686, 0xBFD3C, // 01 bg darkgreen fg mediumblue
   0xAE247, 0x5C58E, 0xBFE00, // 11 fg mediumblue fg mediumblue
   0x421F7, 0x9E686, 0x70D3C, // 00 bg darkgreen bg darkgreen
   0x4223D, 0x21ADF, 0x70E00, // 10 fg lightblue bg darkgreen
   0xB09F7, 0x37E86, 0xBFD3C, // 01 bg darkgreen fg lightblue
   0xB0A3D, 0x882DF, 0xBFE00, // 11 fg lightblue fg lightblue
   0x421F7, 0x9E686, 0x70D3C, // 00 bg darkgreen bg darkgreen
   0x421C7, 0x9E523, 0x70D00, // 10 fg brown bg darkgreen
   0x4E1F7, 0x77286, 0x7FD3C, // 01 bg darkgreen fg brown
   0x4E1C7, 0x77123, 0x7FD00, // 11 fg brown fg brown
   0x421F7, 0x9E686, 0x70D3C, // 00 bg darkgreen bg darkgreen
   0x4221D, 0x9E684, 0x70D00, // 10 fg orange bg darkgreen
   0xB89F7, 0x9EE86, 0x7FD3C, // 01 bg darkgreen fg orange
   0xB8A1D, 0x9EE84, 0x7FD00, // 11 fg orange fg orange
   0x421F7, 0x9E686, 0x70D3C, // 00 bg darkgreen bg darkgreen
   0x4217F, 0x2197F, 0x70D7F, // 10 fg gray2 (light gray) bg darkgreen
   0x601F7, 0xDFE86, 0x6013C, // 01 bg darkgreen fg gray2 (light gray)
   0x6017F, 0x6017F, 0x6017F, // 11 fg gray2 (light gray) fg gray2 (light gray)
   0x421F7, 0x9E686, 0x70D3C, // 00 bg darkgreen bg darkgreen
   0x422FC, 0x9E6D0, 0x70EC1, // 10 fg pink bg darkgreen
   0x80DF7, 0x8BE86, 0x8F93C, // 01 bg darkgreen fg pink
   0x80EFC, 0x8BED0, 0x8FAC1, // 11 fg pink fg pink
   0x421F7, 0x9E686, 0x70D3C, // 00 bg darkgreen bg darkgreen
   0xFDE41, 0x9E611, 0x70D00, // 10 fg green bg darkgreen
   0x105F7, 0xBBA86, 0x7FD3C, // 01 bg darkgreen fg green
   0xAFA41, 0xBBA11, 0x7FD00, // 11 fg green fg green
   0x421F7, 0x9E686, 0x70D3C, // 00 bg darkgreen bg darkgreen
   0xFDEC1, 0x21AE7, 0x70D00, // 10 fg yellow bg darkgreen
   0x305F7, 0x39E86, 0x7FD3C, // 01 bg darkgreen fg yellow
   0x8FAC1, 0x862E7, 0x7FD00, // 11 fg yellow fg yellow
   0x421F7, 0x9E686, 0x70D3C, // 00 bg darkgreen bg darkgreen
   0x42271, 0x21AF7, 0x70E3D, // 10 fg aqua bg darkgreen
   0xA39F7, 0x3DE86, 0xB093C, // 01 bg darkgreen fg aqua
   0xA3A71, 0x822F7, 0xB0A3D, // 11 fg aqua fg aqua
   0x421F7, 0x9E686, 0x70D3C, // 00 bg darkgreen bg darkgreen
   0x422FA, 0x21AFA, 0x70EFA, // 10 fg white bg darkgreen
   0x815F7, 0x3EA86, 0x8153C, // 01 bg darkgreen fg white
   0x816FA, 0x816FA, 0x816FA, // 11 fg white fg white
   0x6017F, 0x6017F, 0x6017F, // 00 bg gray1 (dark gray) bg gray1 (dark gray)
   0xDFD00, 0xDFD00, 0xDFD00, // 10 fg black bg gray1 (dark gray)
   0xC017F, 0xC017F, 0xC017F, // 01 bg gray1 (dark gray) fg black
   0x7FD00, 0x7FD00, 0x7FD00, // 11 fg black fg black
   0x6017F, 0x6017F, 0x6017F, // 00 bg gray1 (dark gray) bg gray1 (dark gray)
   0xDFE20, 0xDFD09, 0xDFD38, // 10 fg magenta bg gray1 (dark gray)
   0x0817F, 0xC257F, 0xCE17F, // 01 bg gray1 (dark gray) fg magenta
   0xB7E20, 0x7D909, 0x71D38, // 11 fg magenta fg magenta
   0x6017F, 0x6017F, 0x6017F, // 00 bg gray1 (dark gray) bg gray1 (dark gray)
   0xDFD38, 0x601EE, 0xDFE0C, // 10 fg darkblue bg gray1 (dark gray)
   0xCE17F, 0x4457F, 0x0317F, // 01 bg gray1 (dark gray) fg darkblue
   0x71D38, 0x445EE, 0xBCE0C, // 11 fg darkblue fg darkblue
   0x6017F, 0x6017F, 0x6017F, // 00 bg gray1 (dark gray) bg gray1 (dark gray)
   0x602E7, 0xDFDC1, 0xDFE00, // 10 fg purple bg gray1 (dark gray)
   0x8617F, 0xF057F, 0x0017F, // 01 bg gray1 (dark gray) fg purple
   0x862E7, 0x4F9C1, 0xBFE00, // 11 fg purple fg purple
   0x6017F, 0x6017F, 0x6017F, // 00 bg gray1 (dark gray) bg gray1 (dark gray)
   0x601F7, 0xDFE86, 0x6013C, // 10 fg darkgreen bg gray1 (dark gray)
   0x4217F, 0x2197F, 0x70D7F, // 01 bg gray1 (dark gray) fg darkgreen
   0x421F7, 0x9E686, 0x70D3C, // 11 fg darkgreen fg darkgreen
   0x6017F, 0x6017F, 0x6017F, // 00 bg gray1 (dark gray) bg gray1 (dark gray)
   0x6017F, 0x6017F, 0x6017F, // 10 fg gray1 (dark gray) bg gray1 (dark gray)
   0x6017F, 0x6017F, 0x6017F, // 01 bg gray1 (dark gray) fg gray1 (dark gray)
   0x6017F, 0x6017F, 0x6017F, // 11 fg gray1 (dark gray) fg gray1 (dark gray)
   0x6017F, 0x6017F, 0x6017F, // 00 bg gray1 (dark gray) bg gray1 (dark gray)
   0x60247, 0x6018E, 0xDFE00, // 10 fg mediumblue bg gray1 (dark gray)
   0xAE17F, 0x5C57F, 0x0017F, // 01 bg gray1 (dark gray) fg mediumblue
   0xAE247, 0x5C58E, 0xBFE00, // 11 fg mediumblue fg mediumblue
   0x6017F, 0x6017F, 0x6017F, // 00 bg gray1 (dark gray) bg gray1 (dark gray)
   0x6023D, 0x602DF, 0xDFE00, // 10 fg lightblue bg gray1 (dark gray)
   0xB097F, 0x8817F, 0x0017F, // 01 bg gray1 (dark gray) fg lightblue
   0xB0A3D, 0x882DF, 0xBFE00, // 11 fg lightblue fg lightblue
   0x6017F, 0x6017F, 0x6017F, // 00 bg gray1 (dark gray) bg gray1 (dark gray)
   0x601C7, 0xDFD23, 0xDFD00, // 10 fg brown bg gray1 (dark gray)
   0x4E17F, 0xC8D7F, 0xC017F, // 01 bg gray1 (dark gray) fg brown
   0x4E1C7, 0x77123, 0x7FD00, // 11 fg brown fg brown
   0x6017F, 0x6017F, 0x6017F, // 00 bg gray1 (dark gray) bg gray1 (dark gray)
   0x6021D, 0xDFE84, 0xDFD00, // 10 fg orange bg gray1 (dark gray)
   0xB897F, 0x2117F, 0xC017F, // 01 bg gray1 (dark gray) fg orange
   0xB8A1D, 0x9EE84, 0x7FD00, // 11 fg orange fg orange
   0x6017F, 0x6017F, 0x6017F, // 00 bg gray1 (dark gray) bg gray1 (dark gray)
   0x6017F, 0x6017F, 0x6017F, // 10 fg gray2 (light gray) bg gray1 (dark gray)
   0x6017F, 0x6017F, 0x6017F, // 01 bg gray1 (dark gray) fg gray2 (light gray)
   0x6017F, 0x6017F, 0x6017F, // 11 fg gray2 (light gray) fg gray2 (light gray)
   0x6017F, 0x6017F, 0x6017F, // 00 bg gray1 (dark gray) bg gray1 (dark gray)
   0x602FC, 0xDFED0, 0xDFEC1, // 10 fg pink bg gray1 (dark gray)
   0x80D7F, 0x3417F, 0x3057F, // 01 bg gray1 (dark gray) fg pink
   0x80EFC, 0x8BED0, 0x8FAC1, // 11 fg pink fg pink
   0x6017F, 0x6017F, 0x6017F, // 00 bg gray1 (dark gray) bg gray1 (dark gray)
   0xDFE41, 0xDFE11, 0xDFD00, // 10 fg green bg gray1 (dark gray)
   0x1057F, 0x0457F, 0xC017F, // 01 bg gray1 (dark gray) fg green
   0xAFA41, 0xBBA11, 0x7FD00, // 11 fg green fg green
   0x6017F, 0x6017F, 0x6017F, // 00 bg gray1 (dark gray) bg gray1 (dark gray)
   0xDFEC1, 0x602E7, 0xDFD00, // 10 fg yellow bg gray1 (dark gray)
   0x3057F, 0x8617F, 0xC017F, // 01 bg gray1 (dark gray) fg yellow
   0x8FAC1, 0x862E7, 0x7FD00, // 11 fg yellow fg yellow
   0x6017F, 0x6017F, 0x6017F, // 00 bg gray1 (dark gray) bg gray1 (dark gray)
   0x60271, 0x602F7, 0x6023D, // 10 fg aqua bg gray1 (dark gray)
   0xA397F, 0x8217F, 0xB097F, // 01 bg gray1 (dark gray) fg aqua
   0xA3A71, 0x822F7, 0xB0A3D, // 11 fg aqua fg aqua
   0x6017F, 0x6017F, 0x6017F, // 00 bg gray1 (dark gray) bg gray1 (dark gray)
   0x602FA, 0x602FA, 0x602FA, // 10 fg white bg gray1 (dark gray)
   0x8157F, 0x8157F, 0x8157F, // 01 bg gray1 (dark gray) fg white
   0x816FA, 0x816FA, 0x816FA, // 11 fg white fg white
   0xAE247, 0x5C58E, 0xBFE00, // 00 bg mediumblue bg mediumblue
   0xAE100, 0x5C500, 0xBFD00, // 10 fg black bg mediumblue
   0x7FE47, 0x7FD8E, 0x7FE00, // 01 bg mediumblue fg black
   0x7FD00, 0x7FD00, 0x7FD00, // 11 fg black fg black
   0xAE247, 0x5C58E, 0xBFE00, // 00 bg mediumblue bg mediumblue
   0xAE220, 0x5C509, 0xBFD38, // 10 fg magenta bg mediumblue
   0xB7E47, 0x7D98E, 0x71E00, // 01 bg mediumblue fg magenta
   0xB7E20, 0x7D909, 0x71D38, // 11 fg magenta fg magenta
   0xAE247, 0x5C58E, 0xBFE00, // 00 bg mediumblue bg mediumblue
   0xAE138, 0x5C5EE, 0xBFE0C, // 10 fg darkblue bg mediumblue
   0x71E47, 0x4458E, 0xBCE00, // 01 bg mediumblue fg darkblue
   0x71D38, 0x445EE, 0xBCE0C, // 11 fg darkblue fg darkblue
   0xAE247, 0x5C58E, 0xBFE00, // 00 bg mediumblue bg mediumblue
   0xAE2E7, 0x5C5C1, 0xBFE00, // 10 fg purple bg mediumblue
   0x86247, 0x4F98E, 0xBFE00, // 01 bg mediumblue fg purple
   0x862E7, 0x4F9C1, 0xBFE00, // 11 fg purple fg purple
   0xAE247, 0x5C58E, 0xBFE00, // 00 bg mediumblue bg mediumblue
   0xAE1F7, 0x5C686, 0xBFD3C, // 10 fg darkgreen bg mediumblue
   0x42247, 0x9E58E, 0x70E00, // 01 bg mediumblue fg darkgreen
   0x421F7, 0x9E686, 0x70D3C, // 11 fg darkgreen fg darkgreen
   0xAE247, 0x5C58E, 0xBFE00, // 00 bg mediumblue bg mediumblue
   0xAE17F, 0x5C57F, 0x0017F, // 10 fg gray1 (dark gray) bg mediumblue
   0x60247, 0x6018E, 0xDFE00, // 01 bg mediumblue fg gray1 (dark gray)
   0x6017F, 0x6017F, 0x6017F, // 11 fg gray1 (dark gray) fg gray1 (dark gray)
   0xAE247, 0x5C58E, 0xBFE00, // 00 bg mediumblue bg mediumblue
   0xAE247, 0x5C58E, 0xBFE00, // 10 fg mediumblue bg mediumblue
   0xAE247, 0x5C58E, 0xBFE00, // 01 bg mediumblue fg mediumblue
   0xAE247, 0x5C58E, 0xBFE00, // 11 fg mediumblue fg mediumblue
   0xAE247, 0x5C58E, 0xBFE00, // 00 bg mediumblue bg mediumblue
   0xAE23D, 0x5C6DF, 0xBFE00, // 10 fg lightblue bg mediumblue
   0xB0A47, 0x8818E, 0xBFE00, // 01 bg mediumblue fg lightblue
   0xB0A3D, 0x882DF, 0xBFE00, // 11 fg lightblue fg lightblue
   0xAE247, 0x5C58E, 0xBFE00, // 00 bg mediumblue bg mediumblue
   0xAE1C7, 0x5C523, 0xBFD00, // 10 fg brown bg mediumblue
   0x4E247, 0x7718E, 0x7FE00, // 01 bg mediumblue fg brown
   0x4E1C7, 0x77123, 0x7FD00, // 11 fg brown fg brown
   0xAE247, 0x5C58E, 0xBFE00, // 00 bg mediumblue bg mediumblue
   0xAE21D, 0x5C684, 0xBFD00, // 10 fg orange bg mediumblue
   0xB8A47, 0x9ED8E, 0x7FE00, // 01 bg mediumblue fg orange
   0xB8A1D, 0x9EE84, 0x7FD00, // 11 fg orange fg orange
   0xAE247, 0x5C58E, 0xBFE00, // 00 bg mediumblue bg mediumblue
   0xAE17F, 0x5C57F, 0x0017F, // 10 fg gray2 (light gray) bg mediumblue
   0x60247, 0x6018E, 0xDFE00, // 01 bg mediumblue fg gray2 (light gray)
   0x6017F, 0x6017F, 0x6017F, // 11 fg gray2 (light gray) fg gray2 (light gray)
   0xAE247, 0x5C58E, 0xBFE00, // 00 bg mediumblue bg mediumblue
   0xAE2FC, 0x5C6D0, 0xBFEC1, // 10 fg pink bg mediumblue
   0x80E47, 0x8BD8E, 0x8FA00, // 01 bg mediumblue fg pink
   0x80EFC, 0x8BED0, 0x8FAC1, // 11 fg pink fg pink
   0xAE247, 0x5C58E, 0xBFE00, // 00 bg mediumblue bg mediumblue
   0xAE241, 0x5C611, 0xBFD00, // 10 fg green bg mediumblue
   0xAFA47, 0xBB98E, 0x7FE00, // 01 bg mediumblue fg green
   0xAFA41, 0xBBA11, 0x7FD00, // 11 fg green fg green
   0xAE247, 0x5C58E, 0xBFE00, // 00 bg mediumblue bg mediumblue
   0xAE2C1, 0x5C6E7, 0xBFD00, // 10 fg yellow bg mediumblue
   0x8FA47, 0x8618E, 0x7FE00, // 01 bg mediumblue fg yellow
   0x8FAC1, 0x862E7, 0x7FD00, // 11 fg yellow fg yellow
   0xAE247, 0x5C58E, 0xBFE00, // 00 bg mediumblue bg mediumblue
   0xAE271, 0x5C6F7, 0x0023D, // 10 fg aqua bg mediumblue
   0xA3A47, 0x8218E, 0x0F600, // 01 bg mediumblue fg aqua
   0xA3A71, 0x822F7, 0xB0A3D, // 11 fg aqua fg aqua
   0xAE247, 0x5C58E, 0xBFE00, // 00 bg mediumblue bg mediumblue
   0xAE2FA, 0x5C6FA, 0x002FA, // 10 fg white bg mediumblue
   0x81647, 0x8158E, 0x3EA00, // 01 bg mediumblue fg white
   0x816FA, 0x816FA, 0x816FA, // 11 fg white fg white
   0xB0A3D, 0x882DF, 0xBFE00, // 00 bg lightblue bg lightblue
   0x0F500, 0x37D00, 0xBFD00, // 10 fg black bg lightblue
   0xC023D, 0xC02DF, 0x7FE00, // 01 bg lightblue fg black
   0x7FD00, 0x7FD00, 0x7FD00, // 11 fg black fg black
   0xB0A3D, 0x882DF, 0xBFE00, // 00 bg lightblue bg lightblue
   0x0F620, 0x37D09, 0xBFD38, // 10 fg magenta bg lightblue
   0x0823D, 0xC26DF, 0x71E00, // 01 bg lightblue fg magenta
   0xB7E20, 0x7D909, 0x71D38, // 11 fg magenta fg magenta
   0xB0A3D, 0x882DF, 0xBFE00, // 00 bg lightblue bg lightblue
   0x0F538, 0x881EE, 0xBFE0C, // 10 fg darkblue bg lightblue
   0xCE23D, 0x446DF, 0xBCE00, // 01 bg lightblue fg darkblue
   0x71D38, 0x445EE, 0xBCE0C, // 11 fg darkblue fg darkblue
   0xB0A3D, 0x882DF, 0xBFE00, // 00 bg lightblue bg lightblue
   0xB0AE7, 0x37DC1, 0xBFE00, // 10 fg purple bg lightblue
   0x8623D, 0xF06DF, 0xBFE00, // 01 bg lightblue fg purple
   0x862E7, 0x4F9C1, 0xBFE00, // 11 fg purple fg purple
   0xB0A3D, 0x882DF, 0xBFE00, // 00 bg lightblue bg lightblue
   0xB09F7, 0x37E86, 0xBFD3C, // 10 fg darkgreen bg lightblue
   0x4223D, 0x21ADF, 0x70E00, // 01 bg lightblue fg darkgreen
   0x421F7, 0x9E686, 0x70D3C, // 11 fg darkgreen fg darkgreen
   0xB0A3D, 0x882DF, 0xBFE00, // 00 bg lightblue bg lightblue
   0xB097F, 0x8817F, 0x0017F, // 10 fg gray1 (dark gray) bg lightblue
   0x6023D, 0x602DF, 0xDFE00, // 01 bg lightblue fg gray1 (dark gray)
   0x6017F, 0x6017F, 0x6017F, // 11 fg gray1 (dark gray) fg gray1 (dark gray)
   0xB0A3D, 0x882DF, 0xBFE00, // 00 bg lightblue bg lightblue
   0xB0A47, 0x8818E, 0xBFE00, // 10 fg mediumblue bg lightblue
   0xAE23D, 0x5C6DF, 0xBFE00, // 01 bg lightblue fg mediumblue
   0xAE247, 0x5C58E, 0xBFE00, // 11 fg mediumblue fg mediumblue
   0xB0A3D, 0x882DF, 0xBFE00, // 00 bg lightblue bg lightblue
   0xB0A3D, 0x882DF, 0xBFE00, // 10 fg lightblue bg lightblue
   0xB0A3D, 0x882DF, 0xBFE00, // 01 bg lightblue fg lightblue
   0xB0A3D, 0x882DF, 0xBFE00, // 11 fg lightblue fg lightblue
   0xB0A3D, 0x882DF, 0xBFE00, // 00 bg lightblue bg lightblue
   0xB09C7, 0x37D23, 0xBFD00, // 10 fg brown bg lightblue
   0x4E23D, 0xC8EDF, 0x7FE00, // 01 bg lightblue fg brown
   0x4E1C7, 0x77123, 0x7FD00, // 11 fg brown fg brown
   0xB0A3D, 0x882DF, 0xBFE00, // 00 bg lightblue bg lightblue
   0xB0A1D, 0x37E84, 0xBFD00, // 10 fg orange bg lightblue
   0xB8A3D, 0x212DF, 0x7FE00, // 01 bg lightblue fg orange
   0xB8A1D, 0x9EE84, 0x7FD00, // 11 fg orange fg orange
   0xB0A3D, 0x882DF, 0xBFE00, // 00 bg lightblue bg lightblue
   0xB097F, 0x8817F, 0x0017F, // 10 fg gray2 (light gray) bg lightblue
   0x6023D, 0x602DF, 0xDFE00, // 01 bg lightblue fg gray2 (light gray)
   0x6017F, 0x6017F, 0x6017F, // 11 fg gray2 (light gray) fg gray2 (light gray)
   0xB0A3D, 0x882DF, 0xBFE00, // 00 bg lightblue bg lightblue
   0xB0AFC, 0x37ED0, 0xBFEC1, // 10 fg pink bg lightblue
   0x80E3D, 0x342DF, 0x8FA00, // 01 bg lightblue fg pink
   0x80EFC, 0x8BED0, 0x8FAC1, // 11 fg pink fg pink
   0xB0A3D, 0x882DF, 0xBFE00, // 00 bg lightblue bg lightblue
   0x0F641, 0x37E11, 0xBFD00, // 10 fg green bg lightblue
   0x1063D, 0x046DF, 0x7FE00, // 01 bg lightblue fg green
   0xAFA41, 0xBBA11, 0x7FD00, // 11 fg green fg green
   0xB0A3D, 0x882DF, 0xBFE00, // 00 bg lightblue bg lightblue
   0x0F6C1, 0x882E7, 0xBFD00, // 10 fg yellow bg lightblue
   0x3063D, 0x862DF, 0x7FE00, // 01 bg lightblue fg yellow
   0x8FAC1, 0x862E7, 0x7FD00, // 11 fg yellow fg yellow
   0xB0A3D, 0x882DF, 0xBFE00, // 00 bg lightblue bg lightblue
   0xB0A71, 0x882F7, 0x0023D, // 10 fg aqua bg lightblue
   0xA3A3D, 0x822DF, 0x0F600, // 01 bg lightblue fg aqua
   0xA3A71, 0x822F7, 0xB0A3D, // 11 fg aqua fg aqua
   0xB0A3D, 0x882DF, 0xBFE00, // 00 bg lightblue bg lightblue
   0xB0AFA, 0x882FA, 0x002FA, // 10 fg white bg lightblue
   0x8163D, 0x816DF, 0x3EA00, // 01 bg lightblue fg white
   0x816FA, 0x816FA, 0x816FA, // 11 fg white fg white
   0x4E1C7, 0x77123, 0x7FD00, // 00 bg brown bg brown
   0xF1D00, 0x77100, 0x7FD00, // 10 fg black bg brown
   0xC01C7, 0x7FD23, 0x7FD00, // 01 bg brown fg black
   0x7FD00, 0x7FD00, 0x7FD00, // 11 fg black fg black
   0x4E1C7, 0x77123, 0x7FD00, // 00 bg brown bg brown
   0xF1E20, 0x77109, 0x7FD38, // 10 fg magenta bg brown
   0x081C7, 0x7D923, 0x71D00, // 01 bg brown fg magenta
   0xB7E20, 0x7D909, 0x71D38, // 11 fg magenta fg magenta
   0x4E1C7, 0x77123, 0x7FD00, // 00 bg brown bg brown
   0xF1D38, 0xC8DEE, 0x7FE0C, // 10 fg darkblue bg brown
   0xCE1C7, 0xFB923, 0xBCD00, // 01 bg brown fg darkblue
   0x71D38, 0x445EE, 0xBCE0C, // 11 fg darkblue fg darkblue
   0x4E1C7, 0x77123, 0x7FD00, // 00 bg brown bg brown
   0x4E2E7, 0x771C1, 0x7FE00, // 10 fg purple bg brown
   0x861C7, 0x4F923, 0xBFD00, // 01 bg brown fg purple
   0x862E7, 0x4F9C1, 0xBFE00, // 11 fg purple fg purple
   0x4E1C7, 0x77123, 0x7FD00, // 00 bg brown bg brown
   0x4E1F7, 0x77286, 0x7FD3C, // 10 fg darkgreen bg brown
   0x421C7, 0x9E523, 0x70D00, // 01 bg brown fg darkgreen
   0x421F7, 0x9E686, 0x70D3C, // 11 fg darkgreen fg darkgreen
   0x4E1C7, 0x77123, 0x7FD00, // 00 bg brown bg brown
   0x4E17F, 0xC8D7F, 0xC017F, // 10 fg gray1 (dark gray) bg brown
   0x601C7, 0xDFD23, 0xDFD00, // 01 bg brown fg gray1 (dark gray)
   0x6017F, 0x6017F, 0x6017F, // 11 fg gray1 (dark gray) fg gray1 (dark gray)
   0x4E1C7, 0x77123, 0x7FD00, // 00 bg brown bg brown
   0x4E247, 0x7718E, 0x7FE00, // 10 fg mediumblue bg brown
   0xAE1C7, 0x5C523, 0xBFD00, // 01 bg brown fg mediumblue
   0xAE247, 0x5C58E, 0xBFE00, // 11 fg mediumblue fg mediumblue
   0x4E1C7, 0x77123, 0x7FD00, // 00 bg brown bg brown
   0x4E23D, 0xC8EDF, 0x7FE00, // 10 fg lightblue bg brown
   0xB09C7, 0x37D23, 0xBFD00, // 01 bg brown fg lightblue
   0xB0A3D, 0x882DF, 0xBFE00, // 11 fg lightblue fg lightblue
   0x4E1C7, 0x77123, 0x7FD00, // 00 bg brown bg brown
   0x4E1C7, 0x77123, 0x7FD00, // 10 fg brown bg brown
   0x4E1C7, 0x77123, 0x7FD00, // 01 bg brown fg brown
   0x4E1C7, 0x77123, 0x7FD00, // 11 fg brown fg brown
   0x4E1C7, 0x77123, 0x7FD00, // 00 bg brown bg brown
   0x4E21D, 0x77284, 0x7FD00, // 10 fg orange bg brown
   0xB89C7, 0x9ED23, 0x7FD00, // 01 bg brown fg orange
   0xB8A1D, 0x9EE84, 0x7FD00, // 11 fg orange fg orange
   0x4E1C7, 0x77123, 0x7FD00, // 00 bg brown bg brown
   0x4E17F, 0xC8D7F, 0xC017F, // 10 fg gray2 (light gray) bg brown
   0x601C7, 0xDFD23, 0xDFD00, // 01 bg brown fg gray2 (light gray)
   0x6017F, 0x6017F, 0x6017F, // 11 fg gray2 (light gray) fg gray2 (light gray)
   0x4E1C7, 0x77123, 0x7FD00, // 00 bg brown bg brown
   0x4E2FC, 0x772D0, 0x7FEC1, // 10 fg pink bg brown
   0x80DC7, 0x8BD23, 0x8F900, // 01 bg brown fg pink
   0x80EFC, 0x8BED0, 0x8FAC1, // 11 fg pink fg pink
   0x4E1C7, 0x77123, 0x7FD00, // 00 bg brown bg brown
   0xF1E41, 0x77211, 0x7FD00, // 10 fg green bg brown
   0x105C7, 0xBB923, 0x7FD00, // 01 bg brown fg green
   0xAFA41, 0xBBA11, 0x7FD00, // 11 fg green fg green
   0x4E1C7, 0x77123, 0x7FD00, // 00 bg brown bg brown
   0xF1EC1, 0xC8EE7, 0x7FD00, // 10 fg yellow bg brown
   0x305C7, 0x39D23, 0x7FD00, // 01 bg brown fg yellow
   0x8FAC1, 0x862E7, 0x7FD00, // 11 fg yellow fg yellow
   0x4E1C7, 0x77123, 0x7FD00, // 00 bg brown bg brown
   0x4E271, 0xC8EF7, 0xC023D, // 10 fg aqua bg brown
   0xA39C7, 0x3DD23, 0x0F500, // 01 bg brown fg aqua
   0xA3A71, 0x822F7, 0xB0A3D, // 11 fg aqua fg aqua
   0x4E1C7, 0x77123, 0x7FD00, // 00 bg brown bg brown
   0x4E2FA, 0xC8EFA, 0xC02FA, // 10 fg white bg brown
   0x815C7, 0x3E923, 0x3E900, // 01 bg brown fg white
   0x816FA, 0x816FA, 0x816FA, // 11 fg white fg white
   0xB8A1D, 0x9EE84, 0x7FD00, // 00 bg orange bg orange
   0xB8900, 0x9ED00, 0x7FD00, // 10 fg black bg orange
   0x7FE1D, 0x7FE84, 0x7FD00, // 01 bg orange fg black
   0x7FD00, 0x7FD00, 0x7FD00, // 11 fg black fg black
   0xB8A1D, 0x9EE84, 0x7FD00, // 00 bg orange bg orange
   0xB8A20, 0x9ED09, 0x7FD38, // 10 fg magenta bg orange
   0xB7E1D, 0x7DA84, 0x71D00, // 01 bg orange fg magenta
   0xB7E20, 0x7D909, 0x71D38, // 11 fg magenta fg magenta
   0xB8A1D, 0x9EE84, 0x7FD00, // 00 bg orange bg orange
   0xB8938, 0x211EE, 0x7FE0C, // 10 fg darkblue bg orange
   0x71E1D, 0xFBA84, 0xBCD00, // 01 bg orange fg darkblue
   0x71D38, 0x445EE, 0xBCE0C, // 11 fg darkblue fg darkblue
   0xB8A1D, 0x9EE84, 0x7FD00, // 00 bg orange bg orange
   0xB8AE7, 0x9EDC1, 0x7FE00, // 10 fg purple bg orange
   0x8621D, 0x4FA84, 0xBFD00, // 01 bg orange fg purple
   0x862E7, 0x4F9C1, 0xBFE00, // 11 fg purple fg purple
   0xB8A1D, 0x9EE84, 0x7FD00, // 00 bg orange bg orange
   0xB89F7, 0x9EE86, 0x7FD3C, // 10 fg darkgreen bg orange
   0x4221D, 0x9E684, 0x70D00, // 01 bg orange fg darkgreen
   0x421F7, 0x9E686, 0x70D3C, // 11 fg darkgreen fg darkgreen
   0xB8A1D, 0x9EE84, 0x7FD00, // 00 bg orange bg orange
   0xB897F, 0x2117F, 0xC017F, // 10 fg gray1 (dark gray) bg orange
   0x6021D, 0xDFE84, 0xDFD00, // 01 bg orange fg gray1 (dark gray)
   0x6017F, 0x6017F, 0x6017F, // 11 fg gray1 (dark gray) fg gray1 (dark gray)
   0xB8A1D, 0x9EE84, 0x7FD00, // 00 bg orange bg orange
   0xB8A47, 0x9ED8E, 0x7FE00, // 10 fg mediumblue bg orange
   0xAE21D, 0x5C684, 0xBFD00, // 01 bg orange fg mediumblue
   0xAE247, 0x5C58E, 0xBFE00, // 11 fg mediumblue fg mediumblue
   0xB8A1D, 0x9EE84, 0x7FD00, // 00 bg orange bg orange
   0xB8A3D, 0x212DF, 0x7FE00, // 10 fg lightblue bg orange
   0xB0A1D, 0x37E84, 0xBFD00, // 01 bg orange fg lightblue
   0xB0A3D, 0x882DF, 0xBFE00, // 11 fg lightblue fg lightblue
   0xB8A1D, 0x9EE84, 0x7FD00, // 00 bg orange bg orange
   0xB89C7, 0x9ED23, 0x7FD00, // 10 fg brown bg orange
   0x4E21D, 0x77284, 0x7FD00, // 01 bg orange fg brown
   0x4E1C7, 0x77123, 0x7FD00, // 11 fg brown fg brown
   0xB8A1D, 0x9EE84, 0x7FD00, // 00 bg orange bg orange
   0xB8A1D, 0x9EE84, 0x7FD00, // 10 fg orange bg orange
   0xB8A1D, 0x9EE84, 0x7FD00, // 01 bg orange fg orange
   0xB8A1D, 0x9EE84, 0x7FD00, // 11 fg orange fg orange
   0xB8A1D, 0x9EE84, 0x7FD00, // 00 bg orange bg orange
   0xB897F, 0x2117F, 0xC017F, // 10 fg gray2 (light gray) bg orange
   0x6021D, 0xDFE84, 0xDFD00, // 01 bg orange fg gray2 (light gray)
   0x6017F, 0x6017F, 0x6017F, // 11 fg gray2 (light gray) fg gray2 (light gray)
   0xB8A1D, 0x9EE84, 0x7FD00, // 00 bg orange bg orange
   0xB8AFC, 0x9EED0, 0x7FEC1, // 10 fg pink bg orange
   0x80E1D, 0x8BE84, 0x8F900, // 01 bg orange fg pink
   0x80EFC, 0x8BED0, 0x8FAC1, // 11 fg pink fg pink
   0xB8A1D, 0x9EE84, 0x7FD00, // 00 bg orange bg orange
   0xB8A41, 0x9EE11, 0x7FD00, // 10 fg green bg orange
   0xAFA1D, 0xBBA84, 0x7FD00, // 01 bg orange fg green
   0xAFA41, 0xBBA11, 0x7FD00, // 11 fg green fg green
   0xB8A1D, 0x9EE84, 0x7FD00, // 00 bg orange bg orange
   0xB8AC1, 0x212E7, 0x7FD00, // 10 fg yellow bg orange
   0x8FA1D, 0x39E84, 0x7FD00, // 01 bg orange fg yellow
   0x8FAC1, 0x862E7, 0x7FD00, // 11 fg yellow fg yellow
   0xB8A1D, 0x9EE84, 0x7FD00, // 00 bg orange bg orange
   0xB8A71, 0x212F7, 0xC023D, // 10 fg aqua bg orange
   0xA3A1D, 0x3DE84, 0x0F500, // 01 bg orange fg aqua
   0xA3A71, 0x822F7, 0xB0A3D, // 11 fg aqua fg aqua
   0xB8A1D, 0x9EE84, 0x7FD00, // 00 bg orange bg orange
   0xB8AFA, 0x212FA, 0xC02FA, // 10 fg white bg orange
   0x8161D, 0x3EA84, 0x3E900, // 01 bg orange fg white
   0x816FA, 0x816FA, 0x816FA, // 11 fg white fg white
   0x6017F, 0x6017F, 0x6017F, // 00 bg gray2 (light gray) bg gray2 (light gray)
   0xDFD00, 0xDFD00, 0xDFD00, // 10 fg black bg gray2 (light gray)
   0xC017F, 0xC017F, 0xC017F, // 01 bg gray2 (light gray) fg black
   0x7FD00, 0x7FD00, 0x7FD00, // 11 fg black fg black
   0x6017F, 0x6017F, 0x6017F, // 00 bg gray2 (light gray) bg gray2 (light gray)
   0xDFE20, 0xDFD09, 0xDFD38, // 10 fg magenta bg gray2 (light gray)
   0x0817F, 0xC257F, 0xCE17F, // 01 bg gray2 (light gray) fg magenta
   0xB7E20, 0x7D909, 0x71D38, // 11 fg magenta fg magenta
   0x6017F, 0x6017F, 0x6017F, // 00 bg gray2 (light gray) bg gray2 (light gray)
   0xDFD38, 0x601EE, 0xDFE0C, // 10 fg darkblue bg gray2 (light gray)
   0xCE17F, 0x4457F, 0x0317F, // 01 bg gray2 (light gray) fg darkblue
   0x71D38, 0x445EE, 0xBCE0C, // 11 fg darkblue fg darkblue
   0x6017F, 0x6017F, 0x6017F, // 00 bg gray2 (light gray) bg gray2 (light gray)
   0x602E7, 0xDFDC1, 0xDFE00, // 10 fg purple bg gray2 (light gray)
   0x8617F, 0xF057F, 0x0017F, // 01 bg gray2 (light gray) fg purple
   0x862E7, 0x4F9C1, 0xBFE00, // 11 fg purple fg purple
   0x6017F, 0x6017F, 0x6017F, // 00 bg gray2 (light gray) bg gray2 (light gray)
   0x601F7, 0xDFE86, 0x6013C, // 10 fg darkgreen bg gray2 (light gray)
   0x4217F, 0x2197F, 0x70D7F, // 01 bg gray2 (light gray) fg darkgreen
   0x421F7, 0x9E686, 0x70D3C, // 11 fg darkgreen fg darkgreen
   0x6017F, 0x6017F, 0x6017F, // 00 bg gray2 (light gray) bg gray2 (light gray)
   0x6017F, 0x6017F, 0x6017F, // 10 fg gray1 (dark gray) bg gray2 (light gray)
   0x6017F, 0x6017F, 0x6017F, // 01 bg gray2 (light gray) fg gray1 (dark gray)
   0x6017F, 0x6017F, 0x6017F, // 11 fg gray1 (dark gray) fg gray1 (dark gray)
   0x6017F, 0x6017F, 0x6017F, // 00 bg gray2 (light gray) bg gray2 (light gray)
   0x60247, 0x6018E, 0xDFE00, // 10 fg mediumblue bg gray2 (light gray)
   0xAE17F, 0x5C57F, 0x0017F, // 01 bg gray2 (light gray) fg mediumblue
   0xAE247, 0x5C58E, 0xBFE00, // 11 fg mediumblue fg mediumblue
   0x6017F, 0x6017F, 0x6017F, // 00 bg gray2 (light gray) bg gray2 (light gray)
   0x6023D, 0x602DF, 0xDFE00, // 10 fg lightblue bg gray2 (light gray)
   0xB097F, 0x8817F, 0x0017F, // 01 bg gray2 (light gray) fg lightblue
   0xB0A3D, 0x882DF, 0xBFE00, // 11 fg lightblue fg lightblue
   0x6017F, 0x6017F, 0x6017F, // 00 bg gray2 (light gray) bg gray2 (light gray)
   0x601C7, 0xDFD23, 0xDFD00, // 10 fg brown bg gray2 (light gray)
   0x4E17F, 0xC8D7F, 0xC017F, // 01 bg gray2 (light gray) fg brown
   0x4E1C7, 0x77123, 0x7FD00, // 11 fg brown fg brown
   0x6017F, 0x6017F, 0x6017F, // 00 bg gray2 (light gray) bg gray2 (light gray)
   0x6021D, 0xDFE84, 0xDFD00, // 10 fg orange bg gray2 (light gray)
   0xB897F, 0x2117F, 0xC017F, // 01 bg gray2 (light gray) fg orange
   0xB8A1D, 0x9EE84, 0x7FD00, // 11 fg orange fg orange
   0x6017F, 0x6017F, 0x6017F, // 00 bg gray2 (light gray) bg gray2 (light gray)
   0x6017F, 0x6017F, 0x6017F, // 10 fg gray2 (light gray) bg gray2 (light gray)
   0x6017F, 0x6017F, 0x6017F, // 01 bg gray2 (light gray) fg gray2 (light gray)
   0x6017F, 0x6017F, 0x6017F, // 11 fg gray2 (light gray) fg gray2 (light gray)
   0x6017F, 0x6017F, 0x6017F, // 00 bg gray2 (light gray) bg gray2 (light gray)
   0x602FC, 0xDFED0, 0xDFEC1, // 10 fg pink bg gray2 (light gray)
   0x80D7F, 0x3417F, 0x3057F, // 01 bg gray2 (light gray) fg pink
   0x80EFC, 0x8BED0, 0x8FAC1, // 11 fg pink fg pink
   0x6017F, 0x6017F, 0x6017F, // 00 bg gray2 (light gray) bg gray2 (light gray)
   0xDFE41, 0xDFE11, 0xDFD00, // 10 fg green bg gray2 (light gray)
   0x1057F, 0x0457F, 0xC017F, // 01 bg gray2 (light gray) fg green
   0xAFA41, 0xBBA11, 0x7FD00, // 11 fg green fg green
   0x6017F, 0x6017F, 0x6017F, // 00 bg gray2 (light gray) bg gray2 (light gray)
   0xDFEC1, 0x602E7, 0xDFD00, // 10 fg yellow bg gray2 (light gray)
   0x3057F, 0x8617F, 0xC017F, // 01 bg gray2 (light gray) fg yellow
   0x8FAC1, 0x862E7, 0x7FD00, // 11 fg yellow fg yellow
   0x6017F, 0x6017F, 0x6017F, // 00 bg gray2 (light gray) bg gray2 (light gray)
   0x60271, 0x602F7, 0x6023D, // 10 fg aqua bg gray2 (light gray)
   0xA397F, 0x8217F, 0xB097F, // 01 bg gray2 (light gray) fg aqua
   0xA3A71, 0x822F7, 0xB0A3D, // 11 fg aqua fg aqua
   0x6017F, 0x6017F, 0x6017F, // 00 bg gray2 (light gray) bg gray2 (light gray)
   0x602FA, 0x602FA, 0x602FA, // 10 fg white bg gray2 (light gray)
   0x8157F, 0x8157F, 0x8157F, // 01 bg gray2 (light gray) fg white
   0x816FA, 0x816FA, 0x816FA, // 11 fg white fg white
   0x80EFC, 0x8BED0, 0x8FAC1, // 00 bg pink bg pink
   0x3F100, 0x8BD00, 0x8F900, // 10 fg black bg pink
   0xC02FC, 0x7FED0, 0x7FEC1, // 01 bg pink fg black
   0x7FD00, 0x7FD00, 0x7FD00, // 11 fg black fg black
   0x80EFC, 0x8BED0, 0x8FAC1, // 00 bg pink bg pink
   0x3F220, 0x8BD09, 0x8F938, // 10 fg magenta bg pink
   0x082FC, 0x7DAD0, 0x71EC1, // 01 bg pink fg magenta
   0xB7E20, 0x7D909, 0x71D38, // 11 fg magenta fg magenta
   0x80EFC, 0x8BED0, 0x8FAC1, // 00 bg pink bg pink
   0x3F138, 0x341EE, 0x8FA0C, // 10 fg darkblue bg pink
   0xCE2FC, 0xFBAD0, 0xBCEC1, // 01 bg pink fg darkblue
   0x71D38, 0x445EE, 0xBCE0C, // 11 fg darkblue fg darkblue
   0x80EFC, 0x8BED0, 0x8FAC1, // 00 bg pink bg pink
   0x80EE7, 0x8BDC1, 0x8FA00, // 10 fg purple bg pink
   0x862FC, 0x4FAD0, 0xBFEC1, // 01 bg pink fg purple
   0x862E7, 0x4F9C1, 0xBFE00, // 11 fg purple fg purple
   0x80EFC, 0x8BED0, 0x8FAC1, // 00 bg pink bg pink
   0x80DF7, 0x8BE86, 0x8F93C, // 10 fg darkgreen bg pink
   0x422FC, 0x9E6D0, 0x70EC1, // 01 bg pink fg darkgreen
   0x421F7, 0x9E686, 0x70D3C, // 11 fg darkgreen fg darkgreen
   0x80EFC, 0x8BED0, 0x8FAC1, // 00 bg pink bg pink
   0x80D7F, 0x3417F, 0x3057F, // 10 fg gray1 (dark gray) bg pink
   0x602FC, 0xDFED0, 0xDFEC1, // 01 bg pink fg gray1 (dark gray)
   0x6017F, 0x6017F, 0x6017F, // 11 fg gray1 (dark gray) fg gray1 (dark gray)
   0x80EFC, 0x8BED0, 0x8FAC1, // 00 bg pink bg pink
   0x80E47, 0x8BD8E, 0x8FA00, // 10 fg mediumblue bg pink
   0xAE2FC, 0x5C6D0, 0xBFEC1, // 01 bg pink fg mediumblue
   0xAE247, 0x5C58E, 0xBFE00, // 11 fg mediumblue fg mediumblue
   0x80EFC, 0x8BED0, 0x8FAC1, // 00 bg pink bg pink
   0x80E3D, 0x342DF, 0x8FA00, // 10 fg lightblue bg pink
   0xB0AFC, 0x37ED0, 0xBFEC1, // 01 bg pink fg lightblue
   0xB0A3D, 0x882DF, 0xBFE00, // 11 fg lightblue fg lightblue
   0x80EFC, 0x8BED0, 0x8FAC1, // 00 bg pink bg pink
   0x80DC7, 0x8BD23, 0x8F900, // 10 fg brown bg pink
   0x4E2FC, 0x772D0, 0x7FEC1, // 01 bg pink fg brown
   0x4E1C7, 0x77123, 0x7FD00, // 11 fg brown fg brown
   0x80EFC, 0x8BED0, 0x8FAC1, // 00 bg pink bg pink
   0x80E1D, 0x8BE84, 0x8F900, // 10 fg orange bg pink
   0xB8AFC, 0x9EED0, 0x7FEC1, // 01 bg pink fg orange
   0xB8A1D, 0x9EE84, 0x7FD00, // 11 fg orange fg orange
   0x80EFC, 0x8BED0, 0x8FAC1, // 00 bg pink bg pink
   0x80D7F, 0x3417F, 0x3057F, // 10 fg gray2 (light gray) bg pink
   0x602FC, 0xDFED0, 0xDFEC1, // 01 bg pink fg gray2 (light gray)
   0x6017F, 0x6017F, 0x6017F, // 11 fg gray2 (light gray) fg gray2 (light gray)
   0x80EFC, 0x8BED0, 0x8FAC1, // 00 bg pink bg pink
   0x80EFC, 0x8BED0, 0x8FAC1, // 10 fg pink bg pink
   0x80EFC, 0x8BED0, 0x8FAC1, // 01 bg pink fg pink
   0x80EFC, 0x8BED0, 0x8FAC1, // 11 fg pink fg pink
   0x80EFC, 0x8BED0, 0x8FAC1, // 00 bg pink bg pink
   0x3F241, 0x8BE11, 0x8F900, // 10 fg green bg pink
   0x106FC, 0xBBAD0, 0x7FEC1, // 01 bg pink fg green
   0xAFA41, 0xBBA11, 0x7FD00, // 11 fg green fg green
   0x80EFC, 0x8BED0, 0x8FAC1, // 00 bg pink bg pink
   0x3F2C1, 0x342E7, 0x8F900, // 10 fg yellow bg pink
   0x306FC, 0x39ED0, 0x7FEC1, // 01 bg pink fg yellow
   0x8FAC1, 0x862E7, 0x7FD00, // 11 fg yellow fg yellow
   0x80EFC, 0x8BED0, 0x8FAC1, // 00 bg pink bg pink
   0x80E71, 0x342F7, 0x3063D, // 10 fg aqua bg pink
   0xA3AFC, 0x3DED0, 0x0F6C1, // 01 bg pink fg aqua
   0xA3A71, 0x822F7, 0xB0A3D, // 11 fg aqua fg aqua
   0x80EFC, 0x8BED0, 0x8FAC1, // 00 bg pink bg pink
   0x80EFA, 0x342FA, 0x306FA, // 10 fg white bg pink
   0x816FC, 0x3EAD0, 0x3EAC1, // 01 bg pink fg white
   0x816FA, 0x816FA, 0x816FA, // 11 fg white fg white
   0xAFA41, 0xBBA11, 0x7FD00, // 00 bg green bg green
   0xAF900, 0xBB900, 0x7FD00, // 10 fg black bg green
   0x7FE41, 0x7FE11, 0x7FD00, // 01 bg green fg black
   0x7FD00, 0x7FD00, 0x7FD00, // 11 fg black fg black
   0xAFA41, 0xBBA11, 0x7FD00, // 00 bg green bg green
   0xAFA20, 0xBB909, 0x7FD38, // 10 fg magenta bg green
   0xB7E41, 0x7DA11, 0x71D00, // 01 bg green fg magenta
   0xB7E20, 0x7D909, 0x71D38, // 11 fg magenta fg magenta
   0xAFA41, 0xBBA11, 0x7FD00, // 00 bg green bg green
   0xAF938, 0x045EE, 0x7FE0C, // 10 fg darkblue bg green
   0x71E41, 0xFBA11, 0xBCD00, // 01 bg green fg darkblue
   0x71D38, 0x445EE, 0xBCE0C, // 11 fg darkblue fg darkblue
   0xAFA41, 0xBBA11, 0x7FD00, // 00 bg green bg green
   0x106E7, 0xBB9C1, 0x7FE00, // 10 fg purple bg green
   0x39E41, 0x4FA11, 0xBFD00, // 01 bg green fg purple
   0x862E7, 0x4F9C1, 0xBFE00, // 11 fg purple fg purple
   0xAFA41, 0xBBA11, 0x7FD00, // 00 bg green bg green
   0x105F7, 0xBBA86, 0x7FD3C, // 10 fg darkgreen bg green
   0xFDE41, 0x9E611, 0x70D00, // 01 bg green fg darkgreen
   0x421F7, 0x9E686, 0x70D3C, // 11 fg darkgreen fg darkgreen
   0xAFA41, 0xBBA11, 0x7FD00, // 00 bg green bg green
   0x1057F, 0x0457F, 0xC017F, // 10 fg gray1 (dark gray) bg green
   0xDFE41, 0xDFE11, 0xDFD00, // 01 bg green fg gray1 (dark gray)
   0x6017F, 0x6017F, 0x6017F, // 11 fg gray1 (dark gray) fg gray1 (dark gray)
   0xAFA41, 0xBBA11, 0x7FD00, // 00 bg green bg green
   0xAFA47, 0xBB98E, 0x7FE00, // 10 fg mediumblue bg green
   0xAE241, 0x5C611, 0xBFD00, // 01 bg green fg mediumblue
   0xAE247, 0x5C58E, 0xBFE00, // 11 fg mediumblue fg mediumblue
   0xAFA41, 0xBBA11, 0x7FD00, // 00 bg green bg green
   0x1063D, 0x046DF, 0x7FE00, // 10 fg lightblue bg green
   0x0F641, 0x37E11, 0xBFD00, // 01 bg green fg lightblue
   0xB0A3D, 0x882DF, 0xBFE00, // 11 fg lightblue fg lightblue
   0xAFA41, 0xBBA11, 0x7FD00, // 00 bg green bg green
   0x105C7, 0xBB923, 0x7FD00, // 10 fg brown bg green
   0xF1E41, 0x77211, 0x7FD00, // 01 bg green fg brown
   0x4E1C7, 0x77123, 0x7FD00, // 11 fg brown fg brown
   0xAFA41, 0xBBA11, 0x7FD00, // 00 bg green bg green
   0xAFA1D, 0xBBA84, 0x7FD00, // 10 fg orange bg green
   0xB8A41, 0x9EE11, 0x7FD00, // 01 bg green fg orange
   0xB8A1D, 0x9EE84, 0x7FD00, // 11 fg orange fg orange
   0xAFA41, 0xBBA11, 0x7FD00, // 00 bg green bg green
   0x1057F, 0x0457F, 0xC017F, // 10 fg gray2 (light gray) bg green
   0xDFE41, 0xDFE11, 0xDFD00, // 01 bg green fg gray2 (light gray)
   0x6017F, 0x6017F, 0x6017F, // 11 fg gray2 (light gray) fg gray2 (light gray)
   0xAFA41, 0xBBA11, 0x7FD00, // 00 bg green bg green
   0x106FC, 0xBBAD0, 0x7FEC1, // 10 fg pink bg green
   0x3F241, 0x8BE11, 0x8F900, // 01 bg green fg pink
   0x80EFC, 0x8BED0, 0x8FAC1, // 11 fg pink fg pink
   0xAFA41, 0xBBA11, 0x7FD00, // 00 bg green bg green
   0xAFA41, 0xBBA11, 0x7FD00, // 10 fg green bg green
   0xAFA41, 0xBBA11, 0x7FD00, // 01 bg green fg green
   0xAFA41, 0xBBA11, 0x7FD00, // 11 fg green fg green
   0xAFA41, 0xBBA11, 0x7FD00, // 00 bg green bg green
   0xAFAC1, 0x046E7, 0x7FD00, // 10 fg yellow bg green
   0x8FA41, 0x39E11, 0x7FD00, // 01 bg green fg yellow
   0x8FAC1, 0x862E7, 0x7FD00, // 11 fg yellow fg yellow
   0xAFA41, 0xBBA11, 0x7FD00, // 00 bg green bg green
   0xAFA71, 0x046F7, 0xC023D, // 10 fg aqua bg green
   0xA3A41, 0x3DE11, 0x0F500, // 01 bg green fg aqua
   0xA3A71, 0x822F7, 0xB0A3D, // 11 fg aqua fg aqua
   0xAFA41, 0xBBA11, 0x7FD00, // 00 bg green bg green
   0x106FA, 0x046FA, 0xC02FA, // 10 fg white bg green
   0x3EA41, 0x3EA11, 0x3E900, // 01 bg green fg white
   0x816FA, 0x816FA, 0x816FA, // 11 fg white fg white
   0x8FAC1, 0x862E7, 0x7FD00, // 00 bg yellow bg yellow
   0x8F900, 0x39D00, 0x7FD00, // 10 fg black bg yellow
   0x7FEC1, 0xC02E7, 0x7FD00, // 01 bg yellow fg black
   0x7FD00, 0x7FD00, 0x7FD00, // 11 fg black fg black
   0x8FAC1, 0x862E7, 0x7FD00, // 00 bg yellow bg yellow
   0x8FA20, 0x39D09, 0x7FD38, // 10 fg magenta bg yellow
   0xB7EC1, 0xC26E7, 0x71D00, // 01 bg yellow fg magenta
   0xB7E20, 0x7D909, 0x71D38, // 11 fg magenta fg magenta
   0x8FAC1, 0x862E7, 0x7FD00, // 00 bg yellow bg yellow
   0x8F938, 0x861EE, 0x7FE0C, // 10 fg darkblue bg yellow
   0x71EC1, 0x446E7, 0xBCD00, // 01 bg yellow fg darkblue
   0x71D38, 0x445EE, 0xBCE0C, // 11 fg darkblue fg darkblue
   0x8FAC1, 0x862E7, 0x7FD00, // 00 bg yellow bg yellow
   0x306E7, 0x39DC1, 0x7FE00, // 10 fg purple bg yellow
   0x39EC1, 0xF06E7, 0xBFD00, // 01 bg yellow fg purple
   0x862E7, 0x4F9C1, 0xBFE00, // 11 fg purple fg purple
   0x8FAC1, 0x862E7, 0x7FD00, // 00 bg yellow bg yellow
   0x305F7, 0x39E86, 0x7FD3C, // 10 fg darkgreen bg yellow
   0xFDEC1, 0x21AE7, 0x70D00, // 01 bg yellow fg darkgreen
   0x421F7, 0x9E686, 0x70D3C, // 11 fg darkgreen fg darkgreen
   0x8FAC1, 0x862E7, 0x7FD00, // 00 bg yellow bg yellow
   0x3057F, 0x8617F, 0xC017F, // 10 fg gray1 (dark gray) bg yellow
   0xDFEC1, 0x602E7, 0xDFD00, // 01 bg yellow fg gray1 (dark gray)
   0x6017F, 0x6017F, 0x6017F, // 11 fg gray1 (dark gray) fg gray1 (dark gray)
   0x8FAC1, 0x862E7, 0x7FD00, // 00 bg yellow bg yellow
   0x8FA47, 0x8618E, 0x7FE00, // 10 fg mediumblue bg yellow
   0xAE2C1, 0x5C6E7, 0xBFD00, // 01 bg yellow fg mediumblue
   0xAE247, 0x5C58E, 0xBFE00, // 11 fg mediumblue fg mediumblue
   0x8FAC1, 0x862E7, 0x7FD00, // 00 bg yellow bg yellow
   0x3063D, 0x862DF, 0x7FE00, // 10 fg lightblue bg yellow
   0x0F6C1, 0x882E7, 0xBFD00, // 01 bg yellow fg lightblue
   0xB0A3D, 0x882DF, 0xBFE00, // 11 fg lightblue fg lightblue
   0x8FAC1, 0x862E7, 0x7FD00, // 00 bg yellow bg yellow
   0x305C7, 0x39D23, 0x7FD00, // 10 fg brown bg yellow
   0xF1EC1, 0xC8EE7, 0x7FD00, // 01 bg yellow fg brown
   0x4E1C7, 0x77123, 0x7FD00, // 11 fg brown fg brown
   0x8FAC1, 0x862E7, 0x7FD00, // 00 bg yellow bg yellow
   0x8FA1D, 0x39E84, 0x7FD00, // 10 fg orange bg yellow
   0xB8AC1, 0x212E7, 0x7FD00, // 01 bg yellow fg orange
   0xB8A1D, 0x9EE84, 0x7FD00, // 11 fg orange fg orange
   0x8FAC1, 0x862E7, 0x7FD00, // 00 bg yellow bg yellow
   0x3057F, 0x8617F, 0xC017F, // 10 fg gray2 (light gray) bg yellow
   0xDFEC1, 0x602E7, 0xDFD00, // 01 bg yellow fg gray2 (light gray)
   0x6017F, 0x6017F, 0x6017F, // 11 fg gray2 (light gray) fg gray2 (light gray)
   0x8FAC1, 0x862E7, 0x7FD00, // 00 bg yellow bg yellow
   0x306FC, 0x39ED0, 0x7FEC1, // 10 fg pink bg yellow
   0x3F2C1, 0x342E7, 0x8F900, // 01 bg yellow fg pink
   0x80EFC, 0x8BED0, 0x8FAC1, // 11 fg pink fg pink
   0x8FAC1, 0x862E7, 0x7FD00, // 00 bg yellow bg yellow
   0x8FA41, 0x39E11, 0x7FD00, // 10 fg green bg yellow
   0xAFAC1, 0x046E7, 0x7FD00, // 01 bg yellow fg green
   0xAFA41, 0xBBA11, 0x7FD00, // 11 fg green fg green
   0x8FAC1, 0x862E7, 0x7FD00, // 00 bg yellow bg yellow
   0x8FAC1, 0x862E7, 0x7FD00, // 10 fg yellow bg yellow
   0x8FAC1, 0x862E7, 0x7FD00, // 01 bg yellow fg yellow
   0x8FAC1, 0x862E7, 0x7FD00, // 11 fg yellow fg yellow
   0x8FAC1, 0x862E7, 0x7FD00, // 00 bg yellow bg yellow
   0x8FA71, 0x862F7, 0xC023D, // 10 fg aqua bg yellow
   0xA3AC1, 0x822E7, 0x0F500, // 01 bg yellow fg aqua
   0xA3A71, 0x822F7, 0xB0A3D, // 11 fg aqua fg aqua
   0x8FAC1, 0x862E7, 0x7FD00, // 00 bg yellow bg yellow
   0x306FA, 0x862FA, 0xC02FA, // 10 fg white bg yellow
   0x3EAC1, 0x816E7, 0x3E900, // 01 bg yellow fg white
   0x816FA, 0x816FA, 0x816FA, // 11 fg white fg white
   0xA3A71, 0x822F7, 0xB0A3D, // 00 bg aqua bg aqua
   0xA3900, 0x3DD00, 0x0F500, // 10 fg black bg aqua
   0x7FE71, 0xC02F7, 0xC023D, // 01 bg aqua fg black
   0x7FD00, 0x7FD00, 0x7FD00, // 11 fg black fg black
   0xA3A71, 0x822F7, 0xB0A3D, // 00 bg aqua bg aqua
   0xA3A20, 0x3DD09, 0x0F538, // 10 fg magenta bg aqua
   0xB7E71, 0xC26F7, 0xCE23D, // 01 bg aqua fg magenta
   0xB7E20, 0x7D909, 0x71D38, // 11 fg magenta fg magenta
   0xA3A71, 0x822F7, 0xB0A3D, // 00 bg aqua bg aqua
   0xA3938, 0x821EE, 0x0F60C, // 10 fg darkblue bg aqua
   0x71E71, 0x446F7, 0x0323D, // 01 bg aqua fg darkblue
   0x71D38, 0x445EE, 0xBCE0C, // 11 fg darkblue fg darkblue
   0xA3A71, 0x822F7, 0xB0A3D, // 00 bg aqua bg aqua
   0xA3AE7, 0x3DDC1, 0x0F600, // 10 fg purple bg aqua
   0x86271, 0xF06F7, 0x0023D, // 01 bg aqua fg purple
   0x862E7, 0x4F9C1, 0xBFE00, // 11 fg purple fg purple
   0xA3A71, 0x822F7, 0xB0A3D, // 00 bg aqua bg aqua
   0xA39F7, 0x3DE86, 0xB093C, // 10 fg darkgreen bg aqua
   0x42271, 0x21AF7, 0x70E3D, // 01 bg aqua fg darkgreen
   0x421F7, 0x9E686, 0x70D3C, // 11 fg darkgreen fg darkgreen
   0xA3A71, 0x822F7, 0xB0A3D, // 00 bg aqua bg aqua
   0xA397F, 0x8217F, 0xB097F, // 10 fg gray1 (dark gray) bg aqua
   0x60271, 0x602F7, 0x6023D, // 01 bg aqua fg gray1 (dark gray)
   0x6017F, 0x6017F, 0x6017F, // 11 fg gray1 (dark gray) fg gray1 (dark gray)
   0xA3A71, 0x822F7, 0xB0A3D, // 00 bg aqua bg aqua
   0xA3A47, 0x8218E, 0x0F600, // 10 fg mediumblue bg aqua
   0xAE271, 0x5C6F7, 0x0023D, // 01 bg aqua fg mediumblue
   0xAE247, 0x5C58E, 0xBFE00, // 11 fg mediumblue fg mediumblue
   0xA3A71, 0x822F7, 0xB0A3D, // 00 bg aqua bg aqua
   0xA3A3D, 0x822DF, 0x0F600, // 10 fg lightblue bg aqua
   0xB0A71, 0x882F7, 0x0023D, // 01 bg aqua fg lightblue
   0xB0A3D, 0x882DF, 0xBFE00, // 11 fg lightblue fg lightblue
   0xA3A71, 0x822F7, 0xB0A3D, // 00 bg aqua bg aqua
   0xA39C7, 0x3DD23, 0x0F500, // 10 fg brown bg aqua
   0x4E271, 0xC8EF7, 0xC023D, // 01 bg aqua fg brown
   0x4E1C7, 0x77123, 0x7FD00, // 11 fg brown fg brown
   0xA3A71, 0x822F7, 0xB0A3D, // 00 bg aqua bg aqua
   0xA3A1D, 0x3DE84, 0x0F500, // 10 fg orange bg aqua
   0xB8A71, 0x212F7, 0xC023D, // 01 bg aqua fg orange
   0xB8A1D, 0x9EE84, 0x7FD00, // 11 fg orange fg orange
   0xA3A71, 0x822F7, 0xB0A3D, // 00 bg aqua bg aqua
   0xA397F, 0x8217F, 0xB097F, // 10 fg gray2 (light gray) bg aqua
   0x60271, 0x602F7, 0x6023D, // 01 bg aqua fg gray2 (light gray)
   0x6017F, 0x6017F, 0x6017F, // 11 fg gray2 (light gray) fg gray2 (light gray)
   0xA3A71, 0x822F7, 0xB0A3D, // 00 bg aqua bg aqua
   0xA3AFC, 0x3DED0, 0x0F6C1, // 10 fg pink bg aqua
   0x80E71, 0x342F7, 0x3063D, // 01 bg aqua fg pink
   0x80EFC, 0x8BED0, 0x8FAC1, // 11 fg pink fg pink
   0xA3A71, 0x822F7, 0xB0A3D, // 00 bg aqua bg aqua
   0xA3A41, 0x3DE11, 0x0F500, // 10 fg green bg aqua
   0xAFA71, 0x046F7, 0xC023D, // 01 bg aqua fg green
   0xAFA41, 0xBBA11, 0x7FD00, // 11 fg green fg green
   0xA3A71, 0x822F7, 0xB0A3D, // 00 bg aqua bg aqua
   0xA3AC1, 0x822E7, 0x0F500, // 10 fg yellow bg aqua
   0x8FA71, 0x862F7, 0xC023D, // 01 bg aqua fg yellow
   0x8FAC1, 0x862E7, 0x7FD00, // 11 fg yellow fg yellow
   0xA3A71, 0x822F7, 0xB0A3D, // 00 bg aqua bg aqua
   0xA3A71, 0x822F7, 0xB0A3D, // 10 fg aqua bg aqua
   0xA3A71, 0x822F7, 0xB0A3D, // 01 bg aqua fg aqua
   0xA3A71, 0x822F7, 0xB0A3D, // 11 fg aqua fg aqua
   0xA3A71, 0x822F7, 0xB0A3D, // 00 bg aqua bg aqua
   0xA3AFA, 0x822FA, 0xB0AFA, // 10 fg white bg aqua
   0x81671, 0x816F7, 0x8163D, // 01 bg aqua fg white
   0x816FA, 0x816FA, 0x816FA, // 11 fg white fg white
   0x816FA, 0x816FA, 0x816FA, // 00 bg white bg white
   0x3E900, 0x3E900, 0x3E900, // 10 fg black bg white
   0xC02FA, 0xC02FA, 0xC02FA, // 01 bg white fg black
   0x7FD00, 0x7FD00, 0x7FD00, // 11 fg black fg black
   0x816FA, 0x816FA, 0x816FA, // 00 bg white bg white
   0x3EA20, 0x3E909, 0x3E938, // 10 fg magenta bg white
   0x082FA, 0xC26FA, 0xCE2FA, // 01 bg white fg magenta
   0xB7E20, 0x7D909, 0x71D38, // 11 fg magenta fg magenta
   0x816FA, 0x816FA, 0x816FA, // 00 bg white bg white
   0x3E938, 0x815EE, 0x3EA0C, // 10 fg darkblue bg white
   0xCE2FA, 0x446FA, 0x032FA, // 01 bg white fg darkblue
   0x71D38, 0x445EE, 0xBCE0C, // 11 fg darkblue fg darkblue
   0x816FA, 0x816FA, 0x816FA, // 00 bg white bg white
   0x816E7, 0x3E9C1, 0x3EA00, // 10 fg purple bg white
   0x862FA, 0xF06FA, 0x002FA, // 01 bg white fg purple
   0x862E7, 0x4F9C1, 0xBFE00, // 11 fg purple fg purple
   0x816FA, 0x816FA, 0x816FA, // 00 bg white bg white
   0x815F7, 0x3EA86, 0x8153C, // 10 fg darkgreen bg white
   0x422FA, 0x21AFA, 0x70EFA, // 01 bg white fg darkgreen
   0x421F7, 0x9E686, 0x70D3C, // 11 fg darkgreen fg darkgreen
   0x816FA, 0x816FA, 0x816FA, // 00 bg white bg white
   0x8157F, 0x8157F, 0x8157F, // 10 fg gray1 (dark gray) bg white
   0x602FA, 0x602FA, 0x602FA, // 01 bg white fg gray1 (dark gray)
   0x6017F, 0x6017F, 0x6017F, // 11 fg gray1 (dark gray) fg gray1 (dark gray)
   0x816FA, 0x816FA, 0x816FA, // 00 bg white bg white
   0x81647, 0x8158E, 0x3EA00, // 10 fg mediumblue bg white
   0xAE2FA, 0x5C6FA, 0x002FA, // 01 bg white fg mediumblue
   0xAE247, 0x5C58E, 0xBFE00, // 11 fg mediumblue fg mediumblue
   0x816FA, 0x816FA, 0x816FA, // 00 bg white bg white
   0x8163D, 0x816DF, 0x3EA00, // 10 fg lightblue bg white
   0xB0AFA, 0x882FA, 0x002FA, // 01 bg white fg lightblue
   0xB0A3D, 0x882DF, 0xBFE00, // 11 fg lightblue fg lightblue
   0x816FA, 0x816FA, 0x816FA, // 00 bg white bg white
   0x815C7, 0x3E923, 0x3E900, // 10 fg brown bg white
   0x4E2FA, 0xC8EFA, 0xC02FA, // 01 bg white fg brown
   0x4E1C7, 0x77123, 0x7FD00, // 11 fg brown fg brown
   0x816FA, 0x816FA, 0x816FA, // 00 bg white bg white
   0x8161D, 0x3EA84, 0x3E900, // 10 fg orange bg white
   0xB8AFA, 0x212FA, 0xC02FA, // 01 bg white fg orange
   0xB8A1D, 0x9EE84, 0x7FD00, // 11 fg orange fg orange
   0x816FA, 0x816FA, 0x816FA, // 00 bg white bg white
   0x8157F, 0x8157F, 0x8157F, // 10 fg gray2 (light gray) bg white
   0x602FA, 0x602FA, 0x602FA, // 01 bg white fg gray2 (light gray)
   0x6017F, 0x6017F, 0x6017F, // 11 fg gray2 (light gray) fg gray2 (light gray)
   0x816FA, 0x816FA, 0x816FA, // 00 bg white bg white
   0x816FC, 0x3EAD0, 0x3EAC1, // 10 fg pink bg white
   0x80EFA, 0x342FA, 0x306FA, // 01 bg white fg pink
   0x80EFC, 0x8BED0, 0x8FAC1, // 11 fg pink fg pink
   0x816FA, 0x816FA, 0x816FA, // 00 bg white bg white
   0x3EA41, 0x3EA11, 0x3E900, // 10 fg green bg white
   0x106FA, 0x046FA, 0xC02FA, // 01 bg white fg green
   0xAFA41, 0xBBA11, 0x7FD00, // 11 fg green fg green
   0x816FA, 0x816FA, 0x816FA, // 00 bg white bg white
   0x3EAC1, 0x816E7, 0x3E900, // 10 fg yellow bg white
   0x306FA, 0x862FA, 0xC02FA, // 01 bg white fg yellow
   0x8FAC1, 0x862E7, 0x7FD00, // 11 fg yellow fg yellow
   0x816FA, 0x816FA, 0x816FA, // 00 bg white bg white
   0x81671, 0x816F7, 0x8163D, // 10 fg aqua bg white
   0xA3AFA, 0x822FA, 0xB0AFA, // 01 bg white fg aqua
   0xA3A71, 0x822F7, 0xB0A3D, // 11 fg aqua fg aqua
   0x816FA, 0x816FA, 0x816FA, // 00 bg white bg white
   0x816FA, 0x816FA, 0x816FA, // 10 fg white bg white
   0x816FA, 0x816FA, 0x816FA, // 01 bg white fg white
   0x816FA, 0x816FA, 0x816FA, // 11 fg white fg white
    // tmds_lores_improved
   0x7FD00, 0x7FD00, 0x7FD00, // 00 bg black bg black
   0x7FD00, 0x7FD00, 0x7FD00, // 10 fg black bg black
   0x7FD00, 0x7FD00, 0x7FD00, // 01 bg black fg black
   0x7FD00, 0x7FD00, 0x7FD00, // 11 fg black fg black
   0x7FD00, 0x7FD00, 0x7FD00, // 00 bg black bg black
   0x7FE1C, 0x7FEA0, 0xC013E, // 10 fg magenta bg black
   0xB8D00, 0x97D00, 0xCF900, // 01 bg black fg magenta
   0xB8E1C, 0x97EA0, 0x7053E, // 11 fg magenta fg magenta
   0x7FD00, 0x7FD00, 0x7FD00, // 00 bg black bg black
   0x7FD04, 0x7FD0F, 0x7FD9C, // 10 fg darkblue bg black
   0x7ED00, 0x7C100, 0x58D00, // 01 bg black fg darkblue
   0x7ED04, 0x7C10F, 0x58D9C, // 11 fg darkblue fg darkblue
   0x7FD00, 0x7FD00, 0x7FD00, // 00 bg black bg black
   0x7FEE1, 0x7FDC1, 0xC015F, // 10 fg purple bg black
   0x87900, 0x4F900, 0xD7D00, // 01 bg black fg purple
   0x87AE1, 0x4F9C1, 0x6815F, // 11 fg purple fg purple
   0x7FD00, 0x7FD00, 0x7FD00, // 00 bg black bg black
   0xC01F4, 0xC017E, 0xC01EF, // 10 fg darkgreen bg black
   0xFD100, 0xDF900, 0xFBD00, // 01 bg black fg darkgreen
   0x42DF4, 0x6057E, 0x441EF, // 11 fg darkgreen fg darkgreen
   0x7FD00, 0x7FD00, 0x7FD00, // 00 bg black bg black
   0x7FD21, 0x7FD21, 0x7FD21, // 10 fg gray1 (dark gray) bg black
   0x77900, 0x77900, 0x77900, // 01 bg black fg gray1 (dark gray)
   0x77921, 0x77921, 0x77921, // 11 fg gray1 (dark gray) fg gray1 (dark gray)
   0x7FD00, 0x7FD00, 0x7FD00, // 00 bg black bg black
   0x7FD17, 0xC02BE, 0x7FE00, // 10 fg mediumblue bg black
   0x7A100, 0x2F900, 0xBFD00, // 01 bg black fg mediumblue
   0x7A117, 0x906BE, 0xBFE00, // 11 fg mediumblue fg mediumblue
   0x7FD00, 0x7FD00, 0x7FD00, // 00 bg black bg black
   0x7FE83, 0x7FEC4, 0x7FE00, // 10 fg lightblue bg black
   0x9F100, 0x8ED00, 0xBFD00, // 01 bg black fg lightblue
   0x9F283, 0x8EEC4, 0xBFE00, // 11 fg lightblue fg lightblue
   0x7FD00, 0x7FD00, 0x7FD00, // 00 bg black bg black
   0xC018F, 0xC01DC, 0x7FD00, // 10 fg brown bg black
   0xE3D00, 0xF7100, 0x7FD00, // 01 bg black fg brown
   0x5C18F, 0x48DDC, 0x7FD00, // 11 fg brown fg brown
   0x7FD00, 0x7FD00, 0x7FD00, // 00 bg black bg black
   0x7FE03, 0x7FE78, 0x7FD00, // 10 fg orange bg black
   0xBF100, 0xA1D00, 0x7FD00, // 01 bg black fg orange
   0xBF203, 0xA1E78, 0x7FD00, // 11 fg orange fg orange
   0x7FD00, 0x7FD00, 0x7FD00, // 00 bg black bg black
   0x7FEC4, 0x7FEC4, 0x7FEC4, // 10 fg gray2 (light gray) bg black
   0x8ED00, 0x8ED00, 0x8ED00, // 01 bg black fg gray2 (light gray)
   0x8EEC4, 0x8EEC4, 0x8EEC4, // 11 fg gray2 (light gray) fg gray2 (light gray)
   0x7FD00, 0x7FD00, 0x7FD00, // 00 bg black bg black
   0xC02FC, 0x7FD63, 0x7FD71, // 10 fg pink bg black
   0x3F100, 0x67100, 0x63900, // 01 bg black fg pink
   0x80EFC, 0x67163, 0x63971, // 11 fg pink fg pink
   0x7FD00, 0x7FD00, 0x7FD00, // 00 bg black bg black
   0x7FDC0, 0xC021F, 0x7FD00, // 10 fg green bg black
   0x4FD00, 0x07D00, 0x7FD00, // 01 bg black fg green
   0x4FDC0, 0xB821F, 0x7FD00, // 11 fg green fg green
   0x7FD00, 0x7FD00, 0x7FD00, // 00 bg black bg black
   0xC02FF, 0xC02FF, 0x7FD00, // 10 fg yellow bg black
   0x3FD00, 0x3FD00, 0x7FD00, // 01 bg black fg yellow
   0x802FF, 0x802FF, 0x7FD00, // 11 fg yellow fg yellow
   0x7FD00, 0x7FD00, 0x7FD00, // 00 bg black bg black
   0x7FE88, 0x7FE01, 0x7FD61, // 10 fg aqua bg black
   0x9DD00, 0xBF900, 0x67900, // 01 bg black fg aqua
   0x9DE88, 0xBFA01, 0x67961, // 11 fg aqua fg aqua
   0x7FD00, 0x7FD00, 0x7FD00, // 00 bg black bg black
   0x7FE00, 0x7FE00, 0x7FE00, // 10 fg white bg black
   0xBFD00, 0xBFD00, 0xBFD00, // 01 bg black fg white
   0xBFE00, 0xBFE00, 0xBFE00, // 11 fg white fg white
   0xB8E1C, 0x97EA0, 0x7053E, // 00 bg magenta bg magenta
   0xB8D00, 0x97D00, 0xCF900, // 10 fg black bg magenta
   0x7FE1C, 0x7FEA0, 0xC013E, // 01 bg magenta fg black
   0x7FD00, 0x7FD00, 0x7FD00, // 11 fg black fg black
   0xB8E1C, 0x97EA0, 0x7053E, // 00 bg magenta bg magenta
   0xB8E1C, 0x97EA0, 0x7053E, // 10 fg magenta bg magenta
   0xB8E1C, 0x97EA0, 0x7053E, // 01 bg magenta fg magenta
   0xB8E1C, 0x97EA0, 0x7053E, // 11 fg magenta fg magenta
   0xB8E1C, 0x97EA0, 0x7053E, // 00 bg magenta bg magenta
   0xB8D04, 0x97D0F, 0x7059C, // 10 fg darkblue bg magenta
   0x7EE1C, 0x7C2A0, 0x58D3E, // 01 bg magenta fg darkblue
   0x7ED04, 0x7C10F, 0x58D9C, // 11 fg darkblue fg darkblue
   0xB8E1C, 0x97EA0, 0x7053E, // 00 bg magenta bg magenta
   0xB8EE1, 0x97DC1, 0x7055F, // 10 fg purple bg magenta
   0x87A1C, 0x4FAA0, 0x6813E, // 01 bg magenta fg purple
   0x87AE1, 0x4F9C1, 0x6815F, // 11 fg purple fg purple
   0xB8E1C, 0x97EA0, 0x7053E, // 00 bg magenta bg magenta
   0x071F4, 0x2817E, 0x705EF, // 10 fg darkgreen bg magenta
   0xFD21C, 0xDFAA0, 0x4413E, // 01 bg magenta fg darkgreen
   0x42DF4, 0x6057E, 0x441EF, // 11 fg darkgreen fg darkgreen
   0xB8E1C, 0x97EA0, 0x7053E, // 00 bg magenta bg magenta
   0xB8D21, 0x97D21, 0xCF921, // 10 fg gray1 (dark gray) bg magenta
   0x77A1C, 0x77AA0, 0xC853E, // 01 bg magenta fg gray1 (dark gray)
   0x77921, 0x77921, 0x77921, // 11 fg gray1 (dark gray) fg gray1 (dark gray)
   0xB8E1C, 0x97EA0, 0x7053E, // 00 bg magenta bg magenta
   0xB8D17, 0x282BE, 0xCFA00, // 10 fg mediumblue bg magenta
   0x7A21C, 0x2FAA0, 0x0013E, // 01 bg magenta fg mediumblue
   0x7A117, 0x906BE, 0xBFE00, // 11 fg mediumblue fg mediumblue
   0xB8E1C, 0x97EA0, 0x7053E, // 00 bg magenta bg magenta
   0xB8E83, 0x97EC4, 0xCFA00, // 10 fg lightblue bg magenta
   0x9F21C, 0x8EEA0, 0x0013E, // 01 bg magenta fg lightblue
   0x9F283, 0x8EEC4, 0xBFE00, // 11 fg lightblue fg lightblue
   0xB8E1C, 0x97EA0, 0x7053E, // 00 bg magenta bg magenta
   0x0718F, 0x281DC, 0xCF900, // 10 fg brown bg magenta
   0xE3E1C, 0xF72A0, 0xC013E, // 01 bg magenta fg brown
   0x5C18F, 0x48DDC, 0x7FD00, // 11 fg brown fg brown
   0xB8E1C, 0x97EA0, 0x7053E, // 00 bg magenta bg magenta
   0xB8E03, 0x97E78, 0xCF900, // 10 fg orange bg magenta
   0xBF21C, 0xA1EA0, 0xC013E, // 01 bg magenta fg orange
   0xBF203, 0xA1E78, 0x7FD00, // 11 fg orange fg orange
   0xB8E1C, 0x97EA0, 0x7053E, // 00 bg magenta bg magenta
   0xB8EC4, 0x97EC4, 0xCFAC4, // 10 fg gray2 (light gray) bg magenta
   0x8EE1C, 0x8EEA0, 0x3113E, // 01 bg magenta fg gray2 (light gray)
   0x8EEC4, 0x8EEC4, 0x8EEC4, // 11 fg gray2 (light gray) fg gray2 (light gray)
   0xB8E1C, 0x97EA0, 0x7053E, // 00 bg magenta bg magenta
   0x072FC, 0x97D63, 0x70571, // 10 fg pink bg magenta
   0x3F21C, 0x672A0, 0x6393E, // 01 bg magenta fg pink
   0x80EFC, 0x67163, 0x63971, // 11 fg pink fg pink
   0xB8E1C, 0x97EA0, 0x7053E, // 00 bg magenta bg magenta
   0xB8DC0, 0x2821F, 0xCF900, // 10 fg green bg magenta
   0x4FE1C, 0x07EA0, 0xC013E, // 01 bg magenta fg green
   0x4FDC0, 0xB821F, 0x7FD00, // 11 fg green fg green
   0xB8E1C, 0x97EA0, 0x7053E, // 00 bg magenta bg magenta
   0x072FF, 0x282FF, 0xCF900, // 10 fg yellow bg magenta
   0x3FE1C, 0x3FEA0, 0xC013E, // 01 bg magenta fg yellow
   0x802FF, 0x802FF, 0x7FD00, // 11 fg yellow fg yellow
   0xB8E1C, 0x97EA0, 0x7053E, // 00 bg magenta bg magenta
   0xB8E88, 0x97E01, 0xCF961, // 10 fg aqua bg magenta
   0x9DE1C, 0xBFAA0, 0xD853E, // 01 bg magenta fg aqua
   0x9DE88, 0xBFA01, 0x67961, // 11 fg aqua fg aqua
   0xB8E1C, 0x97EA0, 0x7053E, // 00 bg magenta bg magenta
   0xB8E00, 0x97E00, 0xCFA00, // 10 fg white bg magenta
   0xBFE1C, 0xBFEA0, 0x0013E, // 01 bg magenta fg white
   0xBFE00, 0xBFE00, 0xBFE00, // 11 fg white fg white
   0x7ED04, 0x7C10F, 0x58D9C, // 00 bg darkblue bg darkblue
   0x7ED00, 0x7C100, 0x58D00, // 10 fg black bg darkblue
   0x7FD04, 0x7FD0F, 0x7FD9C, // 01 bg darkblue fg black
   0x7FD00, 0x7FD00, 0x7FD00, // 11 fg black fg black
   0x7ED04, 0x7C10F, 0x58D9C, // 00 bg darkblue bg darkblue
   0x7EE1C, 0x7C2A0, 0x58D3E, // 10 fg magenta bg darkblue
   0xB8D04, 0x97D0F, 0x7059C, // 01 bg darkblue fg magenta
   0xB8E1C, 0x97EA0, 0x7053E, // 11 fg magenta fg magenta
   0x7ED04, 0x7C10F, 0x58D9C, // 00 bg darkblue bg darkblue
   0x7ED04, 0x7C10F, 0x58D9C, // 10 fg darkblue bg darkblue
   0x7ED04, 0x7C10F, 0x58D9C, // 01 bg darkblue fg darkblue
   0x7ED04, 0x7C10F, 0x58D9C, // 11 fg darkblue fg darkblue
   0x7ED04, 0x7C10F, 0x58D9C, // 00 bg darkblue bg darkblue
   0x7EEE1, 0x7C1C1, 0x58D5F, // 10 fg purple bg darkblue
   0x87904, 0x4F90F, 0x6819C, // 01 bg darkblue fg purple
   0x87AE1, 0x4F9C1, 0x6815F, // 11 fg purple fg purple
   0x7ED04, 0x7C10F, 0x58D9C, // 00 bg darkblue bg darkblue
   0xC11F4, 0x7C17E, 0x58DEF, // 10 fg darkgreen bg darkblue
   0xFD104, 0x6050F, 0x4419C, // 01 bg darkblue fg darkgreen
   0x42DF4, 0x6057E, 0x441EF, // 11 fg darkgreen fg darkgreen
   0x7ED04, 0x7C10F, 0x58D9C, // 00 bg darkblue bg darkblue
   0x7ED21, 0x7C121, 0x58D21, // 10 fg gray1 (dark gray) bg darkblue
   0x77904, 0x7790F, 0x7799C, // 01 bg darkblue fg gray1 (dark gray)
   0x77921, 0x77921, 0x77921, // 11 fg gray1 (dark gray) fg gray1 (dark gray)
   0x7ED04, 0x7C10F, 0x58D9C, // 00 bg darkblue bg darkblue
   0x7ED17, 0x7C2BE, 0x58E00, // 10 fg mediumblue bg darkblue
   0x7A104, 0x9050F, 0xBFD9C, // 01 bg darkblue fg mediumblue
   0x7A117, 0x906BE, 0xBFE00, // 11 fg mediumblue fg mediumblue
   0x7ED04, 0x7C10F, 0x58D9C, // 00 bg darkblue bg darkblue
   0x7EE83, 0x7C2C4, 0x58E00, // 10 fg lightblue bg darkblue
   0x9F104, 0x8ED0F, 0xBFD9C, // 01 bg darkblue fg lightblue
   0x9F283, 0x8EEC4, 0xBFE00, // 11 fg lightblue fg lightblue
   0x7ED04, 0x7C10F, 0x58D9C, // 00 bg darkblue bg darkblue
   0xC118F, 0x7C1DC, 0x58D00, // 10 fg brown bg darkblue
   0xE3D04, 0x48D0F, 0x7FD9C, // 01 bg darkblue fg brown
   0x5C18F, 0x48DDC, 0x7FD00, // 11 fg brown fg brown
   0x7ED04, 0x7C10F, 0x58D9C, // 00 bg darkblue bg darkblue
   0x7EE03, 0x7C278, 0x58D00, // 10 fg orange bg darkblue
   0xBF104, 0xA1D0F, 0x7FD9C, // 01 bg darkblue fg orange
   0xBF203, 0xA1E78, 0x7FD00, // 11 fg orange fg orange
   0x7ED04, 0x7C10F, 0x58D9C, // 00 bg darkblue bg darkblue
   0x7EEC4, 0x7C2C4, 0x58EC4, // 10 fg gray2 (light gray) bg darkblue
   0x8ED04, 0x8ED0F, 0x8ED9C, // 01 bg darkblue fg gray2 (light gray)
   0x8EEC4, 0x8EEC4, 0x8EEC4, // 11 fg gray2 (light gray) fg gray2 (light gray)
   0x7ED04, 0x7C10F, 0x58D9C, // 00 bg darkblue bg darkblue
   0xC12FC, 0x7C163, 0x58D71, // 10 fg pink bg darkblue
   0x3F104, 0x6710F, 0x6399C, // 01 bg darkblue fg pink
   0x80EFC, 0x67163, 0x63971, // 11 fg pink fg pink
   0x7ED04, 0x7C10F, 0x58D9C, // 00 bg darkblue bg darkblue
   0x7EDC0, 0x7C21F, 0x58D00, // 10 fg green bg darkblue
   0x4FD04, 0xB810F, 0x7FD9C, // 01 bg darkblue fg green
   0x4FDC0, 0xB821F, 0x7FD00, // 11 fg green fg green
   0x7ED04, 0x7C10F, 0x58D9C, // 00 bg darkblue bg darkblue
   0xC12FF, 0x7C2FF, 0x58D00, // 10 fg yellow bg darkblue
   0x3FD04, 0x8010F, 0x7FD9C, // 01 bg darkblue fg yellow
   0x802FF, 0x802FF, 0x7FD00, // 11 fg yellow fg yellow
   0x7ED04, 0x7C10F, 0x58D9C, // 00 bg darkblue bg darkblue
   0x7EE88, 0x7C201, 0x58D61, // 10 fg aqua bg darkblue
   0x9DD04, 0xBF90F, 0x6799C, // 01 bg darkblue fg aqua
   0x9DE88, 0xBFA01, 0x67961, // 11 fg aqua fg aqua
   0x7ED04, 0x7C10F, 0x58D9C, // 00 bg darkblue bg darkblue
   0x7EE00, 0x7C200, 0x58E00, // 10 fg white bg darkblue
   0xBFD04, 0xBFD0F, 0xBFD9C, // 01 bg darkblue fg white
   0xBFE00, 0xBFE00, 0xBFE00, // 11 fg white fg white
   0x87AE1, 0x4F9C1, 0x6815F, // 00 bg purple bg purple
   0x87900, 0x4F900, 0xD7D00, // 10 fg black bg purple
   0x7FEE1, 0x7FDC1, 0xC015F, // 01 bg purple fg black
   0x7FD00, 0x7FD00, 0x7FD00, // 11 fg black fg black
   0x87AE1, 0x4F9C1, 0x6815F, // 00 bg purple bg purple
   0x87A1C, 0x4FAA0, 0x6813E, // 10 fg magenta bg purple
   0xB8EE1, 0x97DC1, 0x7055F, // 01 bg purple fg magenta
   0xB8E1C, 0x97EA0, 0x7053E, // 11 fg magenta fg magenta
   0x87AE1, 0x4F9C1, 0x6815F, // 00 bg purple bg purple
   0x87904, 0x4F90F, 0x6819C, // 10 fg darkblue bg purple
   0x7EEE1, 0x7C1C1, 0x58D5F, // 01 bg purple fg darkblue
   0x7ED04, 0x7C10F, 0x58D9C, // 11 fg darkblue fg darkblue
   0x87AE1, 0x4F9C1, 0x6815F, // 00 bg purple bg purple
   0x87AE1, 0x4F9C1, 0x6815F, // 10 fg purple bg purple
   0x87AE1, 0x4F9C1, 0x6815F, // 01 bg purple fg purple
   0x87AE1, 0x4F9C1, 0x6815F, // 11 fg purple fg purple
   0x87AE1, 0x4F9C1, 0x6815F, // 00 bg purple bg purple
   0x879F4, 0xF057E, 0x681EF, // 10 fg darkgreen bg purple
   0x42EE1, 0xDF9C1, 0x4415F, // 01 bg purple fg darkgreen
   0x42DF4, 0x6057E, 0x441EF, // 11 fg darkgreen fg darkgreen
   0x87AE1, 0x4F9C1, 0x6815F, // 00 bg purple bg purple
   0x87921, 0x4F921, 0xD7D21, // 10 fg gray1 (dark gray) bg purple
   0x77AE1, 0x779C1, 0xC855F, // 01 bg purple fg gray1 (dark gray)
   0x77921, 0x77921, 0x77921, // 11 fg gray1 (dark gray) fg gray1 (dark gray)
   0x87AE1, 0x4F9C1, 0x6815F, // 00 bg purple bg purple
   0x87917, 0xF06BE, 0xD7E00, // 10 fg mediumblue bg purple
   0x7A2E1, 0x2F9C1, 0x0015F, // 01 bg purple fg mediumblue
   0x7A117, 0x906BE, 0xBFE00, // 11 fg mediumblue fg mediumblue
   0x87AE1, 0x4F9C1, 0x6815F, // 00 bg purple bg purple
   0x87A83, 0x4FAC4, 0xD7E00, // 10 fg lightblue bg purple
   0x9F2E1, 0x8EDC1, 0x0015F, // 01 bg purple fg lightblue
   0x9F283, 0x8EEC4, 0xBFE00, // 11 fg lightblue fg lightblue
   0x87AE1, 0x4F9C1, 0x6815F, // 00 bg purple bg purple
   0x8798F, 0xF05DC, 0xD7D00, // 10 fg brown bg purple
   0x5C2E1, 0xF71C1, 0xC015F, // 01 bg purple fg brown
   0x5C18F, 0x48DDC, 0x7FD00, // 11 fg brown fg brown
   0x87AE1, 0x4F9C1, 0x6815F, // 00 bg purple bg purple
   0x87A03, 0x4FA78, 0xD7D00, // 10 fg orange bg purple
   0xBF2E1, 0xA1DC1, 0xC015F, // 01 bg purple fg orange
   0xBF203, 0xA1E78, 0x7FD00, // 11 fg orange fg orange
   0x87AE1, 0x4F9C1, 0x6815F, // 00 bg purple bg purple
   0x87AC4, 0x4FAC4, 0xD7EC4, // 10 fg gray2 (light gray) bg purple
   0x8EEE1, 0x8EDC1, 0x3115F, // 01 bg purple fg gray2 (light gray)
   0x8EEC4, 0x8EEC4, 0x8EEC4, // 11 fg gray2 (light gray) fg gray2 (light gray)
   0x87AE1, 0x4F9C1, 0x6815F, // 00 bg purple bg purple
   0x87AFC, 0x4F963, 0x68171, // 10 fg pink bg purple
   0x80EE1, 0x671C1, 0x6395F, // 01 bg purple fg pink
   0x80EFC, 0x67163, 0x63971, // 11 fg pink fg pink
   0x87AE1, 0x4F9C1, 0x6815F, // 00 bg purple bg purple
   0x879C0, 0xF061F, 0xD7D00, // 10 fg green bg purple
   0x4FEE1, 0x07DC1, 0xC015F, // 01 bg purple fg green
   0x4FDC0, 0xB821F, 0x7FD00, // 11 fg green fg green
   0x87AE1, 0x4F9C1, 0x6815F, // 00 bg purple bg purple
   0x87AFF, 0xF06FF, 0xD7D00, // 10 fg yellow bg purple
   0x802E1, 0x3FDC1, 0xC015F, // 01 bg purple fg yellow
   0x802FF, 0x802FF, 0x7FD00, // 11 fg yellow fg yellow
   0x87AE1, 0x4F9C1, 0x6815F, // 00 bg purple bg purple
   0x87A88, 0x4FA01, 0xD7D61, // 10 fg aqua bg purple
   0x9DEE1, 0xBF9C1, 0xD855F, // 01 bg purple fg aqua
   0x9DE88, 0xBFA01, 0x67961, // 11 fg aqua fg aqua
   0x87AE1, 0x4F9C1, 0x6815F, // 00 bg purple bg purple
   0x87A00, 0x4FA00, 0xD7E00, // 10 fg white bg purple
   0xBFEE1, 0xBFDC1, 0x0015F, // 01 bg purple fg white
   0xBFE00, 0xBFE00, 0xBFE00, // 11 fg white fg white
   0x42DF4, 0x6057E, 0x441EF, // 00 bg darkgreen bg darkgreen
   0xFD100, 0xDF900, 0xFBD00, // 10 fg black bg darkgreen
   0xC01F4, 0xC017E, 0xC01EF, // 01 bg darkgreen fg black
   0x7FD00, 0x7FD00, 0x7FD00, // 11 fg black fg black
   0x42DF4, 0x6057E, 0x441EF, // 00 bg darkgreen bg darkgreen
   0xFD21C, 0xDFAA0, 0x4413E, // 10 fg magenta bg darkgreen
   0x071F4, 0x2817E, 0x705EF, // 01 bg darkgreen fg magenta
   0xB8E1C, 0x97EA0, 0x7053E, // 11 fg magenta fg magenta
   0x42DF4, 0x6057E, 0x441EF, // 00 bg darkgreen bg darkgreen
   0xFD104, 0x6050F, 0x4419C, // 10 fg darkblue bg darkgreen
   0xC11F4, 0x7C17E, 0x58DEF, // 01 bg darkgreen fg darkblue
   0x7ED04, 0x7C10F, 0x58D9C, // 11 fg darkblue fg darkblue
   0x42DF4, 0x6057E, 0x441EF, // 00 bg darkgreen bg darkgreen
   0x42EE1, 0xDF9C1, 0x4415F, // 10 fg purple bg darkgreen
   0x879F4, 0xF057E, 0x681EF, // 01 bg darkgreen fg purple
   0x87AE1, 0x4F9C1, 0x6815F, // 11 fg purple fg purple
   0x42DF4, 0x6057E, 0x441EF, // 00 bg darkgreen bg darkgreen
   0x42DF4, 0x6057E, 0x441EF, // 10 fg darkgreen bg darkgreen
   0x42DF4, 0x6057E, 0x441EF, // 01 bg darkgreen fg darkgreen
   0x42DF4, 0x6057E, 0x441EF, // 11 fg darkgreen fg darkgreen
   0x42DF4, 0x6057E, 0x441EF, // 00 bg darkgreen bg darkgreen
   0xFD121, 0xDF921, 0xFBD21, // 10 fg gray1 (dark gray) bg darkgreen
   0xC85F4, 0xC857E, 0xC85EF, // 01 bg darkgreen fg gray1 (dark gray)
   0x77921, 0x77921, 0x77921, // 11 fg gray1 (dark gray) fg gray1 (dark gray)
   0x42DF4, 0x6057E, 0x441EF, // 00 bg darkgreen bg darkgreen
   0x42D17, 0x606BE, 0xFBE00, // 10 fg mediumblue bg darkgreen
   0x7A1F4, 0x9057E, 0x001EF, // 01 bg darkgreen fg mediumblue
   0x7A117, 0x906BE, 0xBFE00, // 11 fg mediumblue fg mediumblue
   0x42DF4, 0x6057E, 0x441EF, // 00 bg darkgreen bg darkgreen
   0xFD283, 0xDFAC4, 0xFBE00, // 10 fg lightblue bg darkgreen
   0x20DF4, 0x3117E, 0x001EF, // 01 bg darkgreen fg lightblue
   0x9F283, 0x8EEC4, 0xBFE00, // 11 fg lightblue fg lightblue
   0x42DF4, 0x6057E, 0x441EF, // 00 bg darkgreen bg darkgreen
   0x42D8F, 0x605DC, 0xFBD00, // 10 fg brown bg darkgreen
   0x5C1F4, 0x48D7E, 0xC01EF, // 01 bg darkgreen fg brown
   0x5C18F, 0x48DDC, 0x7FD00, // 11 fg brown fg brown
   0x42DF4, 0x6057E, 0x441EF, // 00 bg darkgreen bg darkgreen
   0xFD203, 0x60678, 0xFBD00, // 10 fg orange bg darkgreen
   0x00DF4, 0xA1D7E, 0xC01EF, // 01 bg darkgreen fg orange
   0xBF203, 0xA1E78, 0x7FD00, // 11 fg orange fg orange
   0x42DF4, 0x6057E, 0x441EF, // 00 bg darkgreen bg darkgreen
   0xFD2C4, 0xDFAC4, 0xFBEC4, // 10 fg gray2 (light gray) bg darkgreen
   0x311F4, 0x3117E, 0x311EF, // 01 bg darkgreen fg gray2 (light gray)
   0x8EEC4, 0x8EEC4, 0x8EEC4, // 11 fg gray2 (light gray) fg gray2 (light gray)
   0x42DF4, 0x6057E, 0x441EF, // 00 bg darkgreen bg darkgreen
   0x42EFC, 0x60563, 0x44171, // 10 fg pink bg darkgreen
   0x80DF4, 0x6717E, 0x639EF, // 01 bg darkgreen fg pink
   0x80EFC, 0x67163, 0x63971, // 11 fg pink fg pink
   0x42DF4, 0x6057E, 0x441EF, // 00 bg darkgreen bg darkgreen
   0xFD1C0, 0x6061F, 0xFBD00, // 10 fg green bg darkgreen
   0xF01F4, 0xB817E, 0xC01EF, // 01 bg darkgreen fg green
   0x4FDC0, 0xB821F, 0x7FD00, // 11 fg green fg green
   0x42DF4, 0x6057E, 0x441EF, // 00 bg darkgreen bg darkgreen
   0x42EFF, 0x606FF, 0xFBD00, // 10 fg yellow bg darkgreen
   0x801F4, 0x8017E, 0xC01EF, // 01 bg darkgreen fg yellow
   0x802FF, 0x802FF, 0x7FD00, // 11 fg yellow fg yellow
   0x42DF4, 0x6057E, 0x441EF, // 00 bg darkgreen bg darkgreen
   0xFD288, 0xDFA01, 0xFBD61, // 10 fg aqua bg darkgreen
   0x221F4, 0x0057E, 0xD85EF, // 01 bg darkgreen fg aqua
   0x9DE88, 0xBFA01, 0x67961, // 11 fg aqua fg aqua
   0x42DF4, 0x6057E, 0x441EF, // 00 bg darkgreen bg darkgreen
   0xFD200, 0xDFA00, 0xFBE00, // 10 fg white bg darkgreen
   0x001F4, 0x0017E, 0x001EF, // 01 bg darkgreen fg white
   0xBFE00, 0xBFE00, 0xBFE00, // 11 fg white fg white
   0x77921, 0x77921, 0x77921, // 00 bg gray1 (dark gray) bg gray1 (dark gray)
   0x77900, 0x77900, 0x77900, // 10 fg black bg gray1 (dark gray)
   0x7FD21, 0x7FD21, 0x7FD21, // 01 bg gray1 (dark gray) fg black
   0x7FD00, 0x7FD00, 0x7FD00, // 11 fg black fg black
   0x77921, 0x77921, 0x77921, // 00 bg gray1 (dark gray) bg gray1 (dark gray)
   0x77A1C, 0x77AA0, 0xC853E, // 10 fg magenta bg gray1 (dark gray)
   0xB8D21, 0x97D21, 0xCF921, // 01 bg gray1 (dark gray) fg magenta
   0xB8E1C, 0x97EA0, 0x7053E, // 11 fg magenta fg magenta
   0x77921, 0x77921, 0x77921, // 00 bg gray1 (dark gray) bg gray1 (dark gray)
   0x77904, 0x7790F, 0x7799C, // 10 fg darkblue bg gray1 (dark gray)
   0x7ED21, 0x7C121, 0x58D21, // 01 bg gray1 (dark gray) fg darkblue
   0x7ED04, 0x7C10F, 0x58D9C, // 11 fg darkblue fg darkblue
   0x77921, 0x77921, 0x77921, // 00 bg gray1 (dark gray) bg gray1 (dark gray)
   0x77AE1, 0x779C1, 0xC855F, // 10 fg purple bg gray1 (dark gray)
   0x87921, 0x4F921, 0xD7D21, // 01 bg gray1 (dark gray) fg purple
   0x87AE1, 0x4F9C1, 0x6815F, // 11 fg purple fg purple
   0x77921, 0x77921, 0x77921, // 00 bg gray1 (dark gray) bg gray1 (dark gray)
   0xC85F4, 0xC857E, 0xC85EF, // 10 fg darkgreen bg gray1 (dark gray)
   0xFD121, 0xDF921, 0xFBD21, // 01 bg gray1 (dark gray) fg darkgreen
   0x42DF4, 0x6057E, 0x441EF, // 11 fg darkgreen fg darkgreen
   0x77921, 0x77921, 0x77921, // 00 bg gray1 (dark gray) bg gray1 (dark gray)
   0x77921, 0x77921, 0x77921, // 10 fg gray1 (dark gray) bg gray1 (dark gray)
   0x77921, 0x77921, 0x77921, // 01 bg gray1 (dark gray) fg gray1 (dark gray)
   0x77921, 0x77921, 0x77921, // 11 fg gray1 (dark gray) fg gray1 (dark gray)
   0x77921, 0x77921, 0x77921, // 00 bg gray1 (dark gray) bg gray1 (dark gray)
   0x77917, 0xC86BE, 0x77A00, // 10 fg mediumblue bg gray1 (dark gray)
   0x7A121, 0x2F921, 0xBFD21, // 01 bg gray1 (dark gray) fg mediumblue
   0x7A117, 0x906BE, 0xBFE00, // 11 fg mediumblue fg mediumblue
   0x77921, 0x77921, 0x77921, // 00 bg gray1 (dark gray) bg gray1 (dark gray)
   0x77A83, 0x77AC4, 0x77A00, // 10 fg lightblue bg gray1 (dark gray)
   0x9F121, 0x8ED21, 0xBFD21, // 01 bg gray1 (dark gray) fg lightblue
   0x9F283, 0x8EEC4, 0xBFE00, // 11 fg lightblue fg lightblue
   0x77921, 0x77921, 0x77921, // 00 bg gray1 (dark gray) bg gray1 (dark gray)
   0xC858F, 0xC85DC, 0x77900, // 10 fg brown bg gray1 (dark gray)
   0xE3D21, 0xF7121, 0x7FD21, // 01 bg gray1 (dark gray) fg brown
   0x5C18F, 0x48DDC, 0x7FD00, // 11 fg brown fg brown
   0x77921, 0x77921, 0x77921, // 00 bg gray1 (dark gray) bg gray1 (dark gray)
   0x77A03, 0x77A78, 0x77900, // 10 fg orange bg gray1 (dark gray)
   0xBF121, 0xA1D21, 0x7FD21, // 01 bg gray1 (dark gray) fg orange
   0xBF203, 0xA1E78, 0x7FD00, // 11 fg orange fg orange
   0x77921, 0x77921, 0x77921, // 00 bg gray1 (dark gray) bg gray1 (dark gray)
   0x77AC4, 0x77AC4, 0x77AC4, // 10 fg gray2 (light gray) bg gray1 (dark gray)
   0x8ED21, 0x8ED21, 0x8ED21, // 01 bg gray1 (dark gray) fg gray2 (light gray)
   0x8EEC4, 0x8EEC4, 0x8EEC4, // 11 fg gray2 (light gray) fg gray2 (light gray)
   0x77921, 0x77921, 0x77921, // 00 bg gray1 (dark gray) bg gray1 (dark gray)
   0xC86FC, 0x77963, 0x77971, // 10 fg pink bg gray1 (dark gray)
   0x3F121, 0x67121, 0x63921, // 01 bg gray1 (dark gray) fg pink
   0x80EFC, 0x67163, 0x63971, // 11 fg pink fg pink
   0x77921, 0x77921, 0x77921, // 00 bg gray1 (dark gray) bg gray1 (dark gray)
   0x779C0, 0xC861F, 0x77900, // 10 fg green bg gray1 (dark gray)
   0x4FD21, 0x07D21, 0x7FD21, // 01 bg gray1 (dark gray) fg green
   0x4FDC0, 0xB821F, 0x7FD00, // 11 fg green fg green
   0x77921, 0x77921, 0x77921, // 00 bg gray1 (dark gray) bg gray1 (dark gray)
   0xC86FF, 0xC86FF, 0x77900, // 10 fg yellow bg gray1 (dark gray)
   0x3FD21, 0x3FD21, 0x7FD21, // 01 bg gray1 (dark gray) fg yellow
   0x802FF, 0x802FF, 0x7FD00, // 11 fg yellow fg yellow
   0x77921, 0x77921, 0x77921, // 00 bg gray1 (dark gray) bg gray1 (dark gray)
   0x77A88, 0x77A01, 0x77961, // 10 fg aqua bg gray1 (dark gray)
   0x9DD21, 0xBF921, 0x67921, // 01 bg gray1 (dark gray) fg aqua
   0x9DE88, 0xBFA01, 0x67961, // 11 fg aqua fg aqua
   0x77921, 0x77921, 0x77921, // 00 bg gray1 (dark gray) bg gray1 (dark gray)
   0x77A00, 0x77A00, 0x77A00, // 10 fg white bg gray1 (dark gray)
   0xBFD21, 0xBFD21, 0xBFD21, // 01 bg gray1 (dark gray) fg white
   0xBFE00, 0xBFE00, 0xBFE00, // 11 fg white fg white
   0x7A117, 0x906BE, 0xBFE00, // 00 bg mediumblue bg mediumblue
   0x7A100, 0x2F900, 0xBFD00, // 10 fg black bg mediumblue
   0x7FD17, 0xC02BE, 0x7FE00, // 01 bg mediumblue fg black
   0x7FD00, 0x7FD00, 0x7FD00, // 11 fg black fg black
   0x7A117, 0x906BE, 0xBFE00, // 00 bg mediumblue bg mediumblue
   0x7A21C, 0x2FAA0, 0x0013E, // 10 fg magenta bg mediumblue
   0xB8D17, 0x282BE, 0xCFA00, // 01 bg mediumblue fg magenta
   0xB8E1C, 0x97EA0, 0x7053E, // 11 fg magenta fg magenta
   0x7A117, 0x906BE, 0xBFE00, // 00 bg mediumblue bg mediumblue
   0x7A104, 0x9050F, 0xBFD9C, // 10 fg darkblue bg mediumblue
   0x7ED17, 0x7C2BE, 0x58E00, // 01 bg mediumblue fg darkblue
   0x7ED04, 0x7C10F, 0x58D9C, // 11 fg darkblue fg darkblue
   0x7A117, 0x906BE, 0xBFE00, // 00 bg mediumblue bg mediumblue
   0x7A2E1, 0x2F9C1, 0x0015F, // 10 fg purple bg mediumblue
   0x87917, 0xF06BE, 0xD7E00, // 01 bg mediumblue fg purple
   0x87AE1, 0x4F9C1, 0x6815F, // 11 fg purple fg purple
   0x7A117, 0x906BE, 0xBFE00, // 00 bg mediumblue bg mediumblue
   0x7A1F4, 0x9057E, 0x001EF, // 10 fg darkgreen bg mediumblue
   0x42D17, 0x606BE, 0xFBE00, // 01 bg mediumblue fg darkgreen
   0x42DF4, 0x6057E, 0x441EF, // 11 fg darkgreen fg darkgreen
   0x7A117, 0x906BE, 0xBFE00, // 00 bg mediumblue bg mediumblue
   0x7A121, 0x2F921, 0xBFD21, // 10 fg gray1 (dark gray) bg mediumblue
   0x77917, 0xC86BE, 0x77A00, // 01 bg mediumblue fg gray1 (dark gray)
   0x77921, 0x77921, 0x77921, // 11 fg gray1 (dark gray) fg gray1 (dark gray)
   0x7A117, 0x906BE, 0xBFE00, // 00 bg mediumblue bg mediumblue
   0x7A117, 0x906BE, 0xBFE00, // 10 fg mediumblue bg mediumblue
   0x7A117, 0x906BE, 0xBFE00, // 01 bg mediumblue fg mediumblue
   0x7A117, 0x906BE, 0xBFE00, // 11 fg mediumblue fg mediumblue
   0x7A117, 0x906BE, 0xBFE00, // 00 bg mediumblue bg mediumblue
   0x7A283, 0x2FAC4, 0xBFE00, // 10 fg lightblue bg mediumblue
   0x9F117, 0x312BE, 0xBFE00, // 01 bg mediumblue fg lightblue
   0x9F283, 0x8EEC4, 0xBFE00, // 11 fg lightblue fg lightblue
   0x7A117, 0x906BE, 0xBFE00, // 00 bg mediumblue bg mediumblue
   0x7A18F, 0x905DC, 0xBFD00, // 10 fg brown bg mediumblue
   0x5C117, 0x48EBE, 0x7FE00, // 01 bg mediumblue fg brown
   0x5C18F, 0x48DDC, 0x7FD00, // 11 fg brown fg brown
   0x7A117, 0x906BE, 0xBFE00, // 00 bg mediumblue bg mediumblue
   0x7A203, 0x90678, 0xBFD00, // 10 fg orange bg mediumblue
   0xBF117, 0xA1EBE, 0x7FE00, // 01 bg mediumblue fg orange
   0xBF203, 0xA1E78, 0x7FD00, // 11 fg orange fg orange
   0x7A117, 0x906BE, 0xBFE00, // 00 bg mediumblue bg mediumblue
   0x7A2C4, 0x2FAC4, 0xBFEC4, // 10 fg gray2 (light gray) bg mediumblue
   0x8ED17, 0x312BE, 0x8EE00, // 01 bg mediumblue fg gray2 (light gray)
   0x8EEC4, 0x8EEC4, 0x8EEC4, // 11 fg gray2 (light gray) fg gray2 (light gray)
   0x7A117, 0x906BE, 0xBFE00, // 00 bg mediumblue bg mediumblue
   0x7A2FC, 0x90563, 0xBFD71, // 10 fg pink bg mediumblue
   0x80D17, 0x672BE, 0x63A00, // 01 bg mediumblue fg pink
   0x80EFC, 0x67163, 0x63971, // 11 fg pink fg pink
   0x7A117, 0x906BE, 0xBFE00, // 00 bg mediumblue bg mediumblue
   0x7A1C0, 0x9061F, 0xBFD00, // 10 fg green bg mediumblue
   0x4FD17, 0xB82BE, 0x7FE00, // 01 bg mediumblue fg green
   0x4FDC0, 0xB821F, 0x7FD00, // 11 fg green fg green
   0x7A117, 0x906BE, 0xBFE00, // 00 bg mediumblue bg mediumblue
   0x7A2FF, 0x906FF, 0xBFD00, // 10 fg yellow bg mediumblue
   0x80117, 0x802BE, 0x7FE00, // 01 bg mediumblue fg yellow
   0x802FF, 0x802FF, 0x7FD00, // 11 fg yellow fg yellow
   0x7A117, 0x906BE, 0xBFE00, // 00 bg mediumblue bg mediumblue
   0x7A288, 0x2FA01, 0xBFD61, // 10 fg aqua bg mediumblue
   0x9DD17, 0x006BE, 0x67A00, // 01 bg mediumblue fg aqua
   0x9DE88, 0xBFA01, 0x67961, // 11 fg aqua fg aqua
   0x7A117, 0x906BE, 0xBFE00, // 00 bg mediumblue bg mediumblue
   0x7A200, 0x2FA00, 0xBFE00, // 10 fg white bg mediumblue
   0xBFD17, 0x002BE, 0xBFE00, // 01 bg mediumblue fg white
   0xBFE00, 0xBFE00, 0xBFE00, // 11 fg white fg white
   0x9F283, 0x8EEC4, 0xBFE00, // 00 bg lightblue bg lightblue
   0x9F100, 0x8ED00, 0xBFD00, // 10 fg black bg lightblue
   0x7FE83, 0x7FEC4, 0x7FE00, // 01 bg lightblue fg black
   0x7FD00, 0x7FD00, 0x7FD00, // 11 fg black fg black
   0x9F283, 0x8EEC4, 0xBFE00, // 00 bg lightblue bg lightblue
   0x9F21C, 0x8EEA0, 0x0013E, // 10 fg magenta bg lightblue
   0xB8E83, 0x97EC4, 0xCFA00, // 01 bg lightblue fg magenta
   0xB8E1C, 0x97EA0, 0x7053E, // 11 fg magenta fg magenta
   0x9F283, 0x8EEC4, 0xBFE00, // 00 bg lightblue bg lightblue
   0x9F104, 0x8ED0F, 0xBFD9C, // 10 fg darkblue bg lightblue
   0x7EE83, 0x7C2C4, 0x58E00, // 01 bg lightblue fg darkblue
   0x7ED04, 0x7C10F, 0x58D9C, // 11 fg darkblue fg darkblue
   0x9F283, 0x8EEC4, 0xBFE00, // 00 bg lightblue bg lightblue
   0x9F2E1, 0x8EDC1, 0x0015F, // 10 fg purple bg lightblue
   0x87A83, 0x4FAC4, 0xD7E00, // 01 bg lightblue fg purple
   0x87AE1, 0x4F9C1, 0x6815F, // 11 fg purple fg purple
   0x9F283, 0x8EEC4, 0xBFE00, // 00 bg lightblue bg lightblue
   0x20DF4, 0x3117E, 0x001EF, // 10 fg darkgreen bg lightblue
   0xFD283, 0xDFAC4, 0xFBE00, // 01 bg lightblue fg darkgreen
   0x42DF4, 0x6057E, 0x441EF, // 11 fg darkgreen fg darkgreen
   0x9F283, 0x8EEC4, 0xBFE00, // 00 bg lightblue bg lightblue
   0x9F121, 0x8ED21, 0xBFD21, // 10 fg gray1 (dark gray) bg lightblue
   0x77A83, 0x77AC4, 0x77A00, // 01 bg lightblue fg gray1 (dark gray)
   0x77921, 0x77921, 0x77921, // 11 fg gray1 (dark gray) fg gray1 (dark gray)
   0x9F283, 0x8EEC4, 0xBFE00, // 00 bg lightblue bg lightblue
   0x9F117, 0x312BE, 0xBFE00, // 10 fg mediumblue bg lightblue
   0x7A283, 0x2FAC4, 0xBFE00, // 01 bg lightblue fg mediumblue
   0x7A117, 0x906BE, 0xBFE00, // 11 fg mediumblue fg mediumblue
   0x9F283, 0x8EEC4, 0xBFE00, // 00 bg lightblue bg lightblue
   0x9F283, 0x8EEC4, 0xBFE00, // 10 fg lightblue bg lightblue
   0x9F283, 0x8EEC4, 0xBFE00, // 01 bg lightblue fg lightblue
   0x9F283, 0x8EEC4, 0xBFE00, // 11 fg lightblue fg lightblue
   0x9F283, 0x8EEC4, 0xBFE00, // 00 bg lightblue bg lightblue
   0x20D8F, 0x311DC, 0xBFD00, // 10 fg brown bg lightblue
   0xE3E83, 0xF72C4, 0x7FE00, // 01 bg lightblue fg brown
   0x5C18F, 0x48DDC, 0x7FD00, // 11 fg brown fg brown
   0x9F283, 0x8EEC4, 0xBFE00, // 00 bg lightblue bg lightblue
   0x9F203, 0x8EE78, 0xBFD00, // 10 fg orange bg lightblue
   0xBF283, 0xA1EC4, 0x7FE00, // 01 bg lightblue fg orange
   0xBF203, 0xA1E78, 0x7FD00, // 11 fg orange fg orange
   0x9F283, 0x8EEC4, 0xBFE00, // 00 bg lightblue bg lightblue
   0x9F2C4, 0x8EEC4, 0xBFEC4, // 10 fg gray2 (light gray) bg lightblue
   0x8EE83, 0x8EEC4, 0x8EE00, // 01 bg lightblue fg gray2 (light gray)
   0x8EEC4, 0x8EEC4, 0x8EEC4, // 11 fg gray2 (light gray) fg gray2 (light gray)
   0x9F283, 0x8EEC4, 0xBFE00, // 00 bg lightblue bg lightblue
   0x20EFC, 0x8ED63, 0xBFD71, // 10 fg pink bg lightblue
   0x3F283, 0x672C4, 0x63A00, // 01 bg lightblue fg pink
   0x80EFC, 0x67163, 0x63971, // 11 fg pink fg pink
   0x9F283, 0x8EEC4, 0xBFE00, // 00 bg lightblue bg lightblue
   0x9F1C0, 0x3121F, 0xBFD00, // 10 fg green bg lightblue
   0x4FE83, 0x07EC4, 0x7FE00, // 01 bg lightblue fg green
   0x4FDC0, 0xB821F, 0x7FD00, // 11 fg green fg green
   0x9F283, 0x8EEC4, 0xBFE00, // 00 bg lightblue bg lightblue
   0x20EFF, 0x312FF, 0xBFD00, // 10 fg yellow bg lightblue
   0x3FE83, 0x3FEC4, 0x7FE00, // 01 bg lightblue fg yellow
   0x802FF, 0x802FF, 0x7FD00, // 11 fg yellow fg yellow
   0x9F283, 0x8EEC4, 0xBFE00, // 00 bg lightblue bg lightblue
   0x9F288, 0x8EE01, 0xBFD61, // 10 fg aqua bg lightblue
   0x9DE83, 0xBFAC4, 0x67A00, // 01 bg lightblue fg aqua
   0x9DE88, 0xBFA01, 0x67961, // 11 fg aqua fg aqua
   0x9F283, 0x8EEC4, 0xBFE00, // 00 bg lightblue bg lightblue
   0x9F200, 0x8EE00, 0xBFE00, // 10 fg white bg lightblue
   0xBFE83, 0xBFEC4, 0xBFE00, // 01 bg lightblue fg white
   0xBFE00, 0xBFE00, 0xBFE00, // 11 fg white fg white
   0x5C18F, 0x48DDC, 0x7FD00, // 00 bg brown bg brown
   0xE3D00, 0xF7100, 0x7FD00, // 10 fg black bg brown
   0xC018F, 0xC01DC, 0x7FD00, // 01 bg brown fg black
   0x7FD00, 0x7FD00, 0x7FD00, // 11 fg black fg black
   0x5C18F, 0x48DDC, 0x7FD00, // 00 bg brown bg brown
   0xE3E1C, 0xF72A0, 0xC013E, // 10 fg magenta bg brown
   0x0718F, 0x281DC, 0xCF900, // 01 bg brown fg magenta
   0xB8E1C, 0x97EA0, 0x7053E, // 11 fg magenta fg magenta
   0x5C18F, 0x48DDC, 0x7FD00, // 00 bg brown bg brown
   0xE3D04, 0x48D0F, 0x7FD9C, // 10 fg darkblue bg brown
   0xC118F, 0x7C1DC, 0x58D00, // 01 bg brown fg darkblue
   0x7ED04, 0x7C10F, 0x58D9C, // 11 fg darkblue fg darkblue
   0x5C18F, 0x48DDC, 0x7FD00, // 00 bg brown bg brown
   0x5C2E1, 0xF71C1, 0xC015F, // 10 fg purple bg brown
   0x8798F, 0xF05DC, 0xD7D00, // 01 bg brown fg purple
   0x87AE1, 0x4F9C1, 0x6815F, // 11 fg purple fg purple
   0x5C18F, 0x48DDC, 0x7FD00, // 00 bg brown bg brown
   0x5C1F4, 0x48D7E, 0xC01EF, // 10 fg darkgreen bg brown
   0x42D8F, 0x605DC, 0xFBD00, // 01 bg brown fg darkgreen
   0x42DF4, 0x6057E, 0x441EF, // 11 fg darkgreen fg darkgreen
   0x5C18F, 0x48DDC, 0x7FD00, // 00 bg brown bg brown
   0xE3D21, 0xF7121, 0x7FD21, // 10 fg gray1 (dark gray) bg brown
   0xC858F, 0xC85DC, 0x77900, // 01 bg brown fg gray1 (dark gray)
   0x77921, 0x77921, 0x77921, // 11 fg gray1 (dark gray) fg gray1 (dark gray)
   0x5C18F, 0x48DDC, 0x7FD00, // 00 bg brown bg brown
   0x5C117, 0x48EBE, 0x7FE00, // 10 fg mediumblue bg brown
   0x7A18F, 0x905DC, 0xBFD00, // 01 bg brown fg mediumblue
   0x7A117, 0x906BE, 0xBFE00, // 11 fg mediumblue fg mediumblue
   0x5C18F, 0x48DDC, 0x7FD00, // 00 bg brown bg brown
   0xE3E83, 0xF72C4, 0x7FE00, // 10 fg lightblue bg brown
   0x20D8F, 0x311DC, 0xBFD00, // 01 bg brown fg lightblue
   0x9F283, 0x8EEC4, 0xBFE00, // 11 fg lightblue fg lightblue
   0x5C18F, 0x48DDC, 0x7FD00, // 00 bg brown bg brown
   0x5C18F, 0x48DDC, 0x7FD00, // 10 fg brown bg brown
   0x5C18F, 0x48DDC, 0x7FD00, // 01 bg brown fg brown
   0x5C18F, 0x48DDC, 0x7FD00, // 11 fg brown fg brown
   0x5C18F, 0x48DDC, 0x7FD00, // 00 bg brown bg brown
   0xE3E03, 0x48E78, 0x7FD00, // 10 fg orange bg brown
   0x00D8F, 0xA1DDC, 0x7FD00, // 01 bg brown fg orange
   0xBF203, 0xA1E78, 0x7FD00, // 11 fg orange fg orange
   0x5C18F, 0x48DDC, 0x7FD00, // 00 bg brown bg brown
   0xE3EC4, 0xF72C4, 0x7FEC4, // 10 fg gray2 (light gray) bg brown
   0x3118F, 0x311DC, 0x8ED00, // 01 bg brown fg gray2 (light gray)
   0x8EEC4, 0x8EEC4, 0x8EEC4, // 11 fg gray2 (light gray) fg gray2 (light gray)
   0x5C18F, 0x48DDC, 0x7FD00, // 00 bg brown bg brown
   0x5C2FC, 0x48D63, 0x7FD71, // 10 fg pink bg brown
   0x80D8F, 0x671DC, 0x63900, // 01 bg brown fg pink
   0x80EFC, 0x67163, 0x63971, // 11 fg pink fg pink
   0x5C18F, 0x48DDC, 0x7FD00, // 00 bg brown bg brown
   0xE3DC0, 0x48E1F, 0x7FD00, // 10 fg green bg brown
   0xF018F, 0xB81DC, 0x7FD00, // 01 bg brown fg green
   0x4FDC0, 0xB821F, 0x7FD00, // 11 fg green fg green
   0x5C18F, 0x48DDC, 0x7FD00, // 00 bg brown bg brown
   0x5C2FF, 0x48EFF, 0x7FD00, // 10 fg yellow bg brown
   0x8018F, 0x801DC, 0x7FD00, // 01 bg brown fg yellow
   0x802FF, 0x802FF, 0x7FD00, // 11 fg yellow fg yellow
   0x5C18F, 0x48DDC, 0x7FD00, // 00 bg brown bg brown
   0xE3E88, 0xF7201, 0x7FD61, // 10 fg aqua bg brown
   0x2218F, 0x005DC, 0x67900, // 01 bg brown fg aqua
   0x9DE88, 0xBFA01, 0x67961, // 11 fg aqua fg aqua
   0x5C18F, 0x48DDC, 0x7FD00, // 00 bg brown bg brown
   0xE3E00, 0xF7200, 0x7FE00, // 10 fg white bg brown
   0x0018F, 0x001DC, 0xBFD00, // 01 bg brown fg white
   0xBFE00, 0xBFE00, 0xBFE00, // 11 fg white fg white
   0xBF203, 0xA1E78, 0x7FD00, // 00 bg orange bg orange
   0xBF100, 0xA1D00, 0x7FD00, // 10 fg black bg orange
   0x7FE03, 0x7FE78, 0x7FD00, // 01 bg orange fg black
   0x7FD00, 0x7FD00, 0x7FD00, // 11 fg black fg black
   0xBF203, 0xA1E78, 0x7FD00, // 00 bg orange bg orange
   0xBF21C, 0xA1EA0, 0xC013E, // 10 fg magenta bg orange
   0xB8E03, 0x97E78, 0xCF900, // 01 bg orange fg magenta
   0xB8E1C, 0x97EA0, 0x7053E, // 11 fg magenta fg magenta
   0xBF203, 0xA1E78, 0x7FD00, // 00 bg orange bg orange
   0xBF104, 0xA1D0F, 0x7FD9C, // 10 fg darkblue bg orange
   0x7EE03, 0x7C278, 0x58D00, // 01 bg orange fg darkblue
   0x7ED04, 0x7C10F, 0x58D9C, // 11 fg darkblue fg darkblue
   0xBF203, 0xA1E78, 0x7FD00, // 00 bg orange bg orange
   0xBF2E1, 0xA1DC1, 0xC015F, // 10 fg purple bg orange
   0x87A03, 0x4FA78, 0xD7D00, // 01 bg orange fg purple
   0x87AE1, 0x4F9C1, 0x6815F, // 11 fg purple fg purple
   0xBF203, 0xA1E78, 0x7FD00, // 00 bg orange bg orange
   0x00DF4, 0xA1D7E, 0xC01EF, // 10 fg darkgreen bg orange
   0xFD203, 0x60678, 0xFBD00, // 01 bg orange fg darkgreen
   0x42DF4, 0x6057E, 0x441EF, // 11 fg darkgreen fg darkgreen
   0xBF203, 0xA1E78, 0x7FD00, // 00 bg orange bg orange
   0xBF121, 0xA1D21, 0x7FD21, // 10 fg gray1 (dark gray) bg orange
   0x77A03, 0x77A78, 0x77900, // 01 bg orange fg gray1 (dark gray)
   0x77921, 0x77921, 0x77921, // 11 fg gray1 (dark gray) fg gray1 (dark gray)
   0xBF203, 0xA1E78, 0x7FD00, // 00 bg orange bg orange
   0xBF117, 0xA1EBE, 0x7FE00, // 10 fg mediumblue bg orange
   0x7A203, 0x90678, 0xBFD00, // 01 bg orange fg mediumblue
   0x7A117, 0x906BE, 0xBFE00, // 11 fg mediumblue fg mediumblue
   0xBF203, 0xA1E78, 0x7FD00, // 00 bg orange bg orange
   0xBF283, 0xA1EC4, 0x7FE00, // 10 fg lightblue bg orange
   0x9F203, 0x8EE78, 0xBFD00, // 01 bg orange fg lightblue
   0x9F283, 0x8EEC4, 0xBFE00, // 11 fg lightblue fg lightblue
   0xBF203, 0xA1E78, 0x7FD00, // 00 bg orange bg orange
   0x00D8F, 0xA1DDC, 0x7FD00, // 10 fg brown bg orange
   0xE3E03, 0x48E78, 0x7FD00, // 01 bg orange fg brown
   0x5C18F, 0x48DDC, 0x7FD00, // 11 fg brown fg brown
   0xBF203, 0xA1E78, 0x7FD00, // 00 bg orange bg orange
   0xBF203, 0xA1E78, 0x7FD00, // 10 fg orange bg orange
   0xBF203, 0xA1E78, 0x7FD00, // 01 bg orange fg orange
   0xBF203, 0xA1E78, 0x7FD00, // 11 fg orange fg orange
   0xBF203, 0xA1E78, 0x7FD00, // 00 bg orange bg orange
   0xBF2C4, 0xA1EC4, 0x7FEC4, // 10 fg gray2 (light gray) bg orange
   0x8EE03, 0x8EE78, 0x8ED00, // 01 bg orange fg gray2 (light gray)
   0x8EEC4, 0x8EEC4, 0x8EEC4, // 11 fg gray2 (light gray) fg gray2 (light gray)
   0xBF203, 0xA1E78, 0x7FD00, // 00 bg orange bg orange
   0x00EFC, 0xA1D63, 0x7FD71, // 10 fg pink bg orange
   0x3F203, 0x67278, 0x63900, // 01 bg orange fg pink
   0x80EFC, 0x67163, 0x63971, // 11 fg pink fg pink
   0xBF203, 0xA1E78, 0x7FD00, // 00 bg orange bg orange
   0xBF1C0, 0xA1E1F, 0x7FD00, // 10 fg green bg orange
   0x4FE03, 0xB8278, 0x7FD00, // 01 bg orange fg green
   0x4FDC0, 0xB821F, 0x7FD00, // 11 fg green fg green
   0xBF203, 0xA1E78, 0x7FD00, // 00 bg orange bg orange
   0x00EFF, 0xA1EFF, 0x7FD00, // 10 fg yellow bg orange
   0x3FE03, 0x80278, 0x7FD00, // 01 bg orange fg yellow
   0x802FF, 0x802FF, 0x7FD00, // 11 fg yellow fg yellow
   0xBF203, 0xA1E78, 0x7FD00, // 00 bg orange bg orange
   0xBF288, 0xA1E01, 0x7FD61, // 10 fg aqua bg orange
   0x9DE03, 0xBFA78, 0x67900, // 01 bg orange fg aqua
   0x9DE88, 0xBFA01, 0x67961, // 11 fg aqua fg aqua
   0xBF203, 0xA1E78, 0x7FD00, // 00 bg orange bg orange
   0xBF200, 0xA1E00, 0x7FE00, // 10 fg white bg orange
   0xBFE03, 0xBFE78, 0xBFD00, // 01 bg orange fg white
   0xBFE00, 0xBFE00, 0xBFE00, // 11 fg white fg white
   0x8EEC4, 0x8EEC4, 0x8EEC4, // 00 bg gray2 (light gray) bg gray2 (light gray)
   0x8ED00, 0x8ED00, 0x8ED00, // 10 fg black bg gray2 (light gray)
   0x7FEC4, 0x7FEC4, 0x7FEC4, // 01 bg gray2 (light gray) fg black
   0x7FD00, 0x7FD00, 0x7FD00, // 11 fg black fg black
   0x8EEC4, 0x8EEC4, 0x8EEC4, // 00 bg gray2 (light gray) bg gray2 (light gray)
   0x8EE1C, 0x8EEA0, 0x3113E, // 10 fg magenta bg gray2 (light gray)
   0xB8EC4, 0x97EC4, 0xCFAC4, // 01 bg gray2 (light gray) fg magenta
   0xB8E1C, 0x97EA0, 0x7053E, // 11 fg magenta fg magenta
   0x8EEC4, 0x8EEC4, 0x8EEC4, // 00 bg gray2 (light gray) bg gray2 (light gray)
   0x8ED04, 0x8ED0F, 0x8ED9C, // 10 fg darkblue bg gray2 (light gray)
   0x7EEC4, 0x7C2C4, 0x58EC4, // 01 bg gray2 (light gray) fg darkblue
   0x7ED04, 0x7C10F, 0x58D9C, // 11 fg darkblue fg darkblue
   0x8EEC4, 0x8EEC4, 0x8EEC4, // 00 bg gray2 (light gray) bg gray2 (light gray)
   0x8EEE1, 0x8EDC1, 0x3115F, // 10 fg purple bg gray2 (light gray)
   0x87AC4, 0x4FAC4, 0xD7EC4, // 01 bg gray2 (light gray) fg purple
   0x87AE1, 0x4F9C1, 0x6815F, // 11 fg purple fg purple
   0x8EEC4, 0x8EEC4, 0x8EEC4, // 00 bg gray2 (light gray) bg gray2 (light gray)
   0x311F4, 0x3117E, 0x311EF, // 10 fg darkgreen bg gray2 (light gray)
   0xFD2C4, 0xDFAC4, 0xFBEC4, // 01 bg gray2 (light gray) fg darkgreen
   0x42DF4, 0x6057E, 0x441EF, // 11 fg darkgreen fg darkgreen
   0x8EEC4, 0x8EEC4, 0x8EEC4, // 00 bg gray2 (light gray) bg gray2 (light gray)
   0x8ED21, 0x8ED21, 0x8ED21, // 10 fg gray1 (dark gray) bg gray2 (light gray)
   0x77AC4, 0x77AC4, 0x77AC4, // 01 bg gray2 (light gray) fg gray1 (dark gray)
   0x77921, 0x77921, 0x77921, // 11 fg gray1 (dark gray) fg gray1 (dark gray)
   0x8EEC4, 0x8EEC4, 0x8EEC4, // 00 bg gray2 (light gray) bg gray2 (light gray)
   0x8ED17, 0x312BE, 0x8EE00, // 10 fg mediumblue bg gray2 (light gray)
   0x7A2C4, 0x2FAC4, 0xBFEC4, // 01 bg gray2 (light gray) fg mediumblue
   0x7A117, 0x906BE, 0xBFE00, // 11 fg mediumblue fg mediumblue
   0x8EEC4, 0x8EEC4, 0x8EEC4, // 00 bg gray2 (light gray) bg gray2 (light gray)
   0x8EE83, 0x8EEC4, 0x8EE00, // 10 fg lightblue bg gray2 (light gray)
   0x9F2C4, 0x8EEC4, 0xBFEC4, // 01 bg gray2 (light gray) fg lightblue
   0x9F283, 0x8EEC4, 0xBFE00, // 11 fg lightblue fg lightblue
   0x8EEC4, 0x8EEC4, 0x8EEC4, // 00 bg gray2 (light gray) bg gray2 (light gray)
   0x3118F, 0x311DC, 0x8ED00, // 10 fg brown bg gray2 (light gray)
   0xE3EC4, 0xF72C4, 0x7FEC4, // 01 bg gray2 (light gray) fg brown
   0x5C18F, 0x48DDC, 0x7FD00, // 11 fg brown fg brown
   0x8EEC4, 0x8EEC4, 0x8EEC4, // 00 bg gray2 (light gray) bg gray2 (light gray)
   0x8EE03, 0x8EE78, 0x8ED00, // 10 fg orange bg gray2 (light gray)
   0xBF2C4, 0xA1EC4, 0x7FEC4, // 01 bg gray2 (light gray) fg orange
   0xBF203, 0xA1E78, 0x7FD00, // 11 fg orange fg orange
   0x8EEC4, 0x8EEC4, 0x8EEC4, // 00 bg gray2 (light gray) bg gray2 (light gray)
   0x8EEC4, 0x8EEC4, 0x8EEC4, // 10 fg gray2 (light gray) bg gray2 (light gray)
   0x8EEC4, 0x8EEC4, 0x8EEC4, // 01 bg gray2 (light gray) fg gray2 (light gray)
   0x8EEC4, 0x8EEC4, 0x8EEC4, // 11 fg gray2 (light gray) fg gray2 (light gray)
   0x8EEC4, 0x8EEC4, 0x8EEC4, // 00 bg gray2 (light gray) bg gray2 (light gray)
   0x312FC, 0x8ED63, 0x8ED71, // 10 fg pink bg gray2 (light gray)
   0x3F2C4, 0x672C4, 0x63AC4, // 01 bg gray2 (light gray) fg pink
   0x80EFC, 0x67163, 0x63971, // 11 fg pink fg pink
   0x8EEC4, 0x8EEC4, 0x8EEC4, // 00 bg gray2 (light gray) bg gray2 (light gray)
   0x8EDC0, 0x3121F, 0x8ED00, // 10 fg green bg gray2 (light gray)
   0x4FEC4, 0x07EC4, 0x7FEC4, // 01 bg gray2 (light gray) fg green
   0x4FDC0, 0xB821F, 0x7FD00, // 11 fg green fg green
   0x8EEC4, 0x8EEC4, 0x8EEC4, // 00 bg gray2 (light gray) bg gray2 (light gray)
   0x312FF, 0x312FF, 0x8ED00, // 10 fg yellow bg gray2 (light gray)
   0x3FEC4, 0x3FEC4, 0x7FEC4, // 01 bg gray2 (light gray) fg yellow
   0x802FF, 0x802FF, 0x7FD00, // 11 fg yellow fg yellow
   0x8EEC4, 0x8EEC4, 0x8EEC4, // 00 bg gray2 (light gray) bg gray2 (light gray)
   0x8EE88, 0x8EE01, 0x8ED61, // 10 fg aqua bg gray2 (light gray)
   0x9DEC4, 0xBFAC4, 0x67AC4, // 01 bg gray2 (light gray) fg aqua
   0x9DE88, 0xBFA01, 0x67961, // 11 fg aqua fg aqua
   0x8EEC4, 0x8EEC4, 0x8EEC4, // 00 bg gray2 (light gray) bg gray2 (light gray)
   0x8EE00, 0x8EE00, 0x8EE00, // 10 fg white bg gray2 (light gray)
   0xBFEC4, 0xBFEC4, 0xBFEC4, // 01 bg gray2 (light gray) fg white
   0xBFE00, 0xBFE00, 0xBFE00, // 11 fg white fg white
   0x80EFC, 0x67163, 0x63971, // 00 bg pink bg pink
   0x3F100, 0x67100, 0x63900, // 10 fg black bg pink
   0xC02FC, 0x7FD63, 0x7FD71, // 01 bg pink fg black
   0x7FD00, 0x7FD00, 0x7FD00, // 11 fg black fg black
   0x80EFC, 0x67163, 0x63971, // 00 bg pink bg pink
   0x3F21C, 0x672A0, 0x6393E, // 10 fg magenta bg pink
   0x072FC, 0x97D63, 0x70571, // 01 bg pink fg magenta
   0xB8E1C, 0x97EA0, 0x7053E, // 11 fg magenta fg magenta
   0x80EFC, 0x67163, 0x63971, // 00 bg pink bg pink
   0x3F104, 0x6710F, 0x6399C, // 10 fg darkblue bg pink
   0xC12FC, 0x7C163, 0x58D71, // 01 bg pink fg darkblue
   0x7ED04, 0x7C10F, 0x58D9C, // 11 fg darkblue fg darkblue
   0x80EFC, 0x67163, 0x63971, // 00 bg pink bg pink
   0x80EE1, 0x671C1, 0x6395F, // 10 fg purple bg pink
   0x87AFC, 0x4F963, 0x68171, // 01 bg pink fg purple
   0x87AE1, 0x4F9C1, 0x6815F, // 11 fg purple fg purple
   0x80EFC, 0x67163, 0x63971, // 00 bg pink bg pink
   0x80DF4, 0x6717E, 0x639EF, // 10 fg darkgreen bg pink
   0x42EFC, 0x60563, 0x44171, // 01 bg pink fg darkgreen
   0x42DF4, 0x6057E, 0x441EF, // 11 fg darkgreen fg darkgreen
   0x80EFC, 0x67163, 0x63971, // 00 bg pink bg pink
   0x3F121, 0x67121, 0x63921, // 10 fg gray1 (dark gray) bg pink
   0xC86FC, 0x77963, 0x77971, // 01 bg pink fg gray1 (dark gray)
   0x77921, 0x77921, 0x77921, // 11 fg gray1 (dark gray) fg gray1 (dark gray)
   0x80EFC, 0x67163, 0x63971, // 00 bg pink bg pink
   0x80D17, 0x672BE, 0x63A00, // 10 fg mediumblue bg pink
   0x7A2FC, 0x90563, 0xBFD71, // 01 bg pink fg mediumblue
   0x7A117, 0x906BE, 0xBFE00, // 11 fg mediumblue fg mediumblue
   0x80EFC, 0x67163, 0x63971, // 00 bg pink bg pink
   0x3F283, 0x672C4, 0x63A00, // 10 fg lightblue bg pink
   0x20EFC, 0x8ED63, 0xBFD71, // 01 bg pink fg lightblue
   0x9F283, 0x8EEC4, 0xBFE00, // 11 fg lightblue fg lightblue
   0x80EFC, 0x67163, 0x63971, // 00 bg pink bg pink
   0x80D8F, 0x671DC, 0x63900, // 10 fg brown bg pink
   0x5C2FC, 0x48D63, 0x7FD71, // 01 bg pink fg brown
   0x5C18F, 0x48DDC, 0x7FD00, // 11 fg brown fg brown
   0x80EFC, 0x67163, 0x63971, // 00 bg pink bg pink
   0x3F203, 0x67278, 0x63900, // 10 fg orange bg pink
   0x00EFC, 0xA1D63, 0x7FD71, // 01 bg pink fg orange
   0xBF203, 0xA1E78, 0x7FD00, // 11 fg orange fg orange
   0x80EFC, 0x67163, 0x63971, // 00 bg pink bg pink
   0x3F2C4, 0x672C4, 0x63AC4, // 10 fg gray2 (light gray) bg pink
   0x312FC, 0x8ED63, 0x8ED71, // 01 bg pink fg gray2 (light gray)
   0x8EEC4, 0x8EEC4, 0x8EEC4, // 11 fg gray2 (light gray) fg gray2 (light gray)
   0x80EFC, 0x67163, 0x63971, // 00 bg pink bg pink
   0x80EFC, 0x67163, 0x63971, // 10 fg pink bg pink
   0x80EFC, 0x67163, 0x63971, // 01 bg pink fg pink
   0x80EFC, 0x67163, 0x63971, // 11 fg pink fg pink
   0x80EFC, 0x67163, 0x63971, // 00 bg pink bg pink
   0x3F1C0, 0x6721F, 0x63900, // 10 fg green bg pink
   0xF02FC, 0xB8163, 0x7FD71, // 01 bg pink fg green
   0x4FDC0, 0xB821F, 0x7FD00, // 11 fg green fg green
   0x80EFC, 0x67163, 0x63971, // 00 bg pink bg pink
   0x80EFF, 0x672FF, 0x63900, // 10 fg yellow bg pink
   0x802FC, 0x80163, 0x7FD71, // 01 bg pink fg yellow
   0x802FF, 0x802FF, 0x7FD00, // 11 fg yellow fg yellow
   0x80EFC, 0x67163, 0x63971, // 00 bg pink bg pink
   0x3F288, 0x67201, 0x63961, // 10 fg aqua bg pink
   0x222FC, 0xBF963, 0x67971, // 01 bg pink fg aqua
   0x9DE88, 0xBFA01, 0x67961, // 11 fg aqua fg aqua
   0x80EFC, 0x67163, 0x63971, // 00 bg pink bg pink
   0x3F200, 0x67200, 0x63A00, // 10 fg white bg pink
   0x002FC, 0xBFD63, 0xBFD71, // 01 bg pink fg white
   0xBFE00, 0xBFE00, 0xBFE00, // 11 fg white fg white
   0x4FDC0, 0xB821F, 0x7FD00, // 00 bg green bg green
   0x4FD00, 0x07D00, 0x7FD00, // 10 fg black bg green
   0x7FDC0, 0xC021F, 0x7FD00, // 01 bg green fg black
   0x7FD00, 0x7FD00, 0x7FD00, // 11 fg black fg black
   0x4FDC0, 0xB821F, 0x7FD00, // 00 bg green bg green
   0x4FE1C, 0x07EA0, 0xC013E, // 10 fg magenta bg green
   0xB8DC0, 0x2821F, 0xCF900, // 01 bg green fg magenta
   0xB8E1C, 0x97EA0, 0x7053E, // 11 fg magenta fg magenta
   0x4FDC0, 0xB821F, 0x7FD00, // 00 bg green bg green
   0x4FD04, 0xB810F, 0x7FD9C, // 10 fg darkblue bg green
   0x7EDC0, 0x7C21F, 0x58D00, // 01 bg green fg darkblue
   0x7ED04, 0x7C10F, 0x58D9C, // 11 fg darkblue fg darkblue
   0x4FDC0, 0xB821F, 0x7FD00, // 00 bg green bg green
   0x4FEE1, 0x07DC1, 0xC015F, // 10 fg purple bg green
   0x879C0, 0xF061F, 0xD7D00, // 01 bg green fg purple
   0x87AE1, 0x4F9C1, 0x6815F, // 11 fg purple fg purple
   0x4FDC0, 0xB821F, 0x7FD00, // 00 bg green bg green
   0xF01F4, 0xB817E, 0xC01EF, // 10 fg darkgreen bg green
   0xFD1C0, 0x6061F, 0xFBD00, // 01 bg green fg darkgreen
   0x42DF4, 0x6057E, 0x441EF, // 11 fg darkgreen fg darkgreen
   0x4FDC0, 0xB821F, 0x7FD00, // 00 bg green bg green
   0x4FD21, 0x07D21, 0x7FD21, // 10 fg gray1 (dark gray) bg green
   0x779C0, 0xC861F, 0x77900, // 01 bg green fg gray1 (dark gray)
   0x77921, 0x77921, 0x77921, // 11 fg gray1 (dark gray) fg gray1 (dark gray)
   0x4FDC0, 0xB821F, 0x7FD00, // 00 bg green bg green
   0x4FD17, 0xB82BE, 0x7FE00, // 10 fg mediumblue bg green
   0x7A1C0, 0x9061F, 0xBFD00, // 01 bg green fg mediumblue
   0x7A117, 0x906BE, 0xBFE00, // 11 fg mediumblue fg mediumblue
   0x4FDC0, 0xB821F, 0x7FD00, // 00 bg green bg green
   0x4FE83, 0x07EC4, 0x7FE00, // 10 fg lightblue bg green
   0x9F1C0, 0x3121F, 0xBFD00, // 01 bg green fg lightblue
   0x9F283, 0x8EEC4, 0xBFE00, // 11 fg lightblue fg lightblue
   0x4FDC0, 0xB821F, 0x7FD00, // 00 bg green bg green
   0xF018F, 0xB81DC, 0x7FD00, // 10 fg brown bg green
   0xE3DC0, 0x48E1F, 0x7FD00, // 01 bg green fg brown
   0x5C18F, 0x48DDC, 0x7FD00, // 11 fg brown fg brown
   0x4FDC0, 0xB821F, 0x7FD00, // 00 bg green bg green
   0x4FE03, 0xB8278, 0x7FD00, // 10 fg orange bg green
   0xBF1C0, 0xA1E1F, 0x7FD00, // 01 bg green fg orange
   0xBF203, 0xA1E78, 0x7FD00, // 11 fg orange fg orange
   0x4FDC0, 0xB821F, 0x7FD00, // 00 bg green bg green
   0x4FEC4, 0x07EC4, 0x7FEC4, // 10 fg gray2 (light gray) bg green
   0x8EDC0, 0x3121F, 0x8ED00, // 01 bg green fg gray2 (light gray)
   0x8EEC4, 0x8EEC4, 0x8EEC4, // 11 fg gray2 (light gray) fg gray2 (light gray)
   0x4FDC0, 0xB821F, 0x7FD00, // 00 bg green bg green
   0xF02FC, 0xB8163, 0x7FD71, // 10 fg pink bg green
   0x3F1C0, 0x6721F, 0x63900, // 01 bg green fg pink
   0x80EFC, 0x67163, 0x63971, // 11 fg pink fg pink
   0x4FDC0, 0xB821F, 0x7FD00, // 00 bg green bg green
   0x4FDC0, 0xB821F, 0x7FD00, // 10 fg green bg green
   0x4FDC0, 0xB821F, 0x7FD00, // 01 bg green fg green
   0x4FDC0, 0xB821F, 0x7FD00, // 11 fg green fg green
   0x4FDC0, 0xB821F, 0x7FD00, // 00 bg green bg green
   0xF02FF, 0xB82FF, 0x7FD00, // 10 fg yellow bg green
   0x3FDC0, 0x8021F, 0x7FD00, // 01 bg green fg yellow
   0x802FF, 0x802FF, 0x7FD00, // 11 fg yellow fg yellow
   0x4FDC0, 0xB821F, 0x7FD00, // 00 bg green bg green
   0x4FE88, 0x07E01, 0x7FD61, // 10 fg aqua bg green
   0x9DDC0, 0x0061F, 0x67900, // 01 bg green fg aqua
   0x9DE88, 0xBFA01, 0x67961, // 11 fg aqua fg aqua
   0x4FDC0, 0xB821F, 0x7FD00, // 00 bg green bg green
   0x4FE00, 0x07E00, 0x7FE00, // 10 fg white bg green
   0xBFDC0, 0x0021F, 0xBFD00, // 01 bg green fg white
   0xBFE00, 0xBFE00, 0xBFE00, // 11 fg white fg white
   0x802FF, 0x802FF, 0x7FD00, // 00 bg yellow bg yellow
   0x3FD00, 0x3FD00, 0x7FD00, // 10 fg black bg yellow
   0xC02FF, 0xC02FF, 0x7FD00, // 01 bg yellow fg black
   0x7FD00, 0x7FD00, 0x7FD00, // 11 fg black fg black
   0x802FF, 0x802FF, 0x7FD00, // 00 bg yellow bg yellow
   0x3FE1C, 0x3FEA0, 0xC013E, // 10 fg magenta bg yellow
   0x072FF, 0x282FF, 0xCF900, // 01 bg yellow fg magenta
   0xB8E1C, 0x97EA0, 0x7053E, // 11 fg magenta fg magenta
   0x802FF, 0x802FF, 0x7FD00, // 00 bg yellow bg yellow
   0x3FD04, 0x8010F, 0x7FD9C, // 10 fg darkblue bg yellow
   0xC12FF, 0x7C2FF, 0x58D00, // 01 bg yellow fg darkblue
   0x7ED04, 0x7C10F, 0x58D9C, // 11 fg darkblue fg darkblue
   0x802FF, 0x802FF, 0x7FD00, // 00 bg yellow bg yellow
   0x802E1, 0x3FDC1, 0xC015F, // 10 fg purple bg yellow
   0x87AFF, 0xF06FF, 0xD7D00, // 01 bg yellow fg purple
   0x87AE1, 0x4F9C1, 0x6815F, // 11 fg purple fg purple
   0x802FF, 0x802FF, 0x7FD00, // 00 bg yellow bg yellow
   0x801F4, 0x8017E, 0xC01EF, // 10 fg darkgreen bg yellow
   0x42EFF, 0x606FF, 0xFBD00, // 01 bg yellow fg darkgreen
   0x42DF4, 0x6057E, 0x441EF, // 11 fg darkgreen fg darkgreen
   0x802FF, 0x802FF, 0x7FD00, // 00 bg yellow bg yellow
   0x3FD21, 0x3FD21, 0x7FD21, // 10 fg gray1 (dark gray) bg yellow
   0xC86FF, 0xC86FF, 0x77900, // 01 bg yellow fg gray1 (dark gray)
   0x77921, 0x77921, 0x77921, // 11 fg gray1 (dark gray) fg gray1 (dark gray)
   0x802FF, 0x802FF, 0x7FD00, // 00 bg yellow bg yellow
   0x80117, 0x802BE, 0x7FE00, // 10 fg mediumblue bg yellow
   0x7A2FF, 0x906FF, 0xBFD00, // 01 bg yellow fg mediumblue
   0x7A117, 0x906BE, 0xBFE00, // 11 fg mediumblue fg mediumblue
   0x802FF, 0x802FF, 0x7FD00, // 00 bg yellow bg yellow
   0x3FE83, 0x3FEC4, 0x7FE00, // 10 fg lightblue bg yellow
   0x20EFF, 0x312FF, 0xBFD00, // 01 bg yellow fg lightblue
   0x9F283, 0x8EEC4, 0xBFE00, // 11 fg lightblue fg lightblue
   0x802FF, 0x802FF, 0x7FD00, // 00 bg yellow bg yellow
   0x8018F, 0x801DC, 0x7FD00, // 10 fg brown bg yellow
   0x5C2FF, 0x48EFF, 0x7FD00, // 01 bg yellow fg brown
   0x5C18F, 0x48DDC, 0x7FD00, // 11 fg brown fg brown
   0x802FF, 0x802FF, 0x7FD00, // 00 bg yellow bg yellow
   0x3FE03, 0x80278, 0x7FD00, // 10 fg orange bg yellow
   0x00EFF, 0xA1EFF, 0x7FD00, // 01 bg yellow fg orange
   0xBF203, 0xA1E78, 0x7FD00, // 11 fg orange fg orange
   0x802FF, 0x802FF, 0x7FD00, // 00 bg yellow bg yellow
   0x3FEC4, 0x3FEC4, 0x7FEC4, // 10 fg gray2 (light gray) bg yellow
   0x312FF, 0x312FF, 0x8ED00, // 01 bg yellow fg gray2 (light gray)
   0x8EEC4, 0x8EEC4, 0x8EEC4, // 11 fg gray2 (light gray) fg gray2 (light gray)
   0x802FF, 0x802FF, 0x7FD00, // 00 bg yellow bg yellow
   0x802FC, 0x80163, 0x7FD71, // 10 fg pink bg yellow
   0x80EFF, 0x672FF, 0x63900, // 01 bg yellow fg pink
   0x80EFC, 0x67163, 0x63971, // 11 fg pink fg pink
   0x802FF, 0x802FF, 0x7FD00, // 00 bg yellow bg yellow
   0x3FDC0, 0x8021F, 0x7FD00, // 10 fg green bg yellow
   0xF02FF, 0xB82FF, 0x7FD00, // 01 bg yellow fg green
   0x4FDC0, 0xB821F, 0x7FD00, // 11 fg green fg green
   0x802FF, 0x802FF, 0x7FD00, // 00 bg yellow bg yellow
   0x802FF, 0x802FF, 0x7FD00, // 10 fg yellow bg yellow
   0x802FF, 0x802FF, 0x7FD00, // 01 bg yellow fg yellow
   0x802FF, 0x802FF, 0x7FD00, // 11 fg yellow fg yellow
   0x802FF, 0x802FF, 0x7FD00, // 00 bg yellow bg yellow
   0x3FE88, 0x3FE01, 0x7FD61, // 10 fg aqua bg yellow
   0x222FF, 0x006FF, 0x67900, // 01 bg yellow fg aqua
   0x9DE88, 0xBFA01, 0x67961, // 11 fg aqua fg aqua
   0x802FF, 0x802FF, 0x7FD00, // 00 bg yellow bg yellow
   0x3FE00, 0x3FE00, 0x7FE00, // 10 fg white bg yellow
   0x002FF, 0x002FF, 0xBFD00, // 01 bg yellow fg white
   0xBFE00, 0xBFE00, 0xBFE00, // 11 fg white fg white
   0x9DE88, 0xBFA01, 0x67961, // 00 bg aqua bg aqua
   0x9DD00, 0xBF900, 0x67900, // 10 fg black bg aqua
   0x7FE88, 0x7FE01, 0x7FD61, // 01 bg aqua fg black
   0x7FD00, 0x7FD00, 0x7FD00, // 11 fg black fg black
   0x9DE88, 0xBFA01, 0x67961, // 00 bg aqua bg aqua
   0x9DE1C, 0xBFAA0, 0xD853E, // 10 fg magenta bg aqua
   0xB8E88, 0x97E01, 0xCF961, // 01 bg aqua fg magenta
   0xB8E1C, 0x97EA0, 0x7053E, // 11 fg magenta fg magenta
   0x9DE88, 0xBFA01, 0x67961, // 00 bg aqua bg aqua
   0x9DD04, 0xBF90F, 0x6799C, // 10 fg darkblue bg aqua
   0x7EE88, 0x7C201, 0x58D61, // 01 bg aqua fg darkblue
   0x7ED04, 0x7C10F, 0x58D9C, // 11 fg darkblue fg darkblue
   0x9DE88, 0xBFA01, 0x67961, // 00 bg aqua bg aqua
   0x9DEE1, 0xBF9C1, 0xD855F, // 10 fg purple bg aqua
   0x87A88, 0x4FA01, 0xD7D61, // 01 bg aqua fg purple
   0x87AE1, 0x4F9C1, 0x6815F, // 11 fg purple fg purple
   0x9DE88, 0xBFA01, 0x67961, // 00 bg aqua bg aqua
   0x221F4, 0x0057E, 0xD85EF, // 10 fg darkgreen bg aqua
   0xFD288, 0xDFA01, 0xFBD61, // 01 bg aqua fg darkgreen
   0x42DF4, 0x6057E, 0x441EF, // 11 fg darkgreen fg darkgreen
   0x9DE88, 0xBFA01, 0x67961, // 00 bg aqua bg aqua
   0x9DD21, 0xBF921, 0x67921, // 10 fg gray1 (dark gray) bg aqua
   0x77A88, 0x77A01, 0x77961, // 01 bg aqua fg gray1 (dark gray)
   0x77921, 0x77921, 0x77921, // 11 fg gray1 (dark gray) fg gray1 (dark gray)
   0x9DE88, 0xBFA01, 0x67961, // 00 bg aqua bg aqua
   0x9DD17, 0x006BE, 0x67A00, // 10 fg mediumblue bg aqua
   0x7A288, 0x2FA01, 0xBFD61, // 01 bg aqua fg mediumblue
   0x7A117, 0x906BE, 0xBFE00, // 11 fg mediumblue fg mediumblue
   0x9DE88, 0xBFA01, 0x67961, // 00 bg aqua bg aqua
   0x9DE83, 0xBFAC4, 0x67A00, // 10 fg lightblue bg aqua
   0x9F288, 0x8EE01, 0xBFD61, // 01 bg aqua fg lightblue
   0x9F283, 0x8EEC4, 0xBFE00, // 11 fg lightblue fg lightblue
   0x9DE88, 0xBFA01, 0x67961, // 00 bg aqua bg aqua
   0x2218F, 0x005DC, 0x67900, // 10 fg brown bg aqua
   0xE3E88, 0xF7201, 0x7FD61, // 01 bg aqua fg brown
   0x5C18F, 0x48DDC, 0x7FD00, // 11 fg brown fg brown
   0x9DE88, 0xBFA01, 0x67961, // 00 bg aqua bg aqua
   0x9DE03, 0xBFA78, 0x67900, // 10 fg orange bg aqua
   0xBF288, 0xA1E01, 0x7FD61, // 01 bg aqua fg orange
   0xBF203, 0xA1E78, 0x7FD00, // 11 fg orange fg orange
   0x9DE88, 0xBFA01, 0x67961, // 00 bg aqua bg aqua
   0x9DEC4, 0xBFAC4, 0x67AC4, // 10 fg gray2 (light gray) bg aqua
   0x8EE88, 0x8EE01, 0x8ED61, // 01 bg aqua fg gray2 (light gray)
   0x8EEC4, 0x8EEC4, 0x8EEC4, // 11 fg gray2 (light gray) fg gray2 (light gray)
   0x9DE88, 0xBFA01, 0x67961, // 00 bg aqua bg aqua
   0x222FC, 0xBF963, 0x67971, // 10 fg pink bg aqua
   0x3F288, 0x67201, 0x63961, // 01 bg aqua fg pink
   0x80EFC, 0x67163, 0x63971, // 11 fg pink fg pink
   0x9DE88, 0xBFA01, 0x67961, // 00 bg aqua bg aqua
   0x9DDC0, 0x0061F, 0x67900, // 10 fg green bg aqua
   0x4FE88, 0x07E01, 0x7FD61, // 01 bg aqua fg green
   0x4FDC0, 0xB821F, 0x7FD00, // 11 fg green fg green
   0x9DE88, 0xBFA01, 0x67961, // 00 bg aqua bg aqua
   0x222FF, 0x006FF, 0x67900, // 10 fg yellow bg aqua
   0x3FE88, 0x3FE01, 0x7FD61, // 01 bg aqua fg yellow
   0x802FF, 0x802FF, 0x7FD00, // 11 fg yellow fg yellow
   0x9DE88, 0xBFA01, 0x67961, // 00 bg aqua bg aqua
   0x9DE88, 0xBFA01, 0x67961, // 10 fg aqua bg aqua
   0x9DE88, 0xBFA01, 0x67961, // 01 bg aqua fg aqua
   0x9DE88, 0xBFA01, 0x67961, // 11 fg aqua fg aqua
   0x9DE88, 0xBFA01, 0x67961, // 00 bg aqua bg aqua
   0x9DE00, 0xBFA00, 0x67A00, // 10 fg white bg aqua
   0xBFE88, 0xBFE01, 0xBFD61, // 01 bg aqua fg white
   0xBFE00, 0xBFE00, 0xBFE00, // 11 fg white fg white
   0xBFE00, 0xBFE00, 0xBFE00, // 00 bg white bg white
   0xBFD00, 0xBFD00, 0xBFD00, // 10 fg black bg white
   0x7FE00, 0x7FE00, 0x7FE00, // 01 bg white fg black
   0x7FD00, 0x7FD00, 0x7FD00, // 11 fg black fg black
   0xBFE00, 0xBFE00, 0xBFE00, // 00 bg white bg white
   0xBFE1C, 0xBFEA0, 0x0013E, // 10 fg magenta bg white
   0xB8E00, 0x97E00, 0xCFA00, // 01 bg white fg magenta
   0xB8E1C, 0x97EA0, 0x7053E, // 11 fg magenta fg magenta
   0xBFE00, 0xBFE00, 0xBFE00, // 00 bg white bg white
   0xBFD04, 0xBFD0F, 0xBFD9C, // 10 fg darkblue bg white
   0x7EE00, 0x7C200, 0x58E00, // 01 bg white fg darkblue
   0x7ED04, 0x7C10F, 0x58D9C, // 11 fg darkblue fg darkblue
   0xBFE00, 0xBFE00, 0xBFE00, // 00 bg white bg white
   0xBFEE1, 0xBFDC1, 0x0015F, // 10 fg purple bg white
   0x87A00, 0x4FA00, 0xD7E00, // 01 bg white fg purple
   0x87AE1, 0x4F9C1, 0x6815F, // 11 fg purple fg purple
   0xBFE00, 0xBFE00, 0xBFE00, // 00 bg white bg white
   0x001F4, 0x0017E, 0x001EF, // 10 fg darkgreen bg white
   0xFD200, 0xDFA00, 0xFBE00, // 01 bg white fg darkgreen
   0x42DF4, 0x6057E, 0x441EF, // 11 fg darkgreen fg darkgreen
   0xBFE00, 0xBFE00, 0xBFE00, // 00 bg white bg white
   0xBFD21, 0xBFD21, 0xBFD21, // 10 fg gray1 (dark gray) bg white
   0x77A00, 0x77A00, 0x77A00, // 01 bg white fg gray1 (dark gray)
   0x77921, 0x77921, 0x77921, // 11 fg gray1 (dark gray) fg gray1 (dark gray)
   0xBFE00, 0xBFE00, 0xBFE00, // 00 bg white bg white
   0xBFD17, 0x002BE, 0xBFE00, // 10 fg mediumblue bg white
   0x7A200, 0x2FA00, 0xBFE00, // 01 bg white fg mediumblue
   0x7A117, 0x906BE, 0xBFE00, // 11 fg mediumblue fg mediumblue
   0xBFE00, 0xBFE00, 0xBFE00, // 00 bg white bg white
   0xBFE83, 0xBFEC4, 0xBFE00, // 10 fg lightblue bg white
   0x9F200, 0x8EE00, 0xBFE00, // 01 bg white fg lightblue
   0x9F283, 0x8EEC4, 0xBFE00, // 11 fg lightblue fg lightblue
   0xBFE00, 0xBFE00, 0xBFE00, // 00 bg white bg white
   0x0018F, 0x001DC, 0xBFD00, // 10 fg brown bg white
   0xE3E00, 0xF7200, 0x7FE00, // 01 bg white fg brown
   0x5C18F, 0x48DDC, 0x7FD00, // 11 fg brown fg brown
   0xBFE00, 0xBFE00, 0xBFE00, // 00 bg white bg white
   0xBFE03, 0xBFE78, 0xBFD00, // 10 fg orange bg white
   0xBF200, 0xA1E00, 0x7FE00, // 01 bg white fg orange
   0xBF203, 0xA1E78, 0x7FD00, // 11 fg orange fg orange
   0xBFE00, 0xBFE00, 0xBFE00, // 00 bg white bg white
   0xBFEC4, 0xBFEC4, 0xBFEC4, // 10 fg gray2 (light gray) bg white
   0x8EE00, 0x8EE00, 0x8EE00, // 01 bg white fg gray2 (light gray)
   0x8EEC4, 0x8EEC4, 0x8EEC4, // 11 fg gray2 (light gray) fg gray2 (light gray)
   0xBFE00, 0xBFE00, 0xBFE00, // 00 bg white bg white
   0x002FC, 0xBFD63, 0xBFD71, // 10 fg pink bg white
   0x3F200, 0x67200, 0x63A00, // 01 bg white fg pink
   0x80EFC, 0x67163, 0x63971, // 11 fg pink fg pink
   0xBFE00, 0xBFE00, 0xBFE00, // 00 bg white bg white
   0xBFDC0, 0x0021F, 0xBFD00, // 10 fg green bg white
   0x4FE00, 0x07E00, 0x7FE00, // 01 bg white fg green
   0x4FDC0, 0xB821F, 0x7FD00, // 11 fg green fg green
   0xBFE00, 0xBFE00, 0xBFE00, // 00 bg white bg white
   0x002FF, 0x002FF, 0xBFD00, // 10 fg yellow bg white
   0x3FE00, 0x3FE00, 0x7FE00, // 01 bg white fg yellow
   0x802FF, 0x802FF, 0x7FD00, // 11 fg yellow fg yellow
   0xBFE00, 0xBFE00, 0xBFE00, // 00 bg white bg white
   0xBFE88, 0xBFE01, 0xBFD61, // 10 fg aqua bg white
   0x9DE00, 0xBFA00, 0x67A00, // 01 bg white fg aqua
   0x9DE88, 0xBFA01, 0x67961, // 11 fg aqua fg aqua
   0xBFE00, 0xBFE00, 0xBFE00, // 00 bg white bg white
   0xBFE00, 0xBFE00, 0xBFE00, // 10 fg white bg white
   0xBFE00, 0xBFE00, 0xBFE00, // 01 bg white fg white
   0xBFE00, 0xBFE00, 0xBFE00, // 11 fg white fg white
};