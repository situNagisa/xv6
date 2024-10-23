#pragma once

#include "./detail/page.h"
#include "./detail/kernel_transformer.h"
#include "./detail/primary.h"
#include "./detail/secondary.h"

#include "./detail/allocator.h"

#include "./environment.h"

NAGISA_BUILD_LIB_BEGIN

using details::page;
using details::primary_page;
using details::secondary_page;

using details::kernel_address_transformer;

using details::table_entry;
using details::page_table;
using details::secondary_directory_entry;
using details::secondary_page_directory;

using details::primary_directory_entry;
using details::primary_page_directory;


using details::page_allocator;

NAGISA_BUILD_LIB_END