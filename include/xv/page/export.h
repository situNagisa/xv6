#pragma once

#include "./detail/basic/mode.h"
#include "./detail/basic/selector.h"
#include "./detail/basic/address_iterate.h"
#include "./detail/basic/entry.h"
#include "./detail/basic/page.h"
#include "./detail/basic/table.h"
#include "./detail/basic/directory.h"

#include "./detail/flatten.h"

#include "./environment.h"

NAGISA_BUILD_LIB_BEGIN

using details::page_mode;
using details::page_trait;

using details::page_selector;
using details::primary_page_selector;
using details::secondary_page_selector;
using details::make_page_selector;

using details::iterator;
using details::iterator_t;
using details::sentinel;
using details::sentinel_t;
using details::address_iterable;
using details::address_range;
using details::address_range_t;

using details::target;
using details::target_t;
using details::entry;
using details::entry_mode;
using details::entry_mode_v;
using details::entry_range;

using details::page;
using details::primary_page;
using details::secondary_page;

using details::table_entry;
using details::page_table;

using details::primary_directory_entry;
using details::secondary_directory_entry;
using details::primary_page_directory;
using details::secondary_page_directory;

using details::flatten;

NAGISA_BUILD_LIB_END