#pragma once

#include "./environment.h"

NAGISA_BUILD_LIB_DETAIL_BEGIN

static_assert(CHAR_BIT == 8);
static_assert(sizeof(bool) == 1);
static_assert(sizeof(::std::uint_least8_t) == 1);
static_assert(sizeof(::std::uint_least16_t) == 2);

enum class descriptor_type_t : ::std::uint_least8_t
{
	system,
	code_or_data,
};
enum class privilege_t : ::std::uint_least8_t
{
	ring0,
	ring1,
	ring2,
	ring3
};
constexpr auto operator<=>(privilege_t left, privilege_t right)
{
	return ::std::to_underlying(right) <=> ::std::to_underlying(left);
}

enum class bit_width_t : ::std::uint_least8_t
{
	bits16,
	bits32,
};

enum class descriptor_gate_t : ::std::uint_least8_t
{
	call = 0b00,
	task = 0b01,
	interrupt = 0b10,
	trap = 0b11,
};

enum class code_bit_t : ::std::uint_least8_t
{
	compatibility = 0,
	long_mode = 1
};

enum class granularity : ::std::uint_least32_t
{
	bit = ::nagisa::bits::literals::operator ""_b(1),
	page = ::nagisa::bits::literals::operator ""_kb(4),
};

NAGISA_BUILD_LIB_DETAIL_END