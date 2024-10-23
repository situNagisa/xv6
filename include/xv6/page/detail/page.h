#pragma once

#include "./environment.h"

NAGISA_BUILD_LIB_DETAIL_BEGIN

// for `Providing storage`
// https://en.cppreference.com/w/cpp/language/lifetime
template<::xv::pages::page_mode Mode>
using page = ::fast_io::array<::std::byte, ::xv::pages::page_trait<Mode>::size>;

using secondary_page = page<::xv::pages::page_mode::secondary>;
static_assert(::xv::pages::secondary_page<secondary_page>);
using primary_page = page<::xv::pages::page_mode::primary>;
static_assert(::xv::pages::primary_page<primary_page>);

NAGISA_BUILD_LIB_DETAIL_END