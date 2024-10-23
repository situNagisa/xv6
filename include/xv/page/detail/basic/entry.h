#pragma once

#include "./mode.h"
#include "./environment.h"

#if __cpp_static_call_operator >= 202207L
#	define XV_PAGE_STATIC static
#	define XV_PAGE_CONST 
#else
#	define XV_PAGE_STATIC 
#	define XV_PAGE_CONST const
#endif

NAGISA_BUILD_LIB_DETAIL_BEGIN

namespace target_details
{
	template<class T>
	concept has_member = requires(T t) { { NAGISA_STL_FREESTANDING_UTILITY_FORWARD(t).target() }; };
	template<class T>
	concept has_adl = requires(T t) { { target(NAGISA_STL_FREESTANDING_UTILITY_FORWARD(t)) }; };
}
inline constexpr struct
{
	constexpr XV_PAGE_STATIC decltype(auto) operator()(auto&& entry) XV_PAGE_CONST noexcept
		requires target_details::has_member<decltype(entry)> || target_details::has_adl<decltype(entry)>
	{
		if constexpr (target_details::has_member<decltype(entry)>)
			return NAGISA_STL_FREESTANDING_UTILITY_FORWARD(entry).target();
		else if constexpr (target_details::has_adl<decltype(entry)>)
			return target(NAGISA_STL_FREESTANDING_UTILITY_FORWARD(entry));
	}
}target{};
template<class T>
using target_t = decltype(target(::std::declval<T&>()));

template<class T>
struct entry_mode { constexpr static auto value = ::std::remove_reference_t<T>::mode; };
template<class T>
	requires requires { entry_mode<T>::value; }
inline constexpr auto entry_mode_v = entry_mode<T>::value;

template<class T>
concept entry = true
	&& ::std::is_standard_layout_v<T>
	&& ::std::is_trivially_copyable_v<T>
	&& ::std::is_trivially_destructible_v<T>
	&& ::std::is_object_v<T>
	&& (sizeof(T) == sizeof(::std::uintptr_t))
	&& requires(T t) { target(NAGISA_STL_FREESTANDING_UTILITY_FORWARD(t)); }
	&& requires(T t) { { entry_mode_v<T> } -> ::std::convertible_to<page_mode const&>; }
	;

template<class T>
concept entry_range = ::std::ranges::range<T> && entry<::std::ranges::range_value_t<T>>;

NAGISA_BUILD_LIB_DETAIL_END

#undef XV_PAGE_STATIC
#undef XV_PAGE_CONST