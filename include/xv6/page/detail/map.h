#pragma once

#include "./environment.h"

NAGISA_BUILD_LIB_DETAIL_BEGIN

template<::std::size_t N>
struct address_mapper
{
private:
	using self_type = address_mapper;
public:
	constexpr address_mapper() noexcept = default;
	constexpr address_mapper(
		::std::size_t visual, 
		::std::size_t physical, 
		::fast_io::index_span<::std::size_t, N> sizes
	) noexcept
		: _visual(visual)
		, _physical(physical)
		, _sizes()
	{
		::fast_io::freestanding::copy(sizes.begin(), sizes.end(), self_type::_sizes.begin());
	}
	constexpr address_mapper(
		::std::size_t visual,
		::std::size_t physical,
		::fast_io::array<::std::size_t, N>&& sizes
	) noexcept
		: _visual(visual)
		, _physical(physical)
		, _sizes(::std::move(sizes))
	{
	}
	constexpr address_mapper(
		::std::size_t visual,
		::std::size_t physical,
		::std::convertible_to<::std::size_t> auto... sizes
	) noexcept
		: _visual(visual)
		, _physical(physical)
		, _sizes{ static_cast<::std::size_t>(sizes)... }
	{
	}

	::std::uintptr_t _visual;
	::std::uintptr_t _physical;
	::fast_io::array<::std::size_t, N> _sizes;

	constexpr auto size_bytes() const noexcept
	{
		return ::std::ranges::fold_left(_sizes, static_cast<::std::size_t>(0), ::std::plus{});
	}

	constexpr auto visual_to_physical(::std::uintptr_t address) const noexcept
	{
		return static_cast<::std::uintptr_t>(address - _visual + _physical);
	}
	constexpr decltype(auto) visual_to_physical(::nagisa::concepts::pointer auto address) const noexcept
	{
		return self_type::visual_to_physical(::std::bit_cast<::std::uintptr_t>(address));
	}
	constexpr auto physical_to_visual(::std::uintptr_t address) const noexcept
	{
		return static_cast<::std::uintptr_t>(address - _physical + _visual);
	}
	template<class T>
	constexpr auto physical_to_visual(::std::uintptr_t address) const noexcept
	{
		return ::std::bit_cast<T*>(self_type::physical_to_visual(address));
	}
};

template<::std::convertible_to<::std::size_t>... T>
address_mapper(::std::uintptr_t, ::std::uintptr_t, T...) -> address_mapper<sizeof...(T)>;

namespace layouts
{
	extern "C" char data[];  // defined by kernel.ld

	constexpr auto data_address() noexcept
	{
		return ::std::bit_cast<::std::uintptr_t>(static_cast<char const*>(data));
	}

	inline constexpr auto kernel = address_mapper{
		static_cast<::std::uintptr_t>(0x8000'0000),
		static_cast<::std::uintptr_t>(0x0000'0000),
		0x10'0000u,
		layouts::data_address() - 0x8010'0000,
		0x8e00'0000u - layouts::data_address()
	};
	inline constexpr auto device = address_mapper{
		static_cast<::std::uintptr_t>(0xfe00'0000),
		static_cast<::std::uintptr_t>(0xfe00'0000),
		0x0000'0000u - 0xfe00'0000
	};
}

NAGISA_BUILD_LIB_DETAIL_END