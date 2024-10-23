#pragma once

#include "./page.h"
#include "./entry.h"
#include "./environment.h"

NAGISA_BUILD_LIB_DETAIL_BEGIN

template<class T>
concept table_entry = entry<T> && secondary_page<target_t<T>>;

template<class T>
concept page_table = entry_range<T> && table_entry<::std::ranges::range_value_t<T>>;

NAGISA_BUILD_LIB_DETAIL_END