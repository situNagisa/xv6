#pragma once

#include "./page.h"
#include "./environment.h"

NGS_BUILD_LIB_DETAIL_BEGIN

template<page_size Size>
constexpr auto page_round_floor(::nagisa::concepts::pointer auto address) noexcept
{
	static_assert(::std::has_single_bit(::std::to_underlying(Size)));
	using point_type = decltype(address);
	return ::std::bit_cast<point_type>(::std::bit_cast<::std::uintptr_t>(address) & ::nagisa::bits::mask<::std::countr_zero(::std::to_underlying(Size))>());
}

template<page_size Size>
constexpr auto page_round_ceil(::nagisa::concepts::pointer auto address) noexcept
{
	static_assert(::std::has_single_bit(::std::to_underlying(Size)));
	using point_type = decltype(address);
	return details::page_round_floor<Size>(::std::bit_cast<point_type>(
		::std::bit_cast<::std::uintptr_t>(address) + ::nagisa::bits::mask<::std::countr_zero(::std::to_underlying(Size))>()
	));
}

NGS_BUILD_LIB_DETAIL_END