/*
textdefs.h

diStorm3 - Powerful disassembler for x86/AMD64
http://ragestorm.net/distorm/
distorm at gmail dot com
Copyright (C) 2003-2015 Gil Dabah

Licensed under the diStorm version 3.3 Commercial License, see
the terms and conditions of the license in the file 'LICENSE.TXT'.
*/


#ifndef TEXTDEFS_H
#define TEXTDEFS_H

#include "config.h"
#include "wstring.h"

#ifndef DISTORM_LIGHT

#define PLUS_DISP_CHR '+'
#define MINUS_DISP_CHR '-'
#define OPEN_CHR '['
#define CLOSE_CHR ']'
#define SP_CHR ' '
#define SEG_OFF_CHR ':'

/*
Naming Convention:

* get - returns a pointer to a string.
* str - concatenates to string.

* hex - means the function is used for hex dump (number is padded to required size) - Little Endian output.
* code - means the function is used for disassembled instruction - Big Endian output.
* off - means the function is used for 64bit offset - Big Endian output.

* h - '0x' in front of the string.

* b - byte
* dw - double word (can be used for word also)
* qw - quad word

* all numbers are in HEX.
*/

extern int8_t TextBTable[256][4];

void _FASTCALL_ str_hex_b(_WString* s, unsigned int x);
void _FASTCALL_ str_code_hb(_WString* s, unsigned int x);
void _FASTCALL_ str_code_hdw(_WString* s, uint32_t x);
void _FASTCALL_ str_code_hqw(_WString* s, uint8_t src[8]);

#ifdef SUPPORT_64BIT_OFFSET
void _FASTCALL_ str_off64(_WString* s, OFFSET_INTEGER x);
#endif

#endif /* DISTORM_LIGHT */

#endif /* TEXTDEFS_H */
