#pragma once

#include "./detail/bit_literal.h"
#include "./detail/page.h"

NGS_BUILD_LIB_BEGIN

namespace literals = details::literals;

using details::address_physical_to_visual_transformer;
using details::address_visual_to_physical_transformer;

using details::page_size;

using details::page_round_ceil;
using details::page_round_floor;

using details::page;
using details::page_4_kb;
using details::page_4_mb;
using details::page_table_entry;
using details::page_directory_entry;
using details::page_4_kb_directory_entry;
using details::page_4_mb_directory_entry;

NGS_BUILD_LIB_END