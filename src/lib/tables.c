/*
 * Copyright (c) 2020 Devin Smith <devin@devinsmith.net>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <stdio.h>
#include <stdlib.h>

#include "tables.h"
#include "resource.h"

/* These are framebuffer offsets by line with each line being increased
 * by 320 (Width = 320 pixels).
 * So line 0 = 0x0000, line 1 = 0x140 (320), line 2 = 0x280 (640) and so
 * forth.
 *
 * The original dragon.com has these embedded into the COM file at 0xAEB2
 * maybe because it didn't want to deal with multiplication (?). We copy them
 * here in order to authentically match the original executable. */
/* 0xAEB2 - 0xB041 (400 bytes) */
uint16_t framebuffer_line_offsets[200] = {
  0x0000, 0x0140, 0x0280, 0x03C0, 0x0500, 0x0640, // 0xAEB2-0xAEBD
  0x0780, 0x08C0, 0x0A00, 0x0B40, 0x0C80, 0x0DC0, // 0xAEBE-0xAEC9
  0x0F00, 0x1040, 0x1180, 0x12C0, 0x1400, 0x1540, // 0xAECA-0xAED5
  0x1680, 0x17C0, 0x1900, 0x1A40, 0x1B80, 0x1CC0, // 0xAED6-0xAEE1
  0x1E00, 0x1F40, 0x2080, 0x21C0, 0x2300, 0x2440, // 0xAEE2-0xAEED
  0x2580, 0x26C0, 0x2800, 0x2940, 0x2A80, 0x2BC0, // 0xAEEE-0xAEF9
  0x2D00, 0x2E40, 0x2F80, 0x30C0, 0x3200, 0x3340, // 0xAEFA-0xAF05
  0x3480, 0x35C0, 0x3700, 0x3840, 0x3980, 0x3AC0, // 0xAF06-0xAF11
  0x3C00, 0x3D40, 0x3E80, 0x3FC0, 0x4100, 0x4240, // 0xAF12-0xAF1D
  0x4380, 0x44C0, 0x4600, 0x4740, 0x4880, 0x49C0, // 0xAF1E-0xAF29
  0x4B00, 0x4C40, 0x4D80, 0x4EC0, 0x5000, 0x5140, // 0xAF2A-0xAF35
  0x5280, 0x53C0, 0x5500, 0x5640, 0x5780, 0x58C0, // 0xAF36-0xAF41
  0x5A00, 0x5B40, 0x5C80, 0x5DC0, 0x5F00, 0x6040, // 0xAF42-0xAF4D
  0x6180, 0x62C0, 0x6400, 0x6540, 0x6680, 0x67C0, // 0xAF4E-0xAF59
  0x6900, 0x6A40, 0x6B80, 0x6CC0, 0x6E00, 0x6F40, // 0xAF5A-0xAF65
  0x7080, 0x71C0, 0x7300, 0x7440, 0x7580, 0x76C0, // 0xAF66-0xAF71
  0x7800, 0x7940, 0x7A80, 0x7BC0, 0x7D00, 0x7E40, // 0xAF72-0xAF7D
  0x7F80, 0x80C0, 0x8200, 0x8340, 0x8480, 0x85C0, // 0xAF7E-0xAF89
  0x8700, 0x8840, 0x8980, 0x8AC0, 0x8C00, 0x8D40, // 0xAF8A-0xAF95
  0x8E80, 0x8FC0, 0x9100, 0x9240, 0x9380, 0x94C0, // 0xAF96-0xAFA1
  0x9600, 0x9740, 0x9880, 0x99C0, 0x9B00, 0x9C40, // 0xAFA2-0xAFAD
  0x9D80, 0x9EC0, 0xA000, 0xA140, 0xA280, 0xA3C0, // 0xAFAE-0xAFB9
  0xA500, 0xA640, 0xA780, 0xA8C0, 0xAA00, 0xAB40, // 0xAFBA-0xAFC5
  0xAC80, 0xADC0, 0xAF00, 0xB040, 0xB180, 0xB2C0, // 0xAFC6-0xAFD1
  0xB400, 0xB540, 0xB680, 0xB7C0, 0xB900, 0xBA40, // 0xAFD2-0xAFDD
  0xBB80, 0xBCC0, 0xBE00, 0xBF40, 0xC080, 0xC1C0, // 0xAFDE-0xAFE9
  0xC300, 0xC440, 0xC580, 0xC6C0, 0xC800, 0xC940, // 0xAFEA-0xAFF5
  0xCA80, 0xCBC0, 0xCD00, 0xCE40, 0xCF80, 0xD0C0, // 0xAFF6-0xB001
  0xD200, 0xD340, 0xD480, 0xD5C0, 0xD700, 0xD840, // 0xB002-0xB00D
  0xD980, 0xDAC0, 0xDC00, 0xDD40, 0xDE80, 0xDFC0, // 0xB00E-0xB019
  0xE100, 0xE240, 0xE380, 0xE4C0, 0xE600, 0xE740, // 0xB01A-0xB025
  0xE880, 0xE9C0, 0xEB00, 0xEC40, 0xED80, 0xEEC0, // 0xB026-0xB031
  0xF000, 0xF140, 0xF280, 0xF3C0, 0xF500, 0xF640, // 0xB032-0xB03D
  0xF780, 0xF8C0 // 0xB03E-0xB041
};

// 0xB152 - 0xB251
unsigned char b152_table[256] = {
  0x00, 0x10, 0x20, 0x30, 0x40, 0x50, 0x60, 0x70, // 0xB152-0xB159
  0x80, 0x90, 0xA0, 0xB0, 0xC0, 0xD0, 0xE0, 0xF0, // 0xB15A-0xB161
  0x01, 0x11, 0x21, 0x31, 0x41, 0x51, 0x61, 0x71, // 0xB162-0xB169
  0x81, 0x91, 0xA1, 0xB1, 0xC1, 0xD1, 0xE1, 0xF1, // 0xB16A-0xB171
  0x02, 0x12, 0x22, 0x32, 0x42, 0x52, 0x62, 0x72, // 0xB172-0xB179
  0x82, 0x92, 0xA2, 0xB2, 0xC2, 0xD2, 0xE2, 0xF2, // 0xB17A-0xB181
  0x03, 0x13, 0x23, 0x33, 0x43, 0x53, 0x63, 0x73, // 0xB182-0xB189
  0x83, 0x93, 0xA3, 0xB3, 0xC3, 0xD3, 0xE3, 0xF3, // 0xB18A-0xB191
  0x04, 0x14, 0x24, 0x34, 0x44, 0x54, 0x64, 0x74, // 0xB192-0xB199
  0x84, 0x94, 0xA4, 0xB4, 0xC4, 0xD4, 0xE4, 0xF4, // 0xB19A-0xB1A1
  0x05, 0x15, 0x25, 0x35, 0x45, 0x55, 0x65, 0x75, // 0xB1A2-0xB1A9
  0x85, 0x95, 0xA5, 0xB5, 0xC5, 0xD5, 0xE5, 0xF5, // 0xB1AA-0xB1B1
  0x06, 0x16, 0x26, 0x36, 0x46, 0x56, 0x66, 0x76, // 0xB1B2-0xB1B9
  0x86, 0x96, 0xA6, 0xB6, 0xC6, 0xD6, 0xE6, 0xF6, // 0xB1BA-0xB1C1
  0x07, 0x17, 0x27, 0x37, 0x47, 0x57, 0x67, 0x77, // 0xB1C2-0xB1C9
  0x87, 0x97, 0xA7, 0xB7, 0xC7, 0xD7, 0xE7, 0xF7, // 0xB1CA-0xB1D1
  0x08, 0x18, 0x28, 0x38, 0x48, 0x58, 0x68, 0x78, // 0xB1D2-0xB1D9
  0x88, 0x98, 0xA8, 0xB8, 0xC8, 0xD8, 0xE8, 0xF8, // 0xB1DA-0xB1E1
  0x09, 0x19, 0x29, 0x39, 0x49, 0x59, 0x69, 0x79, // 0xB1E2-0xB1E9
  0x89, 0x99, 0xA9, 0xB9, 0xC9, 0xD9, 0xE9, 0xF9, // 0xB1EA-0xB1F1
  0x0A, 0x1A, 0x2A, 0x3A, 0x4A, 0x5A, 0x6A, 0x7A, // 0xB1F2-0xB1F9
  0x8A, 0x9A, 0xAA, 0xBA, 0xCA, 0xDA, 0xEA, 0xFA, // 0xB1FA-0xB201
  0x0B, 0x1B, 0x2B, 0x3B, 0x4B, 0x5B, 0x6B, 0x7B, // 0xB202-0xB209
  0x8B, 0x9B, 0xAB, 0xBB, 0xCB, 0xDB, 0xEB, 0xFB, // 0xB20A-0xB211
  0x0C, 0x1C, 0x2C, 0x3C, 0x4C, 0x5C, 0x6C, 0x7C, // 0xB212-0xB219
  0x8C, 0x9C, 0xAC, 0xBC, 0xCC, 0xDC, 0xEC, 0xFC, // 0xB21A-0xB221
  0x0D, 0x1D, 0x2D, 0x3D, 0x4D, 0x5D, 0x6D, 0x7D, // 0xB222-0xB229
  0x8D, 0x9D, 0xAD, 0xBD, 0xCD, 0xDD, 0xED, 0xFD, // 0xB22A-0xB231
  0x0E, 0x1E, 0x2E, 0x3E, 0x4E, 0x5E, 0x6E, 0x7E, // 0xB232-0xB239
  0x8E, 0x9E, 0xAE, 0xBE, 0xCE, 0xDE, 0xEE, 0xFE, // 0xB23A-0xB241
  0x0F, 0x1F, 0x2F, 0x3F, 0x4F, 0x5F, 0x6F, 0x7F, // 0xB242-0xB249
  0x8F, 0x9F, 0xAF, 0xBF, 0xCF, 0xDF, 0xEF, 0xFF, // 0xB24A-0xB251
};

/* 0xB252-0xB351 */
unsigned char and_table[256] = {
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0F, 0x00, // 0xB252-0xB259
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0xB25A-0xB261
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0F, 0x00, // 0xB262-0xB269
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0xB26A-0xB271
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0F, 0x00, // 0xB272-0xB279
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0xB27A-0xB281
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0F, 0x00, // 0xB282-0xB289
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0xB28A-0xB291
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0F, 0x00, // 0xB292-0xB299
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0xB29A-0xB2A1
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0F, 0x00, // 0xB2A2-0xB2A9
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0xB2AA-0xB2B1
  0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xFF, 0xF0, // 0xB2B2-0xB2B9
  0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, 0xF0, // 0xB2BA-0xB2C1
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0F, 0x00, // 0xB2C2-0xB2C9
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0xB2CA-0xB2D1
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0F, 0x00, // 0xB2D2-0xB2D9
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0xB2DA-0xB2E1
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0F, 0x00, // 0xB2E2-0xB2E9
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0xB2EA-0xB2F1
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0F, 0x00, // 0xB2F2-0xB2F9
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0xB2FA-0xB301
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0F, 0x00, // 0xB302-0xB309
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0xB30A-0xB311
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0F, 0x00, // 0xB312-0xB319
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0xB31A-0xB321
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0F, 0x00, // 0xB322-0xB329
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0xB32A-0xB331
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0F, 0x00, // 0xB332-0xB339
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 0xB33A-0xB341
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0F, 0x00, // 0xB342-0xB349
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00  // 0xB34A-0xB351
};

/* 0xB352-0xB451 */
unsigned char or_table[256] = {
  0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x00, 0x07, // 0xB352-0xB359
  0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, // 0xB35A-0xB361
  0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x10, 0x17, // 0xB362-0xB369
  0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, // 0xB36A-0xB371
  0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x20, 0x27, // 0xB372-0xB379
  0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F, // 0xB37A-0xB381
  0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x30, 0x37, // 0xB382-0xB389
  0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F, // 0xB38A-0xB391
  0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x40, 0x47, // 0xB392-0xB399
  0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F, // 0xB39A-0xB3A1
  0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x50, 0x57, // 0xB3A2-0xB3A9
  0x58, 0x59, 0x5A, 0x5B, 0x5C, 0x5D, 0x5E, 0x5F, // 0xB3AA-0xB3B1
  0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x00, 0x07, // 0xB3B2-0xB3B9
  0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F, // 0xB3BA-0xB3C1
  0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x50, 0x77, // 0xB3C2-0xB3C9
  0x78, 0x79, 0x7A, 0x7B, 0x7C, 0x7D, 0x7E, 0x7F, // 0xB3CA-0xB3D1
  0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x80, 0x87, // 0xB3D2-0xB3D9
  0x88, 0x89, 0x8A, 0x8B, 0x8C, 0x8D, 0x8E, 0x8F, // 0xB3DA-0xB3E1
  0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x90, 0x97, // 0xB3E2-0xB3E9
  0x98, 0x99, 0x9A, 0x9B, 0x9C, 0x9D, 0x9E, 0x9F, // 0xB3EA-0xB3F1
  0xA0, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5, 0xA0, 0xA7, // 0xB3F2-0xB3F9
  0xA8, 0xA9, 0xAA, 0xAB, 0xAC, 0xAD, 0xAE, 0xAF, // 0xB3FA-0xB401
  0xB0, 0xB1, 0xB2, 0xB3, 0xB4, 0xB5, 0xB0, 0xB7, // 0xB402-0xB409
  0xB8, 0xB9, 0xBA, 0xBB, 0xBC, 0xBD, 0xBE, 0xBF, // 0xB40A-0xB411
  0xC0, 0xC1, 0xC2, 0xC3, 0xC4, 0xC5, 0xC0, 0xC7, // 0xB412-0xB419
  0xC8, 0xC9, 0xCA, 0xCB, 0xCC, 0xCD, 0xCE, 0xCF, // 0xB41A-0xB421
  0xD0, 0xD1, 0xD2, 0xD3, 0xD4, 0xD5, 0xD0, 0xD7, // 0xB422-0xB429
  0xD8, 0xD9, 0xDA, 0xDB, 0xDC, 0xDD, 0xDE, 0xDF, // 0xB42A-0xB431
  0xE0, 0xE1, 0xE2, 0xE3, 0xE4, 0xE5, 0xE0, 0xE7, // 0xB432-0xB439
  0xE8, 0xE9, 0xEA, 0xEB, 0xEC, 0xED, 0xEE, 0xEF, // 0xB43A-0xB441
  0xF0, 0xF1, 0xF2, 0xF3, 0xF4, 0xF5, 0xF0, 0xF7, // 0xB442-0xB449
  0xF8, 0xF9, 0xFA, 0xFB, 0xFC, 0xFD, 0xFE, 0xFF  // 0xB44A-0xB451
};

// 0xB452 - 0xB651
static uint16_t and_table_B452[256] = {
  0x0FF0, 0x0FF0, 0x0FF0, 0x0FF0, 0x0FF0, 0x0FF0, 0xFFF0, 0x0FF0, // 0xB452
  0x0FF0, 0x0FF0, 0x0FF0, 0x0FF0, 0x0FF0, 0x0FF0, 0x0FF0, 0x0FF0, // 0xB462
  0x0FF0, 0x0FF0, 0x0FF0, 0x0FF0, 0x0FF0, 0x0FF0, 0xFFF0, 0x0FF0, // 0xB472
  0x0FF0, 0x0FF0, 0x0FF0, 0x0FF0, 0x0FF0, 0x0FF0, 0x0FF0, 0x0FF0, // 0xB482
  0x0FF0, 0x0FF0, 0x0FF0, 0x0FF0, 0x0FF0, 0x0FF0, 0xFFF0, 0x0FF0, // 0xB492
  0x0FF0, 0x0FF0, 0x0FF0, 0x0FF0, 0x0FF0, 0x0FF0, 0x0FF0, 0x0FF0, // 0xB4A2
  0x0FF0, 0x0FF0, 0x0FF0, 0x0FF0, 0x0FF0, 0x0FF0, 0xFFF0, 0x0FF0, // 0xB4B2
  0x0FF0, 0x0FF0, 0x0FF0, 0x0FF0, 0x0FF0, 0x0FF0, 0x0FF0, 0x0FF0, // 0xB4C2
  0x0FF0, 0x0FF0, 0x0FF0, 0x0FF0, 0x0FF0, 0x0FF0, 0xFFF0, 0x0FF0, // 0xB4D2
  0x0FF0, 0x0FF0, 0x0FF0, 0x0FF0, 0x0FF0, 0x0FF0, 0x0FF0, 0x0FF0, // 0xB4E2
  0x0FF0, 0x0FF0, 0x0FF0, 0x0FF0, 0x0FF0, 0x0FF0, 0xFFF0, 0x0FF0, // 0xB4F2
  0x0FF0, 0x0FF0, 0x0FF0, 0x0FF0, 0x0FF0, 0x0FF0, 0x0FF0, 0x0FF0, // 0xB502
  0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF, 0xFFFF, 0x0FFF, // 0xB512
  0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF, 0x0FFF, // 0xB522
  0x0FF0, 0x0FF0, 0x0FF0, 0x0FF0, 0x0FF0, 0x0FF0, 0xFFF0, 0x0FF0, // 0xB532
  0x0FF0, 0x0FF0, 0x0FF0, 0x0FF0, 0x0FF0, 0x0FF0, 0x0FF0, 0x0FF0, // 0xB542
  0x0FF0, 0x0FF0, 0x0FF0, 0x0FF0, 0x0FF0, 0x0FF0, 0xFFF0, 0x0FF0, // 0xB552
  0x0FF0, 0x0FF0, 0x0FF0, 0x0FF0, 0x0FF0, 0x0FF0, 0x0FF0, 0x0FF0, // 0xB562
  0x0FF0, 0x0FF0, 0x0FF0, 0x0FF0, 0x0FF0, 0x0FF0, 0xFFF0, 0x0FF0, // 0xB572
  0x0FF0, 0x0FF0, 0x0FF0, 0x0FF0, 0x0FF0, 0x0FF0, 0x0FF0, 0x0FF0, // 0xB582
  0x0FF0, 0x0FF0, 0x0FF0, 0x0FF0, 0x0FF0, 0x0FF0, 0xFFF0, 0x0FF0, // 0xB592
  0x0FF0, 0x0FF0, 0x0FF0, 0x0FF0, 0x0FF0, 0x0FF0, 0x0FF0, 0x0FF0, // 0xB5A2
  0x0FF0, 0x0FF0, 0x0FF0, 0x0FF0, 0x0FF0, 0x0FF0, 0xFFF0, 0x0FF0, // 0xB5B2
  0x0FF0, 0x0FF0, 0x0FF0, 0x0FF0, 0x0FF0, 0x0FF0, 0x0FF0, 0x0FF0, // 0xB5C2
  0x0FF0, 0x0FF0, 0x0FF0, 0x0FF0, 0x0FF0, 0x0FF0, 0xFFF0, 0x0FF0, // 0xB5D2
  0x0FF0, 0x0FF0, 0x0FF0, 0x0FF0, 0x0FF0, 0x0FF0, 0x0FF0, 0x0FF0, // 0xB5E2
  0x0FF0, 0x0FF0, 0x0FF0, 0x0FF0, 0x0FF0, 0x0FF0, 0xFFF0, 0x0FF0, // 0xB5F2
  0x0FF0, 0x0FF0, 0x0FF0, 0x0FF0, 0x0FF0, 0x0FF0, 0x0FF0, 0x0FF0, // 0xB602
  0x0FF0, 0x0FF0, 0x0FF0, 0x0FF0, 0x0FF0, 0x0FF0, 0xFFF0, 0x0FF0, // 0xB612
  0x0FF0, 0x0FF0, 0x0FF0, 0x0FF0, 0x0FF0, 0x0FF0, 0x0FF0, 0x0FF0, // 0xB622
  0x0FF0, 0x0FF0, 0x0FF0, 0x0FF0, 0x0FF0, 0x0FF0, 0xFFF0, 0x0FF0, // 0xB632
  0x0FF0, 0x0FF0, 0x0FF0, 0x0FF0, 0x0FF0, 0x0FF0, 0x0FF0, 0x0FF0, // 0xB642
};

// 0xB652 - 0xB851
static uint16_t or_table_B652[256] = {
  0x0000, 0x1000, 0x2000, 0x3000, 0x4000, 0x5000, 0x0000, 0x7000, // 0xB652
  0x8000, 0x9000, 0xA000, 0xB000, 0xC000, 0xD000, 0xE000, 0xF000, // 0xB662
  0x0001, 0x1001, 0x2001, 0x3001, 0x4001, 0x5001, 0x0001, 0x7001, // 0xB672
  0x8001, 0x9001, 0xA001, 0xB001, 0xC001, 0xD001, 0xE001, 0xF001, // 0xB682
  0x0002, 0x1002, 0x2002, 0x3002, 0x4002, 0x5002, 0x0002, 0x7002, // 0xB692
  0x8002, 0x9002, 0xA002, 0xB002, 0xC002, 0xD002, 0xE002, 0xF002, // 0xB6A2
  0x0003, 0x1003, 0x2003, 0x3003, 0x4003, 0x5003, 0x0003, 0x7003, // 0xB6B2
  0x8003, 0x9003, 0xA003, 0xB003, 0xC003, 0xD003, 0xE003, 0xF003, // 0xB6C2
  0x0004, 0x1004, 0x2004, 0x3004, 0x4004, 0x5004, 0x0004, 0x7004, // 0xB6D2
  0x8004, 0x9004, 0xA004, 0xB004, 0xC004, 0xD004, 0xE004, 0xF004, // 0xB6E2
  0x0005, 0x1005, 0x2005, 0x3005, 0x4005, 0x5005, 0x0005, 0x7005, // 0xB6F2
  0x8005, 0x9005, 0xA005, 0xB005, 0xC005, 0xD005, 0xE005, 0xF005, // 0xB702
  0x0000, 0x1000, 0x2000, 0x3000, 0x4000, 0x5000, 0x0000, 0x7000, // 0xB712
  0x8000, 0x9000, 0xA000, 0xB000, 0xC000, 0xD000, 0xE000, 0xF000, // 0xB722
  0x0007, 0x1007, 0x2007, 0x3007, 0x4007, 0x5007, 0x0007, 0x7007, // 0xB732
  0x8007, 0x9007, 0xA007, 0xB007, 0xC007, 0xD007, 0xE007, 0xF007, // 0xB742
  0x0008, 0x1008, 0x2008, 0x3008, 0x4008, 0x5008, 0x0008, 0x7008, // 0xB752
  0x8008, 0x9008, 0xA008, 0xB008, 0xC008, 0xD008, 0xE008, 0xF008, // 0xB762
  0x0009, 0x1009, 0x2009, 0x3009, 0x4009, 0x5009, 0x0009, 0x7009, // 0xB772
  0x8009, 0x9009, 0xA009, 0xB009, 0xC009, 0xD009, 0xE009, 0xF009, // 0xB782
  0x000A, 0x100A, 0x200A, 0x300A, 0x400A, 0x500A, 0x000A, 0x700A, // 0xB792
  0x800A, 0x900A, 0xA00A, 0xB00A, 0xC00A, 0xD00A, 0xE00A, 0xF00A, // 0xB7A2
  0x000B, 0x100B, 0x200B, 0x300B, 0x400B, 0x500B, 0x000B, 0x700B, // 0xB7B2
  0x800B, 0x900B, 0xA00B, 0xB00B, 0xC00B, 0xD00B, 0xE00B, 0xF00B, // 0xB7C2
  0x000C, 0x100C, 0x200C, 0x300C, 0x400C, 0x500C, 0x000C, 0x700C, // 0xB7D2
  0x800C, 0x900C, 0xA00C, 0xB00C, 0xC00C, 0xD00C, 0xE00C, 0xF00C, // 0xB7E2
  0x000D, 0x100D, 0x200D, 0x300D, 0x400D, 0x500D, 0x000D, 0x700D, // 0xB7F2
  0x800D, 0x900D, 0xA00D, 0xB00D, 0xC00D, 0xD00D, 0xE00D, 0xF00D, // 0xB802
  0x000E, 0x100E, 0x200E, 0x300E, 0x400E, 0x500E, 0x000E, 0x700E, // 0xB812
  0x800E, 0x900E, 0xA00E, 0xB00E, 0xC00E, 0xD00E, 0xE00E, 0xF00E, // 0xB822
  0x000F, 0x100F, 0x200F, 0x300F, 0x400F, 0x500F, 0x000F, 0x700F, // 0xB832
  0x800F, 0x900F, 0xA00F, 0xB00F, 0xC00F, 0xD00F, 0xE00F, 0xF00F, // 0xB842
};

// 0x1BC1 - 0x1BE4
static uint8_t unknown_1BC1[] = {
  0x02, 0x04, 0x80, 0x01, 0x1C, 0x1C, 0x1C, 0x1E, 0xD1, 0x1B, 0xD6,
  0x1B, 0xDC, 0x1B, 0xE1, 0x1B, 0x22, 0x44, 0xA7, 0x18, 0xA0, 0x83,
  0xD5, 0x27, 0xB8, 0xC5, 0x00, 0x94, 0xE8, 0xE7, 0x18, 0xA0, 0x29,
  0x84, 0x50, 0x00
};

// Increasing by 23 (0x17) ?
static uint16_t unknown_4456[] = {
  0x0000, 0x0017, 0x002E, 0x0045, 0x005C, 0x0073, 0x008A, 0x00A1,
  0x00B8, 0x00CF, 0x00E6, 0x00FD, 0x00E8
};

/* Stored in dragon.com at 0xBF52-0xC352
 * See ui.c for how this is interpreted.
 *
 * Alpha characters are OR'd with 0x80 like below:
 * 0xC1 = 'A' | 0x80
 * 0xE1 = 'a' | 0x80
 *
 * Each character is an 8x8 square and stored as 8 bytes.
 * For example, the letter "A" is stored as:
 *
 * 0x30 0x78 0xCC 0xCC 0xFC 0xCC 0xCC 0x00
 *
 * Each byte represents a single 8 pixel line:
 * 00110000
 * 01111000
 * 11001100
 * 11001100
 * 11111100
 * 11001100
 * 11001100
 * 00000000
 *
 * Plotting only the "on" bits:
 *
 *   ##
 *  ####
 * ##  ##
 * ##  ##
 * ######
 * ##  ##
 * ##  ##
 *
 **/
static unsigned char *chr_table;

void load_chr_table()
{
  chr_table = com_extract(0xBF52, 0x400);
}

void unload_chr_table()
{
  free(chr_table);
}

const unsigned char *get_chr(int chr_num)
{
  chr_num = chr_num & 0x7F; // Max number of characters.
  chr_num = chr_num << 3; // each character is 8 bytes.

  return chr_table + chr_num;
}

uint8_t get_and_table(uint8_t offset)
{
  return and_table[offset];
}

uint8_t get_or_table(uint8_t offset)
{
  return or_table[offset];
}

uint16_t get_and_table_B452(uint8_t offset)
{
  return and_table_B452[offset];
}

uint16_t get_or_table_B652(uint8_t offset)
{
  return or_table_B652[offset];
}

uint8_t get_1BC1_table(uint8_t offset)
{
  return unknown_1BC1[offset];
}

uint16_t get_line_offset(uint8_t index)
{
  return framebuffer_line_offsets[index];
}

uint16_t get_unknown_4456(uint8_t index)
{
  if (index >= sizeof(unknown_4456) / sizeof(unknown_4456[0])) {
    printf("Requested %d index out of bounds in unknown 4456\n", index);
    exit(1);
  }
  return unknown_4456[index];
}
