#pragma once

#include <cstddef>
#include <cstdint>
#include <concepts>

#include <nagisa/bit/bit.h>

#include "../environment.h"

#define XV_SEGMENT_NS XV_NS::segments

#define NAGISA_BUILD_LIB_NAME XV_SEGMENT_NS
#define NAGISA_BUILD_LIB_CONFIG_VERSION (1,0,0)
#include <nagisa/build_lib/construct.h>