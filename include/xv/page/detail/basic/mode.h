#pragma once

#include "./environment.h"

NAGISA_BUILD_LIB_DETAIL_BEGIN

enum class page_mode
{
	secondary,
	primary,
};

template<page_mode> struct page_trait;

template<>
struct page_trait<page_mode::primary>
{
	constexpr static auto mode = page_mode::primary;
	constexpr static auto size = ::nagisa::bits::literals::operator ""_mb(4);
	constexpr static auto page_bit_width = static_cast<::std::size_t>(::std::countr_zero(size));
	constexpr static auto table_bit_width = 0;
	constexpr static auto directory_bit_width = 10;
};

template<>
struct page_trait<page_mode::secondary>
{
	constexpr static auto mode = page_mode::secondary;
	constexpr static auto size = ::nagisa::bits::literals::operator ""_kb(4);
	constexpr static auto page_bit_width = static_cast<::std::size_t>(::std::countr_zero(size));
	constexpr static auto table_bit_width = 10;
	constexpr static auto directory_bit_width = 10;
};

NAGISA_BUILD_LIB_DETAIL_END