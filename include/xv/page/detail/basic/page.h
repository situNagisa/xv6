#pragma once

#include "./mode.h"
#include "./environment.h"

NAGISA_BUILD_LIB_DETAIL_BEGIN

template<class T, page_mode Mode>
concept page = true
	&& ::std::ranges::contiguous_range<T>
	&& ::std::convertible_to<::std::ranges::range_reference_t<T>, ::std::byte&>
	&& requires(T&& t) { requires (::std::ranges::size(t) == page_trait<Mode>::size); }
	;

template<class T>
concept secondary_page = page<T, page_mode::secondary>;

template<class T>
concept primary_page = page<T, page_mode::primary>;

NAGISA_BUILD_LIB_DETAIL_END