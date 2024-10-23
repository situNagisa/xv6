#pragma once


#include "./basic/mode.h"
#include "./basic/address_iterate.h"
#include "./basic/table.h"
#include "./basic/directory.h"
#include "./environment.h"

#if __cpp_lib_ranges < 202202l
#	erorr "requires c++ 23 ::std::ranges::range_adaptor_closure"
#endif


#if __cpp_static_call_operator >= 202207L
#	define XV_PAGE_STATIC static
#	define XV_PAGE_CONST 
#else
#	define XV_PAGE_STATIC 
#	define XV_PAGE_CONST const
#endif

NAGISA_BUILD_LIB_DETAIL_BEGIN

template<class View, class Value, class Difference>
struct flattened_iterator : ::boost::stl_interfaces::v2::iterator_interface<
	flattened_iterator<View, Value, Difference>,
	::std::random_access_iterator_tag,
	Value, Value&, Value*, Difference
	>
{
private:
	using self_type = flattened_iterator;
	using base_type = ::boost::stl_interfaces::v2::iterator_interface<
		flattened_iterator<View, Value, Difference>,
		::std::random_access_iterator_tag,
		Value, Value&, Value*, Difference
		>;
public:
	using view_type = View;

	constexpr flattened_iterator(view_type& view, ::std::uintptr_t index) noexcept
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

template<secondary_page_directory Directory>
	requires ::std::ranges::view<Directory> && address_iterable<Directory> && address_iterable<target_t<::std::ranges::range_reference_t<Directory>>>
struct flatten_view : ::std::ranges::view_interface<flatten_view<Directory>>
{
private:
	using self_type = flatten_view;
public:
	using directory_type = Directory;
	using table_type = target_t<::std::ranges::range_reference_t<directory_type>>;
	using iterator_type = flattened_iterator<
		self_type,
		::std::remove_reference_t<::std::ranges::range_reference_t<table_type>>,
		::std::intptr_t // TODO
		>;
	constexpr static auto mode = page_mode::secondary;

	constexpr explicit flatten_view(directory_type directory) noexcept
		: _directory(::std::move(directory))
	{}

	constexpr auto&& operator[](::std::uintptr_t index) const noexcept
	{
		auto address = ::std::bit_cast<void const*>(index << page_trait<mode>::page_bit_width);
		return *details::iterator(details::target(*details::iterator(_directory, address)), address);
	}

	constexpr auto iterator(void const* address) const noexcept
	{
		if (::std::bit_cast<::std::uintptr_t>(address) & ::nagisa::bits::mask<::std::uintptr_t, page_trait<mode>::page_bit_width>())
			::fast_io::fast_terminate();
		auto index = ::std::bit_cast<::std::uintptr_t>(address) >> page_trait<page_mode::secondary>::page_bit_width;
		return iterator_type(_directory, index);
	}
	constexpr auto sentinel(void const* address) const noexcept
	{
		return self_type::iterator(address);
	}

	directory_type _directory;
};

inline constexpr struct flatten_directory_cpo
{
	struct adaptor_closure : ::std::ranges::range_adaptor_closure<adaptor_closure>
	{
		constexpr auto operator()(::std::ranges::viewable_range auto&& range) const noexcept
			requires secondary_page_directory<decltype(range)>
				&& address_iterable<decltype(range)>
				&& address_iterable<target_t<::std::ranges::range_reference_t<decltype(range)>>>
		{
			return flatten_view<::std::views::all_t<decltype(range)>>(::std::views::all(NAGISA_STL_FREESTANDING_UTILITY_FORWARD(range)));
		}
	};

	constexpr XV_PAGE_STATIC decltype(auto) operator()(auto&& range) XV_PAGE_CONST noexcept
		requires requires{ adaptor_closure{}(NAGISA_STL_FREESTANDING_UTILITY_FORWARD(range)); }
	{
		return adaptor_closure{}(NAGISA_STL_FREESTANDING_UTILITY_FORWARD(range));
	}
}flatten{};

NAGISA_BUILD_LIB_DETAIL_END

#undef XV_PAGE_STATIC
#undef XV_PAGE_CONST