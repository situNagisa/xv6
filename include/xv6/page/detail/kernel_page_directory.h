#pragma once

#include "./kernel_transformer.h"
#include "./secondary.h"
#include "./map.h"
#include "./environment.h"

NAGISA_BUILD_LIB_DETAIL_BEGIN


struct kernel_visual_memory : secondary_page_directory
{
	kernel_visual_memory() noexcept
	{
		constexpr auto kernel_mapper = address_mapper{
			.source = {
				::std::bit_cast<::std::byte const*>(static_cast<::std::uintptr_t>(0x8000'0000)),
				::std::bit_cast<::std::byte const*>(static_cast<::std::uintptr_t>(0x8e00'0000))
			},
			.result = static_cast<::std::uintptr_t>(0x0000'0000)
		};
		constexpr auto device_mapper = address_mapper{
			.source = {
				::std::bit_cast<::std::byte const*>(static_cast<::std::uintptr_t>(0xfe00'0000)),
				::std::bit_cast<::std::byte const*>(static_cast<::std::uintptr_t>(0x0000'0000))
			},
			.result = static_cast<::std::uintptr_t>(0xfe00'0000)
		};
		auto kernel_space = ::xv::pages::address_range(
			::std::bit_cast<void const*>(static_cast<::std::uintptr_t>(0x8000'0000)),
			::std::bit_cast<void const*>(static_cast<::std::uintptr_t>(0x8e00'0000))
		);
		auto device_space = ::xv::pages::address_range(
			::std::bit_cast<void const*>(static_cast<::std::uintptr_t>(0xfe00'0000)),
			::std::bit_cast<void const*>(static_cast<::std::uintptr_t>(0x0000'0000))
		);
		auto flattened = ::xv::pages::flatten(*this);
		for (auto&& entry : flattened | kernel_space)
		{
			entry.map(kernel_address_transformer::visual_to_physical(::std::ranges::data(_allocator->allocate())));
			entry.writable = true;
		}

		auto visual_start = ::std::bit_cast<void const*>(::xv6::pages::layouts::kernel_base);
		auto visual_code = visual_start + ::xv6::pages::layouts::extended_memory;
		auto visual_data = static_cast<void const*>(data);
		auto visual_end = ::layout_factory::ptv(::xv6::pages::layouts::top_physical_memory);
		auto visual_device = ::std::bit_cast<void const*>(::xv6::pages::layouts::device_space);
		auto visual_device_end = ::std::bit_cast<void const*>(::layout_factory::physical_start);

		static_assert(::layout_factory::physical_end <= ::layout_factory::physical_device_start);

		constexpr auto page_size = ::xv::pages::page_trait<::xv::pages::page_mode::secondary>::size;
		auto flattened_directory = ::xv::pages::flatten(directory, ::xv6::pages::directory_entry_to_table_transformer);
		static_assert(::xv::pages::page_table<decltype(flattened_directory)>);
		for (
			auto start = visual_start;
			auto && [index, entry] : ::xv::pages::address_range(flattened_directory, start, visual_code) | ::std::views::enumerate)
		{
			entry.map(::layout_factory::vtp(start) + index * page_size);
			entry.writable = true;
		}
		for (
			auto start = visual_code;
			auto && [index, entry] : ::xv::pages::address_range(flattened_directory, start, visual_data) | ::std::views::enumerate)
		{
			entry.map(::layout_factory::vtp(start) + index * page_size);
			entry.writable = false;
		}
		for (
			auto start = visual_data;
			auto && [index, entry] : ::xv::pages::address_range(flattened_directory, start, visual_end) | ::std::views::enumerate)
		{
			entry.map(::layout_factory::vtp(start) + index * page_size);
			entry.writable = true;
		}
		for (
			auto start = visual_device;
			auto && [index, entry] : ::xv::pages::address_range(flattened_directory, start, visual_device_end) | ::std::views::enumerate)
		{
			entry.map(::std::bit_cast<::std::uintptr_t>(start) + index * page_size);
			entry.writable = true;
		}
	}

	struct
	{
		::xv6::pages::page_table_range_holder<decltype(::xv6::pages::kernel_pages(_directory))>
			kernel{ ::xv6::pages::kernel_pages(_directory), kernel_memory, {.writable = true, .user_accessible = true } };
		::xv6::pages::page_table_range_holder<decltype(::xv6::pages::device_pages(_directory))>
			user{ ::xv6::pages::device_pages(_directory), kernel_memory, {.writable = true, .user_accessible = true } };
	}_holders{};
};

NAGISA_BUILD_LIB_DETAIL_END