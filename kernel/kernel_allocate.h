#pragma once

#include "xv/page.h"
#include "xv6/page/page.h"
#include "defs.h"

inline struct xv6_allocator : ::xv6::pages::page_allocator
{
	using self_type = ::xv6::pages::page_allocator;
	using base_type = ::xv6::pages::page_allocator;

	xv6_allocator() noexcept
	{
		::initlock(&_lock, "kernel page allocator");
	}

	void reserve_range(::std::ranges::input_range auto const& pages) noexcept
		requires ::std::same_as<::std::ranges::range_value_t<decltype(pages)>, page_type>
	{
		if (_use_lock)
			::acquire(&_lock);
		base_type::reserve_range(pages);
		if (_use_lock)
			::release(&_lock);
	}

	decltype(auto) empty() noexcept
	{
		if (_use_lock)
			::acquire(&_lock);
		auto result = base_type::empty();
		if (_use_lock)
			::release(&_lock);
		return result;
	}

	auto&& allocate() noexcept
	{
		if (_use_lock)
			::acquire(&_lock);
		auto&& result = base_type::allocate();
		if (_use_lock)
			::release(&_lock);
		return result;
	}
	void deallocate(page_type& page) noexcept
	{
		if (_use_lock)
			::acquire(&_lock);
		base_type::deallocate(page);
		if (_use_lock)
			::release(&_lock);
	}

	spinlock _lock{};
	bool _use_lock = false;
}kernel_memory{};