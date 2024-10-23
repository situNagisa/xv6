//#include <boost/stl_interfaces/iterator_interface.hpp>
//#include "xv/memmod.h"
//#include "xv6/memmod.h"
#include "xv/page.h"
#include "xv6/page/page.h"
#include <fast_io_dsal/span.h>
#include "spinlock.h"
#include "defs.h"
#include "./kernel_allocate.h"

namespace
{
	template<::xv::pages::page_mode Mode>
	constexpr auto make_page_span(::nagisa::concepts::pointer auto start, ::nagisa::concepts::pointer auto end) noexcept
	{
		using page_type = ::xv6::pages::page<Mode>;
		using trait_type = ::xv::pages::page_trait<Mode>;
		return ::fast_io::span(
			::std::bit_cast<page_type*>(::nagisa::bits::round_ceil<trait_type::page_bit_width>(::std::bit_cast<::std::uintptr_t>(start))),
			::std::bit_cast<page_type*>(::nagisa::bits::round_ceil<trait_type::page_bit_width>(::std::bit_cast<::std::uintptr_t>(end)))
		);
	}
}

// Initialization happens in two phases.
// 1. main() calls kinit1() while still using entrypgdir to place just
// the pages mapped by entrypgdir on free list.
// 2. main() calls kinit2() with the rest of the physical pages
// after installing a full page table that maps them on all cores.
extern "C" void kinit1(void* vstart, void* vend)
{
	kernel_memory.reserve_range(::make_page_span<::xv::pages::page_mode::secondary>(vstart, vend));
}

extern "C" void kinit2(void* vstart, void* vend)
{
	kernel_memory.reserve_range(::make_page_span<::xv::pages::page_mode::secondary>(vstart, vend));
	kernel_memory._use_lock = true;
}

extern "C" void freerange(void* vstart, void* vend)
{
	kernel_memory.reserve_range(::make_page_span<::xv::pages::page_mode::secondary>(vstart, vend));
}

extern "C" char end[];
//PAGEBREAK: 21
// Free the page of physical memory pointed at by v,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
extern "C" void kfree(char* v)
{
	if (
		::std::bit_cast<::std::uintptr_t>(v) & ::nagisa::bits::mask<::xv::pages::page_trait<::xv::pages::page_mode::secondary>::page_bit_width>()
		|| v < end
		|| ::std::bit_cast<::std::uintptr_t>(::xv6::pages::kernel_address_transformer::visual_to_physical(v)) >= 0xe00'0000u
		)
		::fast_io::fast_terminate();

	kernel_memory.deallocate(*::std::launder(reinterpret_cast<::xv6::pages::secondary_page*>(v)));
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
extern "C" char* kalloc(void)
{
	return ::std::launder(reinterpret_cast<char*>(::std::addressof(kernel_memory.allocate())));
}