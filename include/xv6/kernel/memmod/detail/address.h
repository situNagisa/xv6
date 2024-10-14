#pragma once


#include "./environment.h"

NGS_BUILD_LIB_DETAIL_BEGIN

static_assert(sizeof(::std::uint_least32_t) == 4);
static_assert(sizeof(::std::uintptr_t) == 4);

template<class T, class U>
concept address_visual_to_physical_transformer = 
	::std::invocable<T, const U*>
	&& ::std::convertible_to<::std::invoke_result_t<T, const U*>, ::std::uintptr_t>
	;

template<class T, class U>
concept address_physical_to_visual_transformer = 
	::std::invocable<T, ::std::uintptr_t>
	&& ::std::convertible_to<::std::invoke_result_t<T, ::std::uintptr_t>, const U*>
	;

NGS_BUILD_LIB_DETAIL_END