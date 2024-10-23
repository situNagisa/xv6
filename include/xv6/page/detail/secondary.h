#pragma once

#include "./entry.h"
#include "./kernel_transformer.h"
#include "./page.h"
#include "./environment.h"

NAGISA_BUILD_LIB_DETAIL_BEGIN

#ifdef _MSC_VER
#	pragma pack(1)
#endif

struct table_entry : basic_entry<table_entry, ::xv::pages::page_mode::secondary>
{
private:
	using self_type = table_entry;
	using base_type = basic_entry<table_entry, ::xv::pages::page_mode::secondary>;
public:
	::std::uintptr_t _present : 1;
	::std::uintptr_t writable : 1;
	::std::uintptr_t user_accessible : 1;
	::std::uintptr_t page_level_write_through : 1;
	::std::uintptr_t page_level_cache_disable : 1;
	::std::uintptr_t accessed : 1;
	::std::uintptr_t dirty : 1;
	::std::uintptr_t pat : 1;
	::std::uintptr_t global : 1;
	::std::uintptr_t ignored : 3;
	::std::uintptr_t _base : 20;

	using base_type::map;
	using base_type::remap;
	using base_type::unmap;
	using base_type::present;
	using base_type::physical_address;

	[[noreturn]] constexpr secondary_page& target() noexcept
	{
		::fast_io::fast_terminate();
	}
	[[noreturn]] constexpr secondary_page const& target() const noexcept
	{
		::fast_io::fast_terminate();
	}
}
#if defined(__GNUC__) || defined(__clang__)
__attribute__((packed))
#endif
;
#ifdef _MSC_VER
#	pragma pack()
#endif
static_assert(::xv::pages::table_entry<table_entry>);

using page_table = ::fast_io::array<table_entry, 1 << ::xv::pages::page_trait<::xv::pages::page_mode::secondary>::table_bit_width>;
static_assert(::xv::pages::page_table<page_table>);


struct secondary_directory_entry : basic_entry<secondary_directory_entry, ::xv::pages::page_mode::secondary>
{
private:
	using self_type = secondary_directory_entry;
	using base_type = basic_entry<secondary_directory_entry, ::xv::pages::page_mode::secondary>;
public:
	::std::uintptr_t _present : 1;
	::std::uintptr_t writable : 1;
	::std::uintptr_t user_accessible : 1;
	::std::uintptr_t page_level_write_through : 1;
	::std::uintptr_t page_level_cache_disable : 1;
	::std::uintptr_t accessed : 1;
	::std::uintptr_t dirty : 1;
	::std::uintptr_t _page_size : 1 = false;
	::std::uintptr_t ignored : 4;
	::std::uintptr_t _base : 20;

	using base_type::map;
	using base_type::remap;
	using base_type::unmap;
	using base_type::present;
	using base_type::physical_address;

	constexpr auto&& target() noexcept
	{
		return *kernel_address_transformer::physical_to_visual<page_table>(base_type::physical_address());
	}
	constexpr auto&& target() const noexcept
	{
		return *kernel_address_transformer::physical_to_visual<const page_table>(base_type::physical_address());
	}
};
static_assert(::xv::pages::secondary_directory_entry<secondary_directory_entry>);
using secondary_page_directory = ::fast_io::array<secondary_directory_entry, 1 << ::xv::pages::page_trait<::xv::pages::page_mode::secondary>::directory_bit_width>;
static_assert(::xv::pages::secondary_page_directory<secondary_page_directory>);

NAGISA_BUILD_LIB_DETAIL_END