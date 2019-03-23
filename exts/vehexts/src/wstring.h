/*
wstring.h

diStorm3 - Powerful disassembler for x86/AMD64
http://ragestorm.net/distorm/
distorm at gmail dot com
Copyright (C) 2003-2015 Gil Dabah

Licensed under the diStorm version 3.3 Commercial License, see
the terms and conditions of the license in the file 'LICENSE.TXT'.
*/


#ifndef WSTRING_H
#define WSTRING_H

#include "config.h"

#ifndef DISTORM_LIGHT

void strclear_WS(_WString* s);
void chrcat_WS(_WString* s, uint8_t ch);
void strcpylen_WS(_WString* s, const int8_t* buf, unsigned int len);
void strcatlen_WS(_WString* s, const int8_t* buf, unsigned int len);
void strcat_WS(_WString* s, const _WString* s2);

/*
* Warning, this macro should be used only when the compiler knows the size of string in advance!
* This macro is used in order to spare the call to strlen when the strings are known already.
* Note: sizeof includes NULL terminated character.
*/
#define strcat_WSN(s, t) strcatlen_WS((s), ((const int8_t*)t), sizeof((t))-1)
#define strcpy_WSN(s, t) strcpylen_WS((s), ((const int8_t*)t), sizeof((t))-1)

#endif /* DISTORM_LIGHT */

#endif /* WSTRING_H */
