/*
decoder.h

diStorm3 - Powerful disassembler for x86/AMD64
http://ragestorm.net/distorm/
distorm at gmail dot com
Copyright (C) 2003-2015 Gil Dabah

Licensed under the diStorm version 3.3 Commercial License, see
the terms and conditions of the license in the file 'LICENSE.TXT'.
*/


#ifndef DECODER_H
#define DECODER_H

#include "config.h"

typedef unsigned int _iflags;

_DecodeResult decode_internal(_CodeInfo* ci, int supportOldIntr, _DInst result[], unsigned int maxResultCount, unsigned int* usedInstructionsCount);

#endif /* DECODER_H */
