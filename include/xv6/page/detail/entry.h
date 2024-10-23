#pragma once

#include "./environment.h"

NAGISA_BUILD_LIB_DETAIL_BEGIN

// XV6_KERNEL_MEMMOD_CHECK_PAGE_TABLE_MAP

template<class Derived, ::xv::pages::page_mode Mode>
struct basic_entry
{
	constexpr static auto mode = Mode;
private:
	using self_type = basic_entry;
	using derived_type = Derived;
	using page_trait_type = ::xv::pages::page_trait<Mode>;
	constexpr auto&& as_derived() noexcept { return static_cast<derived_type&>(*this); }
	constexpr auto&& as_derived() const noexcept { return static_cast<derived_type const&>(*this); }
public:
	constexpr bool present() const noexcept { return as_derived()._present; }

	constexpr void map(::std::uintptr_t physical_address) noexcept
	{
#if XV6_KERNEL_MEMMOD_CHECK_PAGE_TABLE_MAP
		if (as_derived().present())
			::fast_io::fast_terminate();
#endif
		as_derived()._present = true;
		as_derived()._base = physical_address >> page_trait_type::page_bit_width;
	}
	constexpr void remap(::std::uintptr_t physical_address) noexcept
	{
#if XV6_KERNEL_MEMMOD_CHECK_PAGE_TABLE_MAP
		if (!as_derived().present())
			::fast_io::fast_terminate();
#endif
		as_derived()._base = physical_address >> page_trait_type::page_bit_width;
	}
	constexpr auto physical_address() const noexcept
	{
#if XV6_KERNEL_MEMMOD_CHECK_PAGE_TABLE_MAP
		if (!as_derived().present())
			::fast_io::fast_terminate();
#endif
		return as_derived()._base << page_trait_type::page_bit_width;
	}
	constexpr void unmap() noexcept
	{
#if XV6_KERNEL_MEMMOD_CHECK_PAGE_TABLE_MAP
		if (!as_derived().present())
			::fast_io::fast_terminate();
#endif
		as_derived()._present = false;
	}
};

NAGISA_BUILD_LIB_DETAIL_END