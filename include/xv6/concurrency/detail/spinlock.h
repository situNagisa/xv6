#pragma once

#include "./environment.h"

NAGISA_BUILD_LIB_DETAIL_BEGIN

// basic lockable

struct spinlock
{
	bool _locked;

	// For debugging:
	char const* name;   // Name of lock.
	struct cpu* cpu;   // The cpu holding the lock.
	uint pcs[10];      // The call stack (an array of program counters)
	// that locked the lock.
};

NAGISA_BUILD_LIB_DETAIL_END