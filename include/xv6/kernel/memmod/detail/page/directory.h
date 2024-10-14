#pragma once

#include "../address.h"
#include "./page.h"
#include "./table.h"
#include "./environment.h"

// XV6_KERNEL_MEMMOD_CHECK_PAGE_DIRECTORY_MAP

NGS_BUILD_LIB_DETAIL_BEGIN

template<page_size>
struct page_directory_entry;
using page_4_kb_directory_entry = page_directory_entry<page_size::_4kb>;
using page_4_mb_directory_entry = page_directory_entry<page_size::_4mb>;

template<>
struct page_directory_entry<page_size::_4kb>
{
	using page_type = page_4_kb;
	using table_type = page_table_entry;
	using underlying_type = ::std::uint_least32_t;

	underlying_type _present : 1;
	underlying_type access_type : 1;
	underlying_type user_supervisor : 1;
	underlying_type page_level_write_through : 1;
	underlying_type page_level_cache_disable : 1;
	underlying_type accessed : 1;
	underlying_type dirty : 1;
	underlying_type _page_size : 1 = false;
	underlying_type ignored : 4;
	underlying_type _table_base : 20;

	constexpr bool present() const noexcept { return _present; }

	template<address_visual_to_physical_transformer<const table_type> auto Transformer>
	constexpr void map(const table_type& table) noexcept
	{
#if XV6_KERNEL_MEMMOD_CHECK_PAGE_DIRECTORY_MAP
		if (_present)
			::fast_io::fast_terminate();
#endif
		_present = true;
		_table_base = Transformer(::std::addressof(table)) >> 12;
	}
	template<address_visual_to_physical_transformer<const table_type> auto Transformer>
	constexpr void remap(const table_type& table) noexcept
	{
#if XV6_KERNEL_MEMMOD_CHECK_PAGE_DIRECTORY_MAP
		if (!_present)
			::fast_io::fast_terminate();
#endif
		_table_base = Transformer(::std::addressof(table)) >> 12;
	}
	constexpr void unmap() noexcept
	{
#if XV6_KERNEL_MEMMOD_CHECK_PAGE_DIRECTORY_MAP
		if (!_present)
			::fast_io::fast_terminate();
#endif
		_present = false;
	}
	template<address_physical_to_visual_transformer<const table_type> auto Transformer>
	constexpr auto&& unmap() noexcept
	{
		unmap();
		return *Transformer(_table_base << 12);
	}
};


template<>
struct page_directory_entry<page_size::_4mb>
{
	using page_type = page_4_mb;
	using underlying_type = ::std::uint_least32_t;

	underlying_type _present : 1;
	underlying_type access_type : 1;
	underlying_type user_supervisor : 1;
	underlying_type page_level_write_through : 1;
	underlying_type page_level_cache_disable : 1;
	underlying_type accessed : 1;
	underlying_type dirty : 1;
	underlying_type _page_size : 1 = true;
	underlying_type global : 1;
	underlying_type ignored : 3;
	underlying_type pat : 1;
	underlying_type offset : 4;
	underlying_type : 5;
	underlying_type _page_base : 10;

	constexpr bool present() const noexcept { return _present; }

	template<address_visual_to_physical_transformer<const page_type> auto Transformer>
	constexpr void map(const page_type& page) noexcept
	{
#if XV6_KERNEL_MEMMOD_CHECK_PAGE_DIRECTORY_MAP
		if (_present)
			::fast_io::fast_terminate();
#endif
		_present = true;
		_page_base = Transformer(::std::addressof(page)) >> 12;
	}
	template<address_visual_to_physical_transformer<const page_type> auto Transformer>
	constexpr void remap(const page_type& page) noexcept
	{
#if XV6_KERNEL_MEMMOD_CHECK_PAGE_DIRECTORY_MAP
		if (!_present)
			::fast_io::fast_terminate();
#endif
		_page_base = Transformer(::std::addressof(page)) >> 12;
	}
	constexpr void unmap() noexcept
	{
#if XV6_KERNEL_MEMMOD_CHECK_PAGE_DIRECTORY_MAP
		if (!_present)
			::fast_io::fast_terminate();
#endif
		_present = false;
	}
	template<address_physical_to_visual_transformer<const page_type> auto Transformer>
	constexpr auto&& unmap() noexcept
	{
		unmap();
		return *Transformer(_page_base << 12);
	}
};

NGS_BUILD_LIB_DETAIL_END