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
static uint64_t decode_uleb128(const uint8_t* p, unsigned* n = nullptr)
{
	auto orig_p = p;
	uint64_t value = 0;
	unsigned shift = 0;
	do
	{
		value += uint64_t(*p & 0x7f) << shift;
		shift += 7;
	}
	while (*p++ >= 128);

	return value;
}

#endif // LLVM_SYSTEM_LEB128_H

static unsigned encode_uleb128(uint64_t value, std::ostream& os,
                               const unsigned pad_to = 0)
{
	unsigned count = 0;
	do
	{
		uint8_t byte = value & 0x7f;
		value >>= 7;
		count++;
		if (value != 0 || count < pad_to)
			byte |= 0x80; // Mark this byte to show that more bytes will follow.
		os << static_cast<char>(byte);
	}
	while (value != 0);

	// Pad with 0x80 and emit a null byte at the end.
	if (count < pad_to)
	{
		for (; count < pad_to - 1; ++count)
			os << '\x80';
		os << '\x00';
		count++;
	}
	return count;
}
