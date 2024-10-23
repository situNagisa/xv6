#pragma once

#include "../environment.h"

#define XV6_CONCURRENCY_NS XV6_NS::concurrency

#define NAGISA_BUILD_LIB_NAME XV6_CONCURRENCY_NS
#define NAGISA_BUILD_LIB_CONFIG_VERSION (1,0,0)
#include <nagisa/build_lib/construct.h>