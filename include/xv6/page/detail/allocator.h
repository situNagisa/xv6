#pragma once

#include "./page.h"
#include "./environment.h"

NAGISA_BUILD_LIB_DETAIL_BEGIN

template<::xv::pages::page_mode Mode>
constexpr auto&& get_next_page(page<Mode> const& current) noexcept
{
	constexpr auto ptr_size = sizeof(::std::uintptr_t);
	::fast_io::array<::std::byte, ptr_size> next{};
	::fast_io::freestanding::copy_n(::std::ranges::begin(current), ptr_size, ::std::ranges::begin(next));
	return *::std::bit_cast<page<Mode>*>(next);
}
template<::xv::pages::page_mode Mode>
constexpr auto set_next_page(page<Mode>& current, page<Mode>& next) noexcept
{
	constexpr auto ptr_size = sizeof(::std::uintptr_t);
	auto address = ::std::bit_cast<::fast_io::array<::std::byte, ptr_size>>(::std::ranges::data(next));
	::fast_io::freestanding::copy_n(::std::ranges::begin(address), ptr_size, ::std::ranges::begin(current));
}

template<::xv::pages::page_mode Mode>
struct free_page_iterator : ::boost::stl_interfaces::iterator_interface<free_page_iterator<Mode>, ::std::forward_iterator_tag, page<Mode>>
{
	using self_type = free_page_iterator;
	using base_type = ::boost::stl_interfaces::iterator_interface<free_page_iterator<Mode>, ::std::forward_iterator_tag, page<Mode>>;
	using page_type = page<Mode>;

	constexpr free_page_iterator() noexcept = default;
	constexpr explicit(false) free_page_iterator(page_type* page) noexcept : _page(page) {}

	constexpr decltype(auto) operator*() const noexcept { return *_page; }
	decltype(auto) operator++() noexcept
	{
		_page = ::std::addressof(details::get_next_page<Mode>(*_page));
		return *this;
	}
	using base_type::operator++;
	constexpr bool operator==(const self_type& other) const noexcept { return _page == other._page; }

	page_type* _page;
};

template<::xv::pages::page_mode Mode>
struct free_page_list : ::std::ranges::view_interface<free_page_list<Mode>>
{
	using self_type = free_page_list;
	using page_type = page<Mode>;

	using iterator_type = free_page_iterator<Mode>;

	constexpr void push_front(page_type& value) noexcept
	{
		details::set_next_page<Mode>(value, *_data);
		_data = ::std::addressof(value);
	}
	constexpr auto&& pop_front() noexcept
	{
		auto next = ::std::addressof(details::get_next_page<Mode>(*_data));
		::std::swap(_data, next);

		return *next;
	}
	constexpr bool empty() const noexcept { return _data == nullptr; }

	constexpr auto begin() const noexcept { return iterator_type(_data); }
	constexpr auto end() const noexcept { return iterator_type(nullptr); }

	page_type* _data = nullptr;
};


struct page_allocator
{
	using list_type = free_page_list<::xv::pages::page_mode::secondary>;
	using page_type = typename list_type::page_type;

	void reserve_range(::std::ranges::input_range auto const& pages) noexcept
		requires ::std::same_as<::std::ranges::range_value_t<decltype(pages)>, page_type>
	{
		for (auto&& page : pages)
			_free_list.push_front(page);
	}

	constexpr bool empty() const noexcept
	{
		return _free_list.empty();
	}

	auto&& allocate() noexcept
	{
		if (empty())
			::fast_io::fast_terminate();
		auto&& result = _free_list.pop_front();
		return result;
	}

	void deallocate(page_type& page) noexcept
	{
		_free_list.push_front(page);
	}

	list_type _free_list{};
};

NAGISA_BUILD_LIB_DETAIL_END