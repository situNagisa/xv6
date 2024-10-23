#pragma once

#include "./detail/enum.h"
#include "./detail/descriptor.h"
#include "./environment.h"

NAGISA_BUILD_LIB_BEGIN

using details::segment_data;
using details::segment_code;
using details::segment_local_descriptor_table;
using details::segment_task;
using details::segment_gate;
using details::segment_descriptor_size;

using details::descriptor_type_t;
using details::privilege_t;
using details::bit_width_t;
using details::descriptor_gate_t;
using details::code_bit_t;
using details::granularity;

namespace types = details::segment_types;

NAGISA_BUILD_LIB_END