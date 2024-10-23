#pragma once

#include <utility>

#include <boost/stl_interfaces/iterator_interface.hpp>

#include <fast_io.h>
#include <fast_io_dsal/array.h>
#include <fast_io_dsal/span.h>

#include "xv/page.h"

#include "../environment.h"

#define XV6_PAGE_NS XV6_NS::pages

#define NAGISA_BUILD_LIB_NAME XV6_PAGE_NS
#define NAGISA_BUILD_LIB_CONFIG_VERSION (1,0,0)
#include <nagisa/build_lib/construct.h>