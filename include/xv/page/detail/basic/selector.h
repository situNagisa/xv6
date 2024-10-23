#pragma once

#include "./mode.h"
#include "./environment.h"

NAGISA_BUILD_LIB_DETAIL_BEGIN

template<page_mode>
struct page_selector;

template<>
struct page_selector<page_mode::secondary>
{
private:
	using self_type = page_selector;
public:
	constexpr static auto mode = page_mode::secondary;
private:
	using page_trait_type = page_trait<mode>;
public:

	::std::uintptr_t offset : page_trait_type::page_bit_width;
	::std::uintptr_t table : page_trait_type::table_bit_width;
	::std::uintptr_t directory : page_trait_type::directory_bit_width;
};
using secondary_page_selector = page_selector<page_mode::secondary>;
static_assert(sizeof(secondary_page_selector) == sizeof(::std::uintptr_t));

template<>
struct page_selector<page_mode::primary>
{
private:
	using self_type = page_selector;
public:
	constexpr static auto mode = page_mode::primary;
private:
	using page_trait_type = page_trait<mode>;
public:

	::std::uintptr_t offset : page_trait_type::page_bit_width;
	constexpr static ::std::uintptr_t table{};
	::std::uintptr_t directory : page_trait_type::directory_bit_width;
};
using primary_page_selector = page_selector<page_mode::primary>;
static_assert(sizeof(primary_page_selector) == sizeof(::std::uintptr_t));


template<page_mode Mode>
constexpr auto make_page_selector(void const* address) noexcept
{
	return ::std::bit_cast<page_selector<Mode>>(address);
}

NAGISA_BUILD_LIB_DETAIL_END