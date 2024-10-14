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
#include <fast_io.h>
#include <fast_io_dsal/array.h>
#include <nagisa/concept/concept.h>
#include <nagisa/bit/bit.h>
#include "../environment.h"

#define XV6_KERNEL_MEMMOD_NS XV6_KERNEL_NS::memmod

#define NGS_BUILD_LIB_NAME XV6_KERNEL_MEMMOD_NS
#define NGS_BUILD_LIB_CONFIG_VERSION (1,0,0)
#include <nagisa/build_lib/construct.h>