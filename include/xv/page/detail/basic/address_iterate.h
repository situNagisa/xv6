#pragma once

#include "./environment.h"

#if __cpp_static_call_operator >= 202207L
#	define XV_PAGE_STATIC static
#	define XV_PAGE_CONST 
#else
#	define XV_PAGE_STATIC 
#	define XV_PAGE_CONST const
#endif

NAGISA_BUILD_LIB_DETAIL_BEGIN

namespace iterator_details
{
	template<class T>
	concept has_member = requires(T t, void const* address) { { NAGISA_STL_FREESTANDING_UTILITY_FORWARD(t).iterator(address) }; };
	template<class T>
	concept has_adl = requires(T t, void const* address) { { iterator(NAGISA_STL_FREESTANDING_UTILITY_FORWARD(t), address) }; };
}
inline constexpr struct
{
	constexpr XV_PAGE_STATIC decltype(auto) operator()(auto&& range, void const* address) XV_PAGE_CONST noexcept
		requires iterator_details::has_member<decltype(range)> || iterator_details::has_adl<decltype(range)>
	{
		if constexpr (iterator_details::has_member<decltype(range)>)
			return NAGISA_STL_FREESTANDING_UTILITY_FORWARD(range).iterator(address);
		else if constexpr (iterator_details::has_adl<decltype(range)>)
			return iterator(NAGISA_STL_FREESTANDING_UTILITY_FORWARD(range), address);
	}
}iterator{};
template<class T>
using iterator_t = decltype(iterator(::std::declval<T&>(), ::std::declval<void const*>()));

namespace sentinel_details
{
	template<class T>
	concept has_member = requires(T t, void const* address) { { NAGISA_STL_FREESTANDING_UTILITY_FORWARD(t).sentinel(address) }; };
	template<class T>
	concept has_adl = requires(T t, void const* address) { { sentinel(NAGISA_STL_FREESTANDING_UTILITY_FORWARD(t), address) }; };
}
inline constexpr struct
{
	constexpr XV_PAGE_STATIC decltype(auto) operator()(auto&& range, void const* address) XV_PAGE_CONST noexcept
		requires sentinel_details::has_member<decltype(range)> || sentinel_details::has_adl<decltype(range)>
	{
		if constexpr (sentinel_details::has_member<decltype(range)>)
			return NAGISA_STL_FREESTANDING_UTILITY_FORWARD(range).sentinel(address);
		else if constexpr (sentinel_details::has_adl<decltype(range)>)
			return sentinel(NAGISA_STL_FREESTANDING_UTILITY_FORWARD(range), address);
	}
}sentinel{};
template<class T>
using sentinel_t = decltype(sentinel(::std::declval<T&>(), ::std::declval<void const*>()));

template<class T>
concept address_iterable = requires(T t, void const* address)
{
	iterator(t, address);
	sentinel(t, address);
};

struct address_range_adaptor_closure : ::std::ranges::range_adaptor_closure<address_range_adaptor_closure>
{
	constexpr address_range_adaptor_closure(void const* i, void const* s) noexcept : _iterator(i), _sentinel(s) {}

	constexpr decltype(auto) operator()(address_iterable auto&& range) const noexcept
	{
		return ::std::ranges::subrange(
			iterator(NAGISA_STL_FREESTANDING_UTILITY_FORWARD(range), _iterator),
			sentinel(NAGISA_STL_FREESTANDING_UTILITY_FORWARD(range), _sentinel)
		);
	}

	void const* _iterator;
	void const* _sentinel;
};

inline constexpr struct
{
	constexpr XV_PAGE_STATIC decltype(auto) operator()(address_iterable auto&& range, void const* begin, void const* end) XV_PAGE_CONST noexcept
	{
		return address_range_adaptor_closure{ begin, end }(NAGISA_STL_FREESTANDING_UTILITY_FORWARD(range));
	}
	constexpr XV_PAGE_STATIC decltype(auto) operator()(void const* begin, void const* end) XV_PAGE_CONST noexcept
	{
		return address_range_adaptor_closure{ begin, end };
	}
}address_range{};

template<class T>
using address_range_t = decltype(address_range(::std::declval<T&>(), ::std::declval<void const*>(), ::std::declval<void const*>()));

NAGISA_BUILD_LIB_DETAIL_END

#undef XV_PAGE_STATIC
#undef XV_PAGE_CONST