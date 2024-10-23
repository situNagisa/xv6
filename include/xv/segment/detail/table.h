#pragma once

#include "./descriptor.h"
#include "./environment.h"

NAGISA_BUILD_LIB_DETAIL_BEGIN

template<class T>
constexpr void segment_descriptor_implicit_convertible(segment_descriptor<T> const&) noexcept {}

template<class T>
concept derived_from_segment_descriptor = 
	requires(T t) { details::segment_descriptor_implicit_convertible(::std::forward<T>(t)); }
	&& ::nagisa::concepts::object<T>
	&& (sizeof(T) == segment_descriptor_size)
	;

using segment_descriptor_bytearray_type = ::fast_io::array<::std::byte, segment_descriptor_size>;

template<derived_from_segment_descriptor... T>
struct segment_descriptor_table : ::fast_io::array<segment_descriptor_bytearray_type, sizeof...(T)>
{
	using self_type = segment_descriptor_table;
	using base_type = ::fast_io::array<segment_descriptor_bytearray_type, sizeof...(T)>;


	constexpr segment_descriptor_table() noexcept = default;
	constexpr explicit(false) segment_descriptor_table(T const&... t) noexcept requires (sizeof...(T) > 0)
		: base_type{ ::std::bit_cast<segment_descriptor_bytearray_type>(t)... }
	{
	}

#if __cpp_pack_indexing < 202311l
	using _tuple_type = ::nagisa::stl_freestanding::tuples::static_tuple<T...>;
#endif

	template<::std::size_t Index>
		requires (Index < sizeof...(T))
	constexpr auto get() const noexcept
	{
#if __cpp_pack_indexing >= 202311l
		return ::std::bit_cast<T...[Index]>(base_type::index_unchecked(Index));
#else
		return ::std::bit_cast<::nagisa::stl_freestanding::tuples::tuple_element_t<Index, _tuple_type>>(base_type::index_unchecked(Index));
#endif
	}

	template<::std::size_t Index>
		requires (Index < sizeof...(T))
#if __cpp_pack_indexing >= 202311l
	constexpr void set(T...[Index] const& t) noexcept
#else
	constexpr void set(::nagisa::stl_freestanding::tuples::tuple_element_t<Index, _tuple_type> const& t) noexcept
#endif
	{
		base_type::index_unchecked(Index) = ::std::bit_cast<segment_descriptor_bytearray_type>(t);
	}
};

template<derived_from_segment_descriptor... T>
segment_descriptor_table(T const&...) -> segment_descriptor_table<T...>;

NAGISA_BUILD_LIB_DETAIL_END