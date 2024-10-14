#pragma once

#include "../bit_literal.h"
#include "./environment.h"

NGS_BUILD_LIB_DETAIL_BEGIN

enum class page_size : ::std::size_t
{
	_4kb = literals::operator""_kb(4),
	_4mb = literals::operator""_mb(4),
};

template<page_size Size>
using page = ::fast_io::array<::std::byte, ::std::to_underlying(Size)>;

using page_4_kb = page<page_size::_4kb>;
using page_4_mb = page<page_size::_4mb>;

NGS_BUILD_LIB_DETAIL_END