#pragma once

#include "./basic/directory.h"
#include "./basic/address_iterate.h"
#include "./environment.h"

#if __cpp_static_call_operator >= 202207L
#	define XV_PAGE_STATIC static
#	define XV_PAGE_CONST 
#else
#	define XV_PAGE_STATIC 
#	define XV_PAGE_CONST const
#endif

NAGISA_BUILD_LIB_DETAIL_BEGIN


template<class View, class Value, class Difference>
struct chunk_by_address_iterator : ::boost::stl_interfaces::v2::iterator_interface<
	chunk_by_address_iterator<View, Value, Difference>,
	::std::random_access_iterator_tag,
	Value, Value&, Value*, Difference
>
{
private:
	using self_type = chunk_by_address_iterator;
	using base_type = ::boost::stl_interfaces::v2::iterator_interface<
		chunk_by_address_iterator<View, Value, Difference>,
		::std::random_access_iterator_tag,
		Value, Value&, Value*, Difference
	>;
public:
	using view_type = View;

	constexpr chunk_by_address_iterator(view_type& view, ::std::uintptr_t index) noexcept
		: _view(::std::addressof(view))
		, _index(index)
	{
	}

	constexpr auto&& operator*() const noexcept { return (*_view)[_index]; }
	constexpr decltype(auto) operator-(self_type const& other) const noexcept { return static_cast<typename base_type::difference_type>(_index - other._index); }
	using base_type::operator+=;
	constexpr auto&& operator+=(typename base_type::difference_type n) noexcept
	{
		_index += n;
		return *this;
	}

	view_type* _view;
	::std::uintptr_t _index;
};

template<address_iterable Range, ::std::ranges::input_range Sizes>
	requires entry_range<Range> && ::std::ranges::sized_range<Sizes> && ::std::convertible_to<::std::ranges::range_value_t<Sizes>, ::std::size_t const&>
struct chunk_by_address_view : ::std::ranges::view_interface<chunk_by_address_view<Range, Sizes>>
{
private:
	using self_type = chunk_by_address_view;
public:
	using range_type = Range;
	using sizes_type = Sizes;
	using iterator_type = chunk_by_address_iterator<self_type, address_range_view<range_type>,
		::std::intptr_t // TODO
	>;
	constexpr static auto mode = entry_mode_v<::std::ranges::range_value_t<Range>>;

	constexpr explicit chunk_by_address_view(range_type& range, ::std::convertible_to<sizes_type const&> auto&& sizes, void const* address) noexcept
		: _range(::std::addressof(range))
		, _sizes(NAGISA_STL_FREESTANDING_UTILITY_FORWARD(sizes))
		, _address(address)
	{
	}

	constexpr auto&& operator[](::std::uintptr_t index) const noexcept
	{
		namespace layouts = ::nagisa::memmod::layouts;
		return address_range(
			*_range, 
			layouts::offset_of(_sizes, index), 
			layouts::offset_of(_sizes, index + 1)
		);
	}
	constexpr auto begin() const noexcept { return iterator_type{ *this, 0 }; }
	constexpr auto end() const noexcept { return iterator_type{ *this, ::std::ranges::size(_sizes) }; }

	range_type* _range;
	sizes_type _sizes;
	void const* _address;
};

inline constexpr struct chunk_by_address_cpo
{
	template<::std::ranges::input_range Sizes>
		requires ::std::ranges::sized_range<Sizes> && ::std::convertible_to<::std::ranges::range_value_t<Sizes>, ::std::size_t const&>
	struct adaptor_closure : ::std::ranges::range_adaptor_closure<adaptor_closure<Sizes>>
	{
		constexpr explicit(false) adaptor_closure(Sizes&& sizes, void const* address) noexcept
			: _sizes(NAGISA_STL_FREESTANDING_UTILITY_FORWARD(sizes))
			, _address(address)
		{}

		constexpr auto operator()(address_iterable auto&& range) const noexcept
			requires ::nagisa::concepts::object<decltype(range)> && entry_range<decltype(range)>
		{
			return chunk_by_address_view<::std::remove_reference_t<decltype(range)>, ::std::remove_reference_t<Sizes>>
				(NAGISA_STL_FREESTANDING_UTILITY_FORWARD(range), _sizes, _address);
		}

		Sizes _sizes;
		void const* _address;
	};

	constexpr XV_PAGE_STATIC decltype(auto) operator()(auto&& sizes, void const* address) XV_PAGE_CONST noexcept
		requires requires{ adaptor_closure<decltype(sizes)>{NAGISA_STL_FREESTANDING_UTILITY_FORWARD(sizes), address}; }
	{
		return adaptor_closure<decltype(sizes)>{NAGISA_STL_FREESTANDING_UTILITY_FORWARD(sizes), address};
	}

	constexpr XV_PAGE_STATIC decltype(auto) operator()(auto&& range, auto&& sizes, void const* address) XV_PAGE_CONST noexcept
		requires requires{
		chunk_by_address_view<::std::remove_reference_t<decltype(range)>, decltype(sizes)>{
			NAGISA_STL_FREESTANDING_UTILITY_FORWARD(range), NAGISA_STL_FREESTANDING_UTILITY_FORWARD(sizes), address
		};}
	{
		return chunk_by_address_view<::std::remove_reference_t<decltype(range)>, decltype(sizes)>{
			NAGISA_STL_FREESTANDING_UTILITY_FORWARD(range), NAGISA_STL_FREESTANDING_UTILITY_FORWARD(sizes), address
		};
	}
}chunk_by_address{};


NAGISA_BUILD_LIB_DETAIL_END

#undef XV_PAGE_STATIC
#undef XV_PAGE_CONST