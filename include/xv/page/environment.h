#pragma once


#include <cstddef>
#include <cstdint>
#if !defined(__cpp_lib_freestanding_utility) && !__has_include(<utility>) 
#	error "need freestanding utility"
#else
#	include <utility>
#endif
#include <bit>
#include <concepts>
#include <climits>
#include <ranges>

#include <boost/stl_interfaces/iterator_interface.hpp>

#include <nagisa/bit/bit.h>

#include <fast_io.h>

#include "../environment.h"

#define XV_PAGE_NS XV_NS::pages

#define NAGISA_BUILD_LIB_NAME XV_PAGE_NS
#define NAGISA_BUILD_LIB_CONFIG_VERSION (1,0,0)
#include <nagisa/build_lib/construct.h>