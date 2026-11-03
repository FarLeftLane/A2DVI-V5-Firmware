/*
MIT License

Copyright (c) 2025 Michael Neil (Far Left Lane)

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

#include "hardware/dma.h"
#include "hardware/irq.h"

#include "applebus/buffers.h"
#include "config/config.h"

#include "render.h"


uint32_t s_shr_320_to_tmds[16] = {
    0x7FD00,      //  i = 0 value = 0
    0x7C10F,      //  i = 1 value = 17
    0x7851E,      //  i = 2 value = 34
    0x7B911,      //  i = 3 value = 51
    0x70D3C,      //  i = 4 value = 68
    0x73133,      //  i = 5 value = 85
    0xA2277,      //  i = 6 value = 102
    0xA1E78,      //  i = 7 value = 119
    0x61D78,      //  i = 8 value = 136
    0x62177,      //  i = 9 value = 153
    0xB3233,      //  i = 10 value = 170
    0xB0E3C,      //  i = 11 value = 187
    0xBBA11,      //  i = 12 value = 204
    0xB861E,      //  i = 13 value = 221
    0xBC20F,      //  i = 14 value = 238
    0xBFE00       //  i = 15 value = 255
};

//  Comnputed each pixel pair for a 4-bit color value
uint32_t s_shr_640_to_tmds[256] = {
    0x7FD00,      //  i = 0 pixel1 = 0 pixel2 = 0 value1 = 0 value2 = 0
    0x7C100,      //  i = 1 pixel1 = 0 pixel2 = 1 value1 = 0 value2 = 17
    0x78500,      //  i = 2 pixel1 = 0 pixel2 = 2 value1 = 0 value2 = 34
    0x7B900,      //  i = 3 pixel1 = 0 pixel2 = 3 value1 = 0 value2 = 51
    0x70D00,      //  i = 4 pixel1 = 0 pixel2 = 4 value1 = 0 value2 = 68
    0x73100,      //  i = 5 pixel1 = 0 pixel2 = 5 value1 = 0 value2 = 85
    0x1DD00,      //  i = 6 pixel1 = 0 pixel2 = 6 value1 = 0 value2 = 102
    0xA1D00,      //  i = 7 pixel1 = 0 pixel2 = 7 value1 = 0 value2 = 119
    0x61D00,      //  i = 8 pixel1 = 0 pixel2 = 8 value1 = 0 value2 = 136
    0xDDD00,      //  i = 9 pixel1 = 0 pixel2 = 9 value1 = 0 value2 = 153
    0xB3100,      //  i = 10 pixel1 = 0 pixel2 = 10 value1 = 0 value2 = 170
    0xB0D00,      //  i = 11 pixel1 = 0 pixel2 = 11 value1 = 0 value2 = 187
    0xBB900,      //  i = 12 pixel1 = 0 pixel2 = 12 value1 = 0 value2 = 204
    0xB8500,      //  i = 13 pixel1 = 0 pixel2 = 13 value1 = 0 value2 = 221
    0xBC100,      //  i = 14 pixel1 = 0 pixel2 = 14 value1 = 0 value2 = 238
    0xBFD00,      //  i = 15 pixel1 = 0 pixel2 = 15 value1 = 0 value2 = 255
    0x7FD0F,      //  i = 16 pixel1 = 1 pixel2 = 0 value1 = 17 value2 = 0
    0x7C10F,      //  i = 17 pixel1 = 1 pixel2 = 1 value1 = 17 value2 = 17
    0x7850F,      //  i = 18 pixel1 = 1 pixel2 = 2 value1 = 17 value2 = 34
    0x7B90F,      //  i = 19 pixel1 = 1 pixel2 = 3 value1 = 17 value2 = 51
    0x70D0F,      //  i = 20 pixel1 = 1 pixel2 = 4 value1 = 17 value2 = 68
    0x7310F,      //  i = 21 pixel1 = 1 pixel2 = 5 value1 = 17 value2 = 85
    0xA210F,      //  i = 22 pixel1 = 1 pixel2 = 6 value1 = 17 value2 = 102
    0xA1D0F,      //  i = 23 pixel1 = 1 pixel2 = 7 value1 = 17 value2 = 119
    0x61D0F,      //  i = 24 pixel1 = 1 pixel2 = 8 value1 = 17 value2 = 136
    0x6210F,      //  i = 25 pixel1 = 1 pixel2 = 9 value1 = 17 value2 = 153
    0xB310F,      //  i = 26 pixel1 = 1 pixel2 = 10 value1 = 17 value2 = 170
    0xB0D0F,      //  i = 27 pixel1 = 1 pixel2 = 11 value1 = 17 value2 = 187
    0xBB90F,      //  i = 28 pixel1 = 1 pixel2 = 12 value1 = 17 value2 = 204
    0xB850F,      //  i = 29 pixel1 = 1 pixel2 = 13 value1 = 17 value2 = 221
    0xBC10F,      //  i = 30 pixel1 = 1 pixel2 = 14 value1 = 17 value2 = 238
    0xBFD0F,      //  i = 31 pixel1 = 1 pixel2 = 15 value1 = 17 value2 = 255
    0x7FD1E,      //  i = 32 pixel1 = 2 pixel2 = 0 value1 = 34 value2 = 0
    0x7C11E,      //  i = 33 pixel1 = 2 pixel2 = 1 value1 = 34 value2 = 17
    0x7851E,      //  i = 34 pixel1 = 2 pixel2 = 2 value1 = 34 value2 = 34
    0x7B91E,      //  i = 35 pixel1 = 2 pixel2 = 3 value1 = 34 value2 = 51
    0x70D1E,      //  i = 36 pixel1 = 2 pixel2 = 4 value1 = 34 value2 = 68
    0x7311E,      //  i = 37 pixel1 = 2 pixel2 = 5 value1 = 34 value2 = 85
    0xA211E,      //  i = 38 pixel1 = 2 pixel2 = 6 value1 = 34 value2 = 102
    0xA1D1E,      //  i = 39 pixel1 = 2 pixel2 = 7 value1 = 34 value2 = 119
    0x61D1E,      //  i = 40 pixel1 = 2 pixel2 = 8 value1 = 34 value2 = 136
    0x6211E,      //  i = 41 pixel1 = 2 pixel2 = 9 value1 = 34 value2 = 153
    0xB311E,      //  i = 42 pixel1 = 2 pixel2 = 10 value1 = 34 value2 = 170
    0xB0D1E,      //  i = 43 pixel1 = 2 pixel2 = 11 value1 = 34 value2 = 187
    0xBB91E,      //  i = 44 pixel1 = 2 pixel2 = 12 value1 = 34 value2 = 204
    0xB851E,      //  i = 45 pixel1 = 2 pixel2 = 13 value1 = 34 value2 = 221
    0xBC11E,      //  i = 46 pixel1 = 2 pixel2 = 14 value1 = 34 value2 = 238
    0xBFD1E,      //  i = 47 pixel1 = 2 pixel2 = 15 value1 = 34 value2 = 255
    0x7FD11,      //  i = 48 pixel1 = 3 pixel2 = 0 value1 = 51 value2 = 0
    0x7C111,      //  i = 49 pixel1 = 3 pixel2 = 1 value1 = 51 value2 = 17
    0x78511,      //  i = 50 pixel1 = 3 pixel2 = 2 value1 = 51 value2 = 34
    0x7B911,      //  i = 51 pixel1 = 3 pixel2 = 3 value1 = 51 value2 = 51
    0x70D11,      //  i = 52 pixel1 = 3 pixel2 = 4 value1 = 51 value2 = 68
    0x73111,      //  i = 53 pixel1 = 3 pixel2 = 5 value1 = 51 value2 = 85
    0x1DD11,      //  i = 54 pixel1 = 3 pixel2 = 6 value1 = 51 value2 = 102
    0xA1D11,      //  i = 55 pixel1 = 3 pixel2 = 7 value1 = 51 value2 = 119
    0x61D11,      //  i = 56 pixel1 = 3 pixel2 = 8 value1 = 51 value2 = 136
    0xDDD11,      //  i = 57 pixel1 = 3 pixel2 = 9 value1 = 51 value2 = 153
    0xB3111,      //  i = 58 pixel1 = 3 pixel2 = 10 value1 = 51 value2 = 170
    0xB0D11,      //  i = 59 pixel1 = 3 pixel2 = 11 value1 = 51 value2 = 187
    0xBB911,      //  i = 60 pixel1 = 3 pixel2 = 12 value1 = 51 value2 = 204
    0xB8511,      //  i = 61 pixel1 = 3 pixel2 = 13 value1 = 51 value2 = 221
    0xBC111,      //  i = 62 pixel1 = 3 pixel2 = 14 value1 = 51 value2 = 238
    0xBFD11,      //  i = 63 pixel1 = 3 pixel2 = 15 value1 = 51 value2 = 255
    0x7FD3C,      //  i = 64 pixel1 = 4 pixel2 = 0 value1 = 68 value2 = 0
    0x7C13C,      //  i = 65 pixel1 = 4 pixel2 = 1 value1 = 68 value2 = 17
    0x7853C,      //  i = 66 pixel1 = 4 pixel2 = 2 value1 = 68 value2 = 34
    0x7B93C,      //  i = 67 pixel1 = 4 pixel2 = 3 value1 = 68 value2 = 51
    0x70D3C,      //  i = 68 pixel1 = 4 pixel2 = 4 value1 = 68 value2 = 68
    0x7313C,      //  i = 69 pixel1 = 4 pixel2 = 5 value1 = 68 value2 = 85
    0xA213C,      //  i = 70 pixel1 = 4 pixel2 = 6 value1 = 68 value2 = 102
    0xA1D3C,      //  i = 71 pixel1 = 4 pixel2 = 7 value1 = 68 value2 = 119
    0x61D3C,      //  i = 72 pixel1 = 4 pixel2 = 8 value1 = 68 value2 = 136
    0x6213C,      //  i = 73 pixel1 = 4 pixel2 = 9 value1 = 68 value2 = 153
    0xB313C,      //  i = 74 pixel1 = 4 pixel2 = 10 value1 = 68 value2 = 170
    0xB0D3C,      //  i = 75 pixel1 = 4 pixel2 = 11 value1 = 68 value2 = 187
    0xBB93C,      //  i = 76 pixel1 = 4 pixel2 = 12 value1 = 68 value2 = 204
    0xB853C,      //  i = 77 pixel1 = 4 pixel2 = 13 value1 = 68 value2 = 221
    0xBC13C,      //  i = 78 pixel1 = 4 pixel2 = 14 value1 = 68 value2 = 238
    0xBFD3C,      //  i = 79 pixel1 = 4 pixel2 = 15 value1 = 68 value2 = 255
    0x7FD33,      //  i = 80 pixel1 = 5 pixel2 = 0 value1 = 85 value2 = 0
    0x7C133,      //  i = 81 pixel1 = 5 pixel2 = 1 value1 = 85 value2 = 17
    0x78533,      //  i = 82 pixel1 = 5 pixel2 = 2 value1 = 85 value2 = 34
    0x7B933,      //  i = 83 pixel1 = 5 pixel2 = 3 value1 = 85 value2 = 51
    0x70D33,      //  i = 84 pixel1 = 5 pixel2 = 4 value1 = 85 value2 = 68
    0x73133,      //  i = 85 pixel1 = 5 pixel2 = 5 value1 = 85 value2 = 85
    0xA2133,      //  i = 86 pixel1 = 5 pixel2 = 6 value1 = 85 value2 = 102
    0xA1D33,      //  i = 87 pixel1 = 5 pixel2 = 7 value1 = 85 value2 = 119
    0x61D33,      //  i = 88 pixel1 = 5 pixel2 = 8 value1 = 85 value2 = 136
    0x62133,      //  i = 89 pixel1 = 5 pixel2 = 9 value1 = 85 value2 = 153
    0xB3133,      //  i = 90 pixel1 = 5 pixel2 = 10 value1 = 85 value2 = 170
    0xB0D33,      //  i = 91 pixel1 = 5 pixel2 = 11 value1 = 85 value2 = 187
    0xBB933,      //  i = 92 pixel1 = 5 pixel2 = 12 value1 = 85 value2 = 204
    0xB8533,      //  i = 93 pixel1 = 5 pixel2 = 13 value1 = 85 value2 = 221
    0xBC133,      //  i = 94 pixel1 = 5 pixel2 = 14 value1 = 85 value2 = 238
    0xBFD33,      //  i = 95 pixel1 = 5 pixel2 = 15 value1 = 85 value2 = 255
    0xC0277,      //  i = 96 pixel1 = 6 pixel2 = 0 value1 = 102 value2 = 0
    0x7C277,      //  i = 97 pixel1 = 6 pixel2 = 1 value1 = 102 value2 = 17
    0x78677,      //  i = 98 pixel1 = 6 pixel2 = 2 value1 = 102 value2 = 34
    0xC4677,      //  i = 99 pixel1 = 6 pixel2 = 3 value1 = 102 value2 = 51
    0x70E77,      //  i = 100 pixel1 = 6 pixel2 = 4 value1 = 102 value2 = 68
    0x73277,      //  i = 101 pixel1 = 6 pixel2 = 5 value1 = 102 value2 = 85
    0xA2277,      //  i = 102 pixel1 = 6 pixel2 = 6 value1 = 102 value2 = 102
    0xA1E77,      //  i = 103 pixel1 = 6 pixel2 = 7 value1 = 102 value2 = 119
    0x61E77,      //  i = 104 pixel1 = 6 pixel2 = 8 value1 = 102 value2 = 136
    0x62277,      //  i = 105 pixel1 = 6 pixel2 = 9 value1 = 102 value2 = 153
    0xB3277,      //  i = 106 pixel1 = 6 pixel2 = 10 value1 = 102 value2 = 170
    0xB0E77,      //  i = 107 pixel1 = 6 pixel2 = 11 value1 = 102 value2 = 187
    0x04677,      //  i = 108 pixel1 = 6 pixel2 = 12 value1 = 102 value2 = 204
    0xB8677,      //  i = 109 pixel1 = 6 pixel2 = 13 value1 = 102 value2 = 221
    0xBC277,      //  i = 110 pixel1 = 6 pixel2 = 14 value1 = 102 value2 = 238
    0x00277,      //  i = 111 pixel1 = 6 pixel2 = 15 value1 = 102 value2 = 255
    0x7FE78,      //  i = 112 pixel1 = 7 pixel2 = 0 value1 = 119 value2 = 0
    0x7C278,      //  i = 113 pixel1 = 7 pixel2 = 1 value1 = 119 value2 = 17
    0x78678,      //  i = 114 pixel1 = 7 pixel2 = 2 value1 = 119 value2 = 34
    0x7BA78,      //  i = 115 pixel1 = 7 pixel2 = 3 value1 = 119 value2 = 51
    0x70E78,      //  i = 116 pixel1 = 7 pixel2 = 4 value1 = 119 value2 = 68
    0x73278,      //  i = 117 pixel1 = 7 pixel2 = 5 value1 = 119 value2 = 85
    0xA2278,      //  i = 118 pixel1 = 7 pixel2 = 6 value1 = 119 value2 = 102
    0xA1E78,      //  i = 119 pixel1 = 7 pixel2 = 7 value1 = 119 value2 = 119
    0x61E78,      //  i = 120 pixel1 = 7 pixel2 = 8 value1 = 119 value2 = 136
    0x62278,      //  i = 121 pixel1 = 7 pixel2 = 9 value1 = 119 value2 = 153
    0xB3278,      //  i = 122 pixel1 = 7 pixel2 = 10 value1 = 119 value2 = 170
    0xB0E78,      //  i = 123 pixel1 = 7 pixel2 = 11 value1 = 119 value2 = 187
    0xBBA78,      //  i = 124 pixel1 = 7 pixel2 = 12 value1 = 119 value2 = 204
    0xB8678,      //  i = 125 pixel1 = 7 pixel2 = 13 value1 = 119 value2 = 221
    0xBC278,      //  i = 126 pixel1 = 7 pixel2 = 14 value1 = 119 value2 = 238
    0xBFE78,      //  i = 127 pixel1 = 7 pixel2 = 15 value1 = 119 value2 = 255
    0x7FD78,      //  i = 128 pixel1 = 8 pixel2 = 0 value1 = 136 value2 = 0
    0x7C178,      //  i = 129 pixel1 = 8 pixel2 = 1 value1 = 136 value2 = 17
    0x78578,      //  i = 130 pixel1 = 8 pixel2 = 2 value1 = 136 value2 = 34
    0x7B978,      //  i = 131 pixel1 = 8 pixel2 = 3 value1 = 136 value2 = 51
    0x70D78,      //  i = 132 pixel1 = 8 pixel2 = 4 value1 = 136 value2 = 68
    0x73178,      //  i = 133 pixel1 = 8 pixel2 = 5 value1 = 136 value2 = 85
    0xA2178,      //  i = 134 pixel1 = 8 pixel2 = 6 value1 = 136 value2 = 102
    0xA1D78,      //  i = 135 pixel1 = 8 pixel2 = 7 value1 = 136 value2 = 119
    0x61D78,      //  i = 136 pixel1 = 8 pixel2 = 8 value1 = 136 value2 = 136
    0x62178,      //  i = 137 pixel1 = 8 pixel2 = 9 value1 = 136 value2 = 153
    0xB3178,      //  i = 138 pixel1 = 8 pixel2 = 10 value1 = 136 value2 = 170
    0xB0D78,      //  i = 139 pixel1 = 8 pixel2 = 11 value1 = 136 value2 = 187
    0xBB978,      //  i = 140 pixel1 = 8 pixel2 = 12 value1 = 136 value2 = 204
    0xB8578,      //  i = 141 pixel1 = 8 pixel2 = 13 value1 = 136 value2 = 221
    0xBC178,      //  i = 142 pixel1 = 8 pixel2 = 14 value1 = 136 value2 = 238
    0xBFD78,      //  i = 143 pixel1 = 8 pixel2 = 15 value1 = 136 value2 = 255
    0xC0177,      //  i = 144 pixel1 = 9 pixel2 = 0 value1 = 153 value2 = 0
    0x7C177,      //  i = 145 pixel1 = 9 pixel2 = 1 value1 = 153 value2 = 17
    0x78577,      //  i = 146 pixel1 = 9 pixel2 = 2 value1 = 153 value2 = 34
    0xC4577,      //  i = 147 pixel1 = 9 pixel2 = 3 value1 = 153 value2 = 51
    0x70D77,      //  i = 148 pixel1 = 9 pixel2 = 4 value1 = 153 value2 = 68
    0x73177,      //  i = 149 pixel1 = 9 pixel2 = 5 value1 = 153 value2 = 85
    0xA2177,      //  i = 150 pixel1 = 9 pixel2 = 6 value1 = 153 value2 = 102
    0xA1D77,      //  i = 151 pixel1 = 9 pixel2 = 7 value1 = 153 value2 = 119
    0x61D77,      //  i = 152 pixel1 = 9 pixel2 = 8 value1 = 153 value2 = 136
    0x62177,      //  i = 153 pixel1 = 9 pixel2 = 9 value1 = 153 value2 = 153
    0xB3177,      //  i = 154 pixel1 = 9 pixel2 = 10 value1 = 153 value2 = 170
    0xB0D77,      //  i = 155 pixel1 = 9 pixel2 = 11 value1 = 153 value2 = 187
    0x04577,      //  i = 156 pixel1 = 9 pixel2 = 12 value1 = 153 value2 = 204
    0xB8577,      //  i = 157 pixel1 = 9 pixel2 = 13 value1 = 153 value2 = 221
    0xBC177,      //  i = 158 pixel1 = 9 pixel2 = 14 value1 = 153 value2 = 238
    0x00177,      //  i = 159 pixel1 = 9 pixel2 = 15 value1 = 153 value2 = 255
    0x7FE33,      //  i = 160 pixel1 = 10 pixel2 = 0 value1 = 170 value2 = 0
    0x7C233,      //  i = 161 pixel1 = 10 pixel2 = 1 value1 = 170 value2 = 17
    0x78633,      //  i = 162 pixel1 = 10 pixel2 = 2 value1 = 170 value2 = 34
    0x7BA33,      //  i = 163 pixel1 = 10 pixel2 = 3 value1 = 170 value2 = 51
    0x70E33,      //  i = 164 pixel1 = 10 pixel2 = 4 value1 = 170 value2 = 68
    0x73233,      //  i = 165 pixel1 = 10 pixel2 = 5 value1 = 170 value2 = 85
    0xA2233,      //  i = 166 pixel1 = 10 pixel2 = 6 value1 = 170 value2 = 102
    0xA1E33,      //  i = 167 pixel1 = 10 pixel2 = 7 value1 = 170 value2 = 119
    0x61E33,      //  i = 168 pixel1 = 10 pixel2 = 8 value1 = 170 value2 = 136
    0x62233,      //  i = 169 pixel1 = 10 pixel2 = 9 value1 = 170 value2 = 153
    0xB3233,      //  i = 170 pixel1 = 10 pixel2 = 10 value1 = 170 value2 = 170
    0xB0E33,      //  i = 171 pixel1 = 10 pixel2 = 11 value1 = 170 value2 = 187
    0xBBA33,      //  i = 172 pixel1 = 10 pixel2 = 12 value1 = 170 value2 = 204
    0xB8633,      //  i = 173 pixel1 = 10 pixel2 = 13 value1 = 170 value2 = 221
    0xBC233,      //  i = 174 pixel1 = 10 pixel2 = 14 value1 = 170 value2 = 238
    0xBFE33,      //  i = 175 pixel1 = 10 pixel2 = 15 value1 = 170 value2 = 255
    0x7FE3C,      //  i = 176 pixel1 = 11 pixel2 = 0 value1 = 187 value2 = 0
    0x7C23C,      //  i = 177 pixel1 = 11 pixel2 = 1 value1 = 187 value2 = 17
    0x7863C,      //  i = 178 pixel1 = 11 pixel2 = 2 value1 = 187 value2 = 34
    0x7BA3C,      //  i = 179 pixel1 = 11 pixel2 = 3 value1 = 187 value2 = 51
    0x70E3C,      //  i = 180 pixel1 = 11 pixel2 = 4 value1 = 187 value2 = 68
    0x7323C,      //  i = 181 pixel1 = 11 pixel2 = 5 value1 = 187 value2 = 85
    0xA223C,      //  i = 182 pixel1 = 11 pixel2 = 6 value1 = 187 value2 = 102
    0xA1E3C,      //  i = 183 pixel1 = 11 pixel2 = 7 value1 = 187 value2 = 119
    0x61E3C,      //  i = 184 pixel1 = 11 pixel2 = 8 value1 = 187 value2 = 136
    0x6223C,      //  i = 185 pixel1 = 11 pixel2 = 9 value1 = 187 value2 = 153
    0xB323C,      //  i = 186 pixel1 = 11 pixel2 = 10 value1 = 187 value2 = 170
    0xB0E3C,      //  i = 187 pixel1 = 11 pixel2 = 11 value1 = 187 value2 = 187
    0xBBA3C,      //  i = 188 pixel1 = 11 pixel2 = 12 value1 = 187 value2 = 204
    0xB863C,      //  i = 189 pixel1 = 11 pixel2 = 13 value1 = 187 value2 = 221
    0xBC23C,      //  i = 190 pixel1 = 11 pixel2 = 14 value1 = 187 value2 = 238
    0xBFE3C,      //  i = 191 pixel1 = 11 pixel2 = 15 value1 = 187 value2 = 255
    0x7FE11,      //  i = 192 pixel1 = 12 pixel2 = 0 value1 = 204 value2 = 0
    0x7C211,      //  i = 193 pixel1 = 12 pixel2 = 1 value1 = 204 value2 = 17
    0x78611,      //  i = 194 pixel1 = 12 pixel2 = 2 value1 = 204 value2 = 34
    0x7BA11,      //  i = 195 pixel1 = 12 pixel2 = 3 value1 = 204 value2 = 51
    0x70E11,      //  i = 196 pixel1 = 12 pixel2 = 4 value1 = 204 value2 = 68
    0x73211,      //  i = 197 pixel1 = 12 pixel2 = 5 value1 = 204 value2 = 85
    0x1DE11,      //  i = 198 pixel1 = 12 pixel2 = 6 value1 = 204 value2 = 102
    0xA1E11,      //  i = 199 pixel1 = 12 pixel2 = 7 value1 = 204 value2 = 119
    0x61E11,      //  i = 200 pixel1 = 12 pixel2 = 8 value1 = 204 value2 = 136
    0xDDE11,      //  i = 201 pixel1 = 12 pixel2 = 9 value1 = 204 value2 = 153
    0xB3211,      //  i = 202 pixel1 = 12 pixel2 = 10 value1 = 204 value2 = 170
    0xB0E11,      //  i = 203 pixel1 = 12 pixel2 = 11 value1 = 204 value2 = 187
    0xBBA11,      //  i = 204 pixel1 = 12 pixel2 = 12 value1 = 204 value2 = 204
    0xB8611,      //  i = 205 pixel1 = 12 pixel2 = 13 value1 = 204 value2 = 221
    0xBC211,      //  i = 206 pixel1 = 12 pixel2 = 14 value1 = 204 value2 = 238
    0xBFE11,      //  i = 207 pixel1 = 12 pixel2 = 15 value1 = 204 value2 = 255
    0x7FE1E,      //  i = 208 pixel1 = 13 pixel2 = 0 value1 = 221 value2 = 0
    0x7C21E,      //  i = 209 pixel1 = 13 pixel2 = 1 value1 = 221 value2 = 17
    0x7861E,      //  i = 210 pixel1 = 13 pixel2 = 2 value1 = 221 value2 = 34
    0x7BA1E,      //  i = 211 pixel1 = 13 pixel2 = 3 value1 = 221 value2 = 51
    0x70E1E,      //  i = 212 pixel1 = 13 pixel2 = 4 value1 = 221 value2 = 68
    0x7321E,      //  i = 213 pixel1 = 13 pixel2 = 5 value1 = 221 value2 = 85
    0xA221E,      //  i = 214 pixel1 = 13 pixel2 = 6 value1 = 221 value2 = 102
    0xA1E1E,      //  i = 215 pixel1 = 13 pixel2 = 7 value1 = 221 value2 = 119
    0x61E1E,      //  i = 216 pixel1 = 13 pixel2 = 8 value1 = 221 value2 = 136
    0x6221E,      //  i = 217 pixel1 = 13 pixel2 = 9 value1 = 221 value2 = 153
    0xB321E,      //  i = 218 pixel1 = 13 pixel2 = 10 value1 = 221 value2 = 170
    0xB0E1E,      //  i = 219 pixel1 = 13 pixel2 = 11 value1 = 221 value2 = 187
    0xBBA1E,      //  i = 220 pixel1 = 13 pixel2 = 12 value1 = 221 value2 = 204
    0xB861E,      //  i = 221 pixel1 = 13 pixel2 = 13 value1 = 221 value2 = 221
    0xBC21E,      //  i = 222 pixel1 = 13 pixel2 = 14 value1 = 221 value2 = 238
    0xBFE1E,      //  i = 223 pixel1 = 13 pixel2 = 15 value1 = 221 value2 = 255
    0x7FE0F,      //  i = 224 pixel1 = 14 pixel2 = 0 value1 = 238 value2 = 0
    0x7C20F,      //  i = 225 pixel1 = 14 pixel2 = 1 value1 = 238 value2 = 17
    0x7860F,      //  i = 226 pixel1 = 14 pixel2 = 2 value1 = 238 value2 = 34
    0x7BA0F,      //  i = 227 pixel1 = 14 pixel2 = 3 value1 = 238 value2 = 51
    0x70E0F,      //  i = 228 pixel1 = 14 pixel2 = 4 value1 = 238 value2 = 68
    0x7320F,      //  i = 229 pixel1 = 14 pixel2 = 5 value1 = 238 value2 = 85
    0xA220F,      //  i = 230 pixel1 = 14 pixel2 = 6 value1 = 238 value2 = 102
    0xA1E0F,      //  i = 231 pixel1 = 14 pixel2 = 7 value1 = 238 value2 = 119
    0x61E0F,      //  i = 232 pixel1 = 14 pixel2 = 8 value1 = 238 value2 = 136
    0x6220F,      //  i = 233 pixel1 = 14 pixel2 = 9 value1 = 238 value2 = 153
    0xB320F,      //  i = 234 pixel1 = 14 pixel2 = 10 value1 = 238 value2 = 170
    0xB0E0F,      //  i = 235 pixel1 = 14 pixel2 = 11 value1 = 238 value2 = 187
    0xBBA0F,      //  i = 236 pixel1 = 14 pixel2 = 12 value1 = 238 value2 = 204
    0xB860F,      //  i = 237 pixel1 = 14 pixel2 = 13 value1 = 238 value2 = 221
    0xBC20F,      //  i = 238 pixel1 = 14 pixel2 = 14 value1 = 238 value2 = 238
    0xBFE0F,      //  i = 239 pixel1 = 14 pixel2 = 15 value1 = 238 value2 = 255
    0x7FE00,      //  i = 240 pixel1 = 15 pixel2 = 0 value1 = 255 value2 = 0
    0x7C200,      //  i = 241 pixel1 = 15 pixel2 = 1 value1 = 255 value2 = 17
    0x78600,      //  i = 242 pixel1 = 15 pixel2 = 2 value1 = 255 value2 = 34
    0x7BA00,      //  i = 243 pixel1 = 15 pixel2 = 3 value1 = 255 value2 = 51
    0x70E00,      //  i = 244 pixel1 = 15 pixel2 = 4 value1 = 255 value2 = 68
    0x73200,      //  i = 245 pixel1 = 15 pixel2 = 5 value1 = 255 value2 = 85
    0x1DE00,      //  i = 246 pixel1 = 15 pixel2 = 6 value1 = 255 value2 = 102
    0xA1E00,      //  i = 247 pixel1 = 15 pixel2 = 7 value1 = 255 value2 = 119
    0x61E00,      //  i = 248 pixel1 = 15 pixel2 = 8 value1 = 255 value2 = 136
    0xDDE00,      //  i = 249 pixel1 = 15 pixel2 = 9 value1 = 255 value2 = 153
    0xB3200,      //  i = 250 pixel1 = 15 pixel2 = 10 value1 = 255 value2 = 170
    0xB0E00,      //  i = 251 pixel1 = 15 pixel2 = 11 value1 = 255 value2 = 187
    0xBBA00,      //  i = 252 pixel1 = 15 pixel2 = 12 value1 = 255 value2 = 204
    0xB8600,      //  i = 253 pixel1 = 15 pixel2 = 13 value1 = 255 value2 = 221
    0xBC200,      //  i = 254 pixel1 = 15 pixel2 = 14 value1 = 255 value2 = 238
    0xBFE00       //  i = 255 pixel1 = 15 pixel2 = 15 value1 = 255 value2 = 255
};


/*

IIgs video modes?
320x640 x 200. 4096 colors 16 colors per line.  256 colors per frame. 
32K.  Aux 64k.   $2000-9FFF
$2000-9D00 pixel data
$9D00-9E00 scan line control
$9E00-9FFF Palettes.  
16 palettes of 16 colors out of 4096 12b 4b each color
640 mode palette is wonky
Color fill mode 320 mode only
GSOS looks to be 320x200
AUX bank needs to be bigger (2000-A000)
320 mode.  Double pixels.  Lookup table. 192k
640 mode.  Compute pixels.  
256 colors.  1.5k lookup.  

*/


//  Computed palettes for drawing
uint32_t s_320_palette_red[256];                    //  16 x 16
uint32_t s_320_palette_green[256];
uint32_t s_320_palette_blue[256];

void DELAYED_COPY_CODE(encode_shr_320_palette)(const uint8_t *in_palette)
{
    for (int i = 0; i < 256; i++)
    {
        uint8_t value = *in_palette++;
        s_320_palette_blue[i] = s_shr_320_to_tmds[value & 0x0F];
        value = value >> 4;
        s_320_palette_green[i] = s_shr_320_to_tmds[value & 0x0F];

        value = *in_palette++;
        s_320_palette_red[i] = s_shr_320_to_tmds[value & 0x0F];
    }
}

uint32_t s_640_palette_red[512];                    //  16 x 16 x 2
uint32_t s_640_palette_green[512];
uint32_t s_640_palette_blue[512];

void DELAYED_COPY_CODE(encode_shr_640_palette)(const uint8_t *in_palette)
{
    int palette_640_index = 0;

    for (int i = 0; i < 16; i++)
    {
        //  Can all of this be done as a lookup table ???
        for (int pixel12 = 0; pixel12 < 16; pixel12++)
        {
            int pixel1 = pixel12 >> 2;
            int pixel2 = pixel12 & 0x03;

            pixel1 = (pixel1 + 0x08) * 2;               //  Pallete index
            pixel2 = (pixel2 + 0x0C) * 2;

            //  Green and Blue
            uint8_t palette1 = in_palette[pixel1];       //  Fetch the Green / Blue palette data
            uint8_t palette2 = in_palette[pixel2];

            //  Blue
            uint8_t value = ((palette1 & 0x0F) << 4) | (palette2 & 0x0F);
            s_640_palette_blue[palette_640_index] = s_shr_640_to_tmds[value];

            //  Green
            palette1 = palette1 >> 4;
            palette2 = palette2 >> 4;
            value = ((palette1 & 0x0F) << 4) | (palette2 & 0x0F);
            s_640_palette_green[palette_640_index] = s_shr_640_to_tmds[value];

            //  Red
            palette1 = in_palette[pixel1 + 1];
            palette2 = in_palette[pixel2 + 1];

            value = ((palette1 & 0x0F) << 4) | (palette2 & 0x0F);
            s_640_palette_red[palette_640_index] = s_shr_640_to_tmds[value];

            palette_640_index++;
        }

        for (int pixel34 = 0; pixel34 < 16; pixel34++)
        {
            int pixel3 = pixel34 >> 2;
            int pixel4 = pixel34 & 0x03;

            pixel3 = (pixel3 + 0x00) * 2;
            pixel4 = (pixel4 + 0x04) * 2;

            //  Green and Blue
            uint8_t palette3 = in_palette[pixel3];       //  Fetch the Green / Blue palette data
            uint8_t palette4 = in_palette[pixel4];

            //  Blue
            uint8_t value = ((palette3 & 0x0F) << 4) | (palette4 & 0x0F);
            s_640_palette_blue[palette_640_index] = s_shr_640_to_tmds[value];

            //  Green
            palette3 = palette3 >> 4;
            palette4 = palette4 >> 4;
            value = ((palette3 & 0x0F) << 4) | (palette4 & 0x0F);
            s_640_palette_green[palette_640_index] = s_shr_640_to_tmds[value];

            //  Red
            palette3 = in_palette[pixel3 + 1];
            palette4 = in_palette[pixel4 + 1];

            value = ((palette3 & 0x0F) << 4) | (palette4 & 0x0F);
            s_640_palette_red[palette_640_index] = s_shr_640_to_tmds[value];

            palette_640_index++;
        }

        in_palette = in_palette + (16 * 2);
    }
}

void DELAYED_COPY_CODE(encode_shr_palette)(uint8_t* in_line_mem)
{
    //  Encode both, incase they are needed
    encode_shr_320_palette(&(in_line_mem[(0x9E00 - 0x2000)]));
    encode_shr_640_palette(&(in_line_mem[(0x9E00 - 0x2000)]));
}

void DELAYED_COPY_CODE(render_shr_320_line_mem)(const uint8_t *in_line_mem, int in_palette_index)
{
    dvi_get_scanline(tmdsbuf);
    dvi_scanline_rgb640(tmdsbuf, tmdsbuf_red, tmdsbuf_green, tmdsbuf_blue);

    in_palette_index = in_palette_index * 16;

    for (uint x = 0; x < 160; x++)
    {
        uint8_t value = *in_line_mem++;
        uint32_t pixel1 = (value >> 4) + in_palette_index;
        uint32_t pixel2 = (value & 0x0F) + in_palette_index;
        
        *(tmdsbuf_red++) = s_320_palette_red[pixel1];
        *(tmdsbuf_green++) = s_320_palette_green[pixel1];
        *(tmdsbuf_blue++) = s_320_palette_blue[pixel1];

        *(tmdsbuf_red++) = s_320_palette_red[pixel2];
        *(tmdsbuf_green++) = s_320_palette_green[pixel2];
        *(tmdsbuf_blue++) = s_320_palette_blue[pixel2];
    }

    dvi_send_scanline(tmdsbuf);
}

void DELAYED_COPY_CODE(render_shr_320_fill_line_mem)(const uint8_t *in_line_mem, int in_palette_index)
{
    dvi_get_scanline(tmdsbuf);
    dvi_scanline_rgb640(tmdsbuf, tmdsbuf_red, tmdsbuf_green, tmdsbuf_blue);

    in_palette_index = in_palette_index * 16;

    uint32_t last_pixel = ((*in_line_mem) >> 4);

    for (uint x = 0; x < 160; x++)
    {
        uint8_t value = *in_line_mem++;
        uint32_t pixel1 = (value >> 4);
        uint32_t pixel2 = (value & 0x0F);

        if (pixel1 == 0)
            pixel1 = last_pixel;
        else
            last_pixel = pixel1;

        if (pixel2 == 0)
            pixel2 = last_pixel;
        else
            last_pixel = pixel2;
        
        pixel1 = pixel1 + in_palette_index;
        pixel2 = pixel2 + in_palette_index;

        *(tmdsbuf_red++) = s_320_palette_red[pixel1];
        *(tmdsbuf_green++) = s_320_palette_green[pixel1];
        *(tmdsbuf_blue++) = s_320_palette_blue[pixel1];

        *(tmdsbuf_red++) = s_320_palette_red[pixel2];
        *(tmdsbuf_green++) = s_320_palette_green[pixel2];
        *(tmdsbuf_blue++) = s_320_palette_blue[pixel2];
    }

    dvi_send_scanline(tmdsbuf);
}

void DELAYED_COPY_CODE(render_shr_640_line_mem)(const uint8_t *in_line_mem, int in_palette_index)
{
    in_palette_index = in_palette_index * 32;

    dvi_get_scanline(tmdsbuf);
    dvi_scanline_rgb640(tmdsbuf, tmdsbuf_red, tmdsbuf_green, tmdsbuf_blue);

    for (uint x = 0; x < 160; x++)
    {
        uint8_t value = *in_line_mem++;
        uint32_t pixel12 = (value >> 4) + in_palette_index;
        uint32_t pixel34 = (value & 0x0F) + in_palette_index + 16;
        
        *(tmdsbuf_red++) = s_640_palette_red[pixel12];
        *(tmdsbuf_green++) = s_640_palette_green[pixel12];
        *(tmdsbuf_blue++) = s_640_palette_blue[pixel12];

        *(tmdsbuf_red++) = s_640_palette_red[pixel34];
        *(tmdsbuf_green++) = s_640_palette_green[pixel34];
        *(tmdsbuf_blue++) = s_640_palette_blue[pixel34];
    }

    dvi_send_scanline(tmdsbuf);
}

void DELAYED_COPY_CODE(render_shr_mem)(uint8_t* in_line_mem, uint32_t in_start_line, uint32_t in_end_line)
{
    for(uint32_t line = in_start_line; line < in_end_line; line++)
    {
        const uint8_t *line_mem = (const uint8_t *)(&(in_line_mem[(line * 160)]));
        uint8_t scanline_control = in_line_mem[(0x9D00 - 0x2000) + line];

        //  High bit set is 640, bit 5 is color fill mode ???
        if ((scanline_control & 0x80) != 0)
        {
            render_shr_640_line_mem(line_mem, (scanline_control & 0x0F));
        }
        else
        {
            if ((scanline_control & 0x20) == 0)
                render_shr_320_line_mem(line_mem, (scanline_control & 0x0F));
            else
                render_shr_320_fill_line_mem(line_mem, (scanline_control & 0x0F));
        }
    }
}

