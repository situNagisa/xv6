#pragma once

#include "./entry.h"
#include "./page.h"
#include "./table.h"
#include "./environment.h"

NAGISA_BUILD_LIB_DETAIL_BEGIN

template<class T>
concept secondary_directory_entry = entry<T> && page_table<target_t<T>>;

template<class T>
concept secondary_page_directory = entry_range<T> && secondary_directory_entry<::std::ranges::range_value_t<T>>;

template<class T>
concept primary_directory_entry = entry<T> && primary_page<target_t<T>>;

template<class T>
concept primary_page_directory = entry_range<T> && primary_directory_entry<::std::ranges::range_value_t<T>>;

NAGISA_BUILD_LIB_DETAIL_END