#include <nagisa/concept/concept.h>
#include <nagisa/bit/bit.h>
#include "xv6/kernel/memmod.h"
#include <ranges>
#include <fast_io.h>
#include <fast_io_dsal/span.h>
#include <boost/stl_interfaces/iterator_interface.hpp>
#include "memlayout.h"
#include "spinlock.h"
#include "defs.h"
#include "mmu.h"

namespace xv6_memmod = ::XV6_KERNEL_MEMMOD_NS;

struct address_transformer_pair
{
	constexpr static auto visual_to_physical(::nagisa::concepts::pointer auto address) noexcept
	{
		return static_cast<::std::uintptr_t>(::std::bit_cast<::std::uintptr_t>(address) - KERNBASE);
	}
	template<class T>
	constexpr static auto physical_to_visual(::std::uintptr_t address) noexcept
	{
		return ::std::bit_cast<T*>(static_cast<::std::uintptr_t>(address + KERNBASE));
	}
};

namespace
{
	template<::xv6_memmod::page_size Size>
	constexpr auto&& get_next_page(::xv6_memmod::page<Size> const& page) noexcept
	{
		constexpr auto ptr_size = sizeof(::std::uintptr_t);
		::fast_io::array<::std::byte, ptr_size> next{};
		::fast_io::freestanding::copy_n(::std::ranges::begin(page), ptr_size, ::std::ranges::begin(next));
		return *::std::bit_cast<::xv6_memmod::page<Size>*>(next);
	}
	template<::xv6_memmod::page_size Size>
	constexpr auto set_next_page(::xv6_memmod::page<Size>& page, ::xv6_memmod::page<Size>& next) noexcept
	{
		constexpr auto ptr_size = sizeof(::std::uintptr_t);
		auto address = ::std::bit_cast<::fast_io::array<::std::byte, ptr_size>>(::std::addressof(next));
		::fast_io::freestanding::copy_n(::std::ranges::begin(address), ptr_size, ::std::ranges::begin(page));
	}
}

template<::xv6_memmod::page_size Size>
struct free_page_iterator : ::boost::stl_interfaces::iterator_interface<free_page_iterator<Size>, ::std::forward_iterator_tag, ::xv6_memmod::page<Size>>
{
	using self_type = free_page_iterator;
	using base_type = ::boost::stl_interfaces::iterator_interface<free_page_iterator<Size>, ::std::forward_iterator_tag, ::xv6_memmod::page<Size>>;
	using page_type = ::xv6_memmod::page<Size>;

	constexpr free_page_iterator() noexcept = default;
	constexpr explicit(false) free_page_iterator(page_type* page) noexcept : _page(page) {}

	constexpr decltype(auto) operator*() const noexcept { return *_page; }
	decltype(auto) operator++() noexcept
	{
		_page = ::std::addressof(::get_next_page<Size>(*_page));
		return *this;
	}
	using base_type::operator++;
	constexpr bool operator==(const self_type& other) const noexcept { return _page == other._page; }

	page_type* _page;
};

template<::xv6_memmod::page_size Size>
struct free_page_list : ::std::ranges::view_interface<free_page_list<Size>>
{
	using self_type = free_page_iterator;
	using page_type = ::xv6_memmod::page<Size>;

	using const_iterator_type = free_page_iterator<Size>;

	constexpr void push_front(page_type& value) noexcept
	{
		::set_next_page<Size>(value, *_data);
		_data = ::std::addressof(value);
	}
	constexpr auto&& pop_front() noexcept
	{
		auto&& next = ::get_next_page<Size>(*_data);
		::std::swap(_data, next);

		return next;
	}
	constexpr bool empty() const noexcept { return _data == nullptr; }

	constexpr auto begin() const noexcept { return const_iterator_type(_data); }
	constexpr auto end() const noexcept { return const_iterator_type(nullptr); }

	page_type* _data = nullptr;
};

struct allocator
{
	using list_type = free_page_list<::xv6_memmod::page_size::_4kb>;
	using page_type = typename list_type::page_type;

	allocator() noexcept
	{
		::initlock(&_lock, "kernel page allocator");
	}

	void reserve_range(::std::ranges::input_range auto const& pages) noexcept
		requires ::std::same_as<::std::ranges::range_value_t<decltype(pages)>, page_type>
	{
		for (auto&& page : pages)
			_free_list.push_front(page);
	}

	auto&& allocate() noexcept
	{
		if (_use_lock)
			::acquire(&_lock);
		if (_free_list.empty())
			::fast_io::fast_terminate();
		auto&& result = _free_list.pop_front();
		if (_use_lock)
			::release(&_lock);
		return result;
	}
	void deallocate(page_type& page) noexcept
	{
		if (_use_lock)
			::acquire(&_lock);
		_free_list.push_front(page);
		if (_use_lock)
			::release(&_lock);
	}

	spinlock _lock{};
	bool _use_lock = false;
	list_type _free_list{};
};

static allocator kmem{};

// Initialization happens in two phases.
// 1. main() calls kinit1() while still using entrypgdir to place just
// the pages mapped by entrypgdir on free list.
// 2. main() calls kinit2() with the rest of the physical pages
// after installing a full page table that maps them on all cores.
extern "C" void kinit1(void* vstart, void* vend)
{
	kmem.reserve_range(::fast_io::span(
		reinterpret_cast<::xv6_memmod::page_4_kb*>(::xv6_memmod::page_round_ceil<::xv6_memmod::page_size::_4kb>(vstart)),
		reinterpret_cast<::xv6_memmod::page_4_kb*>(::xv6_memmod::page_round_ceil<::xv6_memmod::page_size::_4kb>(vend))
	));
}

extern "C" void kinit2(void* vstart, void* vend)
{
	kmem.reserve_range(::fast_io::span(
		reinterpret_cast<::xv6_memmod::page_4_kb*>(::xv6_memmod::page_round_ceil<::xv6_memmod::page_size::_4kb>(vstart)),
		reinterpret_cast<::xv6_memmod::page_4_kb*>(::xv6_memmod::page_round_ceil<::xv6_memmod::page_size::_4kb>(vend))
	));
	kmem._use_lock = true;
}

extern "C" void freerange(void* vstart, void* vend)
{
	kmem.reserve_range(::fast_io::span(
		reinterpret_cast<::xv6_memmod::page_4_kb*>(::xv6_memmod::page_round_ceil<::xv6_memmod::page_size::_4kb>(vstart)),
		reinterpret_cast<::xv6_memmod::page_4_kb*>(::xv6_memmod::page_round_ceil<::xv6_memmod::page_size::_4kb>(vend))
	));
}

extern "C" char end[];
//PAGEBREAK: 21
// Free the page of physical memory pointed at by v,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
extern "C" void kfree(char* v)
{
	if ((uint)v % PGSIZE || v < end || V2P(v) >= PHYSTOP)
		::fast_io::fast_terminate();

	kmem.deallocate(*::std::launder(reinterpret_cast<::xv6_memmod::page_4_kb*>(v)));
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
extern "C" char* kalloc(void)
{
	return ::std::launder(reinterpret_cast<char*>(::std::addressof(kmem.allocate())));
}

