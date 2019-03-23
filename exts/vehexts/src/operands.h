/*
operands.h

diStorm3 - Powerful disassembler for x86/AMD64
http://ragestorm.net/distorm/
distorm at gmail dot com
Copyright (C) 2003-2015 Gil Dabah

Licensed under the diStorm version 3.3 Commercial License, see
the terms and conditions of the license in the file 'LICENSE.TXT'.
*/


#ifndef OPERANDS_H
#define OPERANDS_H

#include "config.h"
#include "decoder.h"
#include "prefix.h"
#include "instructions.h"


extern uint32_t _REGISTERTORCLASS[];

int operands_extract(_CodeInfo* ci, _DInst* di, _InstInfo* ii,
                     _iflags instFlags, _OpType type, _OperandNumberType opNum,
                     unsigned int modrm, _PrefixState* ps, _DecodeType effOpSz,
                     _DecodeType effAdrSz, int* lockableInstruction);

#endif /* OPERANDS_H */
