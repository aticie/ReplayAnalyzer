#pragma once
#include <cstdint>
//===- llvm/Support/LEB128.h - [SU]LEB128 utility functions -----*- C++ -*-===//
 //
 // Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
 // See https://llvm.org/LICENSE.txt for license information.
 // SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
 //
 //===----------------------------------------------------------------------===//
 //
 // This file declares some utility functions for encoding SLEB128 and
 // ULEB128 values.
 //
 //===----------------------------------------------------------------------===//

#ifndef LLVM_SUPPORT_LEB128_H
#define LLVM_SUPPORT_LEB128_H

/// Utility function to decode a ULEB128 value.
static uint64_t decodeULEB128(const uint8_t* p, unsigned* n = nullptr)
{
	const uint8_t* orig_p = p;
	uint64_t Value = 0;
	unsigned Shift = 0;
	do
	{
		Value += uint64_t(*p & 0x7f) << Shift;
		Shift += 7;
	} while (*p++ >= 128);

	return Value;
}

#endif // LLVM_SYSTEM_LEB128_H

static unsigned encodeULEB128(uint64_t Value, std::ostream& OS,
    unsigned PadTo = 0) {
    unsigned Count = 0;
    do {
        uint8_t Byte = Value & 0x7f;
        Value >>= 7;
        Count++;
        if (Value != 0 || Count < PadTo)
            Byte |= 0x80; // Mark this byte to show that more bytes will follow.
        OS << char(Byte);
    } while (Value != 0);

    // Pad with 0x80 and emit a null byte at the end.
    if (Count < PadTo) {
        for (; Count < PadTo - 1; ++Count)
            OS << '\x80';
        OS << '\x00';
        Count++;
    }
    return Count;
}