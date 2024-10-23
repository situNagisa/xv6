#pragma once

#include "./entry.h"
#include "./kernel_transformer.h"
#include "./page.h"
#include "./environment.h"

NAGISA_BUILD_LIB_DETAIL_BEGIN

struct primary_directory_entry : basic_entry<primary_directory_entry, ::xv::pages::page_mode::primary>
{
private:
	using self_type = primary_directory_entry;
	using base_type = basic_entry<primary_directory_entry, ::xv::pages::page_mode::primary>;
public:

	::std::uintptr_t _present : 1;
	::std::uintptr_t writable : 1;
	::std::uintptr_t user_accessible : 1;
	::std::uintptr_t page_level_write_through : 1;
	::std::uintptr_t page_level_cache_disable : 1;
	::std::uintptr_t accessed : 1;
	::std::uintptr_t dirty : 1;
	::std::uintptr_t _page_size : 1 = true;
	::std::uintptr_t global : 1;
	::std::uintptr_t ignored : 3;
	::std::uintptr_t pat : 1;
	::std::uintptr_t offset : 4;
	::std::uintptr_t : 5;
	::std::uintptr_t _base : 10;

	using base_type::map;
	using base_type::remap;
	using base_type::unmap;
	using base_type::present;
	using base_type::physical_address;

	constexpr auto&& target() noexcept
	{
		return *kernel_address_transformer::physical_to_visual<primary_page>(base_type::physical_address());
	}
	constexpr auto&& target() const noexcept
	{
		return *kernel_address_transformer::physical_to_visual<const primary_page>(base_type::physical_address());
	}
};
static_assert(::xv::pages::primary_directory_entry<primary_directory_entry>);
using primary_page_directory = ::fast_io::array<primary_directory_entry, 1 << ::xv::pages::page_trait<::xv::pages::page_mode::primary>::directory_bit_width>;
static_assert(::xv::pages::primary_page_directory<primary_page_directory>);


NAGISA_BUILD_LIB_DETAIL_END