#pragma once

#include "./enum.h"
#include "./environment.h"

NAGISA_BUILD_LIB_DETAIL_BEGIN

#ifdef _MSC_VER
#	pragma pack(1)
#endif

#if defined(__GNUC__) || defined(__clang__)
//#	define XV6_SEGMENT_DETAIL_PACKED __attribute__((packed))
#	define XV6_SEGMENT_DETAIL_PACKED [[gnu::packed]]
#else
#	define XV6_SEGMENT_DETAIL_PACKED
#endif

namespace segment_types
{
	struct basic_segment_type {};

	struct data : basic_segment_type
	{
		bool accessed : 1;
		bool writable : 1;
		bool expand_down : 1;
		bool _ignore : 1 = false;
		descriptor_type_t type : 1;
		privilege_t privilege : 2;
		bool present : 1;

		constexpr bool valid() const noexcept { return _ignore == false; }
	} XV6_SEGMENT_DETAIL_PACKED;
	static_assert(sizeof(data) == 1);

	struct code : basic_segment_type
	{
		bool accessed : 1;
		bool readable : 1;
		bool conforming : 1;
		bool _ignore : 1 = true;
		descriptor_type_t type : 1;
		privilege_t privilege : 2;
		bool present : 1;

		constexpr bool valid() const noexcept { return _ignore == true; }
	} XV6_SEGMENT_DETAIL_PACKED;
	static_assert(sizeof(code) == 1);

	struct local_descriptor_table : basic_segment_type
	{
		::std::uint_least8_t _ignore : 4 = 0b0010;
		descriptor_type_t type : 1;
		privilege_t privilege : 2;
		bool present : 1;

		constexpr bool valid() const noexcept { return _ignore == 0b0010; }
	} XV6_SEGMENT_DETAIL_PACKED;
	static_assert(sizeof(local_descriptor_table) == 1);

	struct task_state : basic_segment_type
	{
		bool _ignore : 1 = true;
		bool busy : 1;
		bool _ignore2 : 1 = false;
		bit_width_t bit_width : 1;
		descriptor_type_t type : 1;
		privilege_t privilege : 2;
		bool present : 1;

		constexpr bool valid() const noexcept { return _ignore == true && _ignore2 == false; }
	} XV6_SEGMENT_DETAIL_PACKED;
	static_assert(sizeof(task_state) == 1);

	struct gate : basic_segment_type
	{
		descriptor_gate_t category : 2;
		bool _ignore : 1 = true;
		bit_width_t bit_width : 1;
		descriptor_type_t type : 1;
		privilege_t privilege : 2;
		bool present : 1;

		constexpr bool valid() const noexcept { return _ignore == true; }
	} XV6_SEGMENT_DETAIL_PACKED;
}

inline constexpr auto segment_descriptor_size = 8;

template<class SegmentType>
	requires ::std::derived_from<SegmentType, segment_types::basic_segment_type>
struct segment_descriptor
{
private:
	using self_type = segment_descriptor;
public:
	using segment_type = SegmentType;

	::std::uint_least16_t _limit_low;
	::std::uint_least16_t _base_low;
	::std::uint_least8_t _base_middle;

	segment_type type;

	::std::uint_least8_t _limit_high : 4;
	bool available : 1;
	bool enable_long_mode : 1;
	bool enable_32_bit : 1;
	bool enable_page_granularity : 1;
	::std::uint_least8_t _base_high;

	struct config_type
	{
		segment_type type{};
		bool available : 1{};
		bool enable_long_mode : 1{};
		bool enable_32_bit : 1{};
		bool enable_page_granularity : 1{};
	};

	constexpr static auto create_from(::std::uintptr_t base, ::std::uint_least32_t limit, config_type config = {}) noexcept
	{
		using ::nagisa::bits::mask;
		using ::nagisa::bits::bit_of;

		return self_type{
			._limit_low = limit & mask<decltype(self_type::_limit_low)>(),
			._base_low = base & mask<decltype(self_type::_base_low)>(),
			._base_middle = (base >> bit_of<decltype(self_type::_base_low)>()) & mask<decltype(self_type::_base_middle)>(),
			.type = config.type,
			._limit_high = (limit >> bit_of<decltype(self_type::_limit_low)>()) & mask<4>(),
			.available = config.available,
			.enable_long_mode = config.enable_long_mode,
			.enable_32_bit = config.enable_32_bit,
			.enable_page_granularity = config.enable_page_granularity,
			._base_high = (base >> bit_of<decltype(self_type::_base_middle)>() >> bit_of<decltype(self_type::_base_low)>()) & mask<decltype(self_type::_base_high)>()
		};
	}

	constexpr auto physical_address() const noexcept
	{
		return static_cast<::std::uintptr_t>(
			(static_cast<::std::uintptr_t>(_base_low))
			| (static_cast<::std::uintptr_t>(_base_middle) << ::nagisa::bits::bit_of<decltype(_base_low)>())
			| (static_cast<::std::uintptr_t>(_base_high) << ::nagisa::bits::bit_of<decltype(_base_middle)>() << ::nagisa::bits::bit_of<decltype(_base_low)>())
			);
	}
	constexpr auto limit() const noexcept
	{
		return static_cast<::std::uint_least32_t>(
			(static_cast<::std::uint_least32_t>(_limit_low))
			| (static_cast<::std::uint_least32_t>(_limit_high) << ::nagisa::bits::bit_of<decltype(_limit_low)>())
			);
	}

	constexpr auto size_bytes() const noexcept { return limit() * ::std::to_underlying(enable_page_granularity ? granularity::page : granularity::bit); }
} XV6_SEGMENT_DETAIL_PACKED;
#ifdef _MSC_VER
#	pragma pack()
#endif

using segment_data = segment_descriptor<segment_types::data>;
using segment_code = segment_descriptor<segment_types::code>;
using segment_local_descriptor_table = segment_descriptor<segment_types::local_descriptor_table>;
using segment_task = segment_descriptor<segment_types::task_state>;
using segment_gate = segment_descriptor<segment_types::gate>;

static_assert(sizeof(segment_data) == segment_descriptor_size);
static_assert(sizeof(segment_code) == segment_descriptor_size);
static_assert(sizeof(segment_local_descriptor_table) == segment_descriptor_size);
static_assert(sizeof(segment_task) == segment_descriptor_size);
static_assert(sizeof(segment_gate) == segment_descriptor_size);

NAGISA_BUILD_LIB_DETAIL_END