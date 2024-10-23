#pragma once

#include "./environment.h"

NAGISA_BUILD_LIB_DETAIL_BEGIN

struct kernel_address_transformer
{
	using self_type = kernel_address_transformer;

	constexpr static auto visual_to_physical(::std::uintptr_t address) noexcept
	{
		return static_cast<::std::uintptr_t>(address - static_cast<::std::uintptr_t>(0x8000'0000u));
	}
	constexpr static decltype(auto) visual_to_physical(::nagisa::concepts::pointer auto address) noexcept
	{
		return self_type::visual_to_physical(::std::bit_cast<::std::uintptr_t>(address));
	}
	constexpr static auto physical_to_visual(::std::uintptr_t address) noexcept
	{
		return static_cast<::std::uintptr_t>(address + static_cast<::std::uintptr_t>(0x8000'0000u));
	}
	template<class T>
	constexpr static auto physical_to_visual(::std::uintptr_t address) noexcept
	{
		return ::std::bit_cast<T*>(self_type::physical_to_visual(address));
	}
};

NAGISA_BUILD_LIB_DETAIL_END