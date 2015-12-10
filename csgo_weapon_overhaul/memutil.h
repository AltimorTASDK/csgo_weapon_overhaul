#pragma once

#include "platform.h"

// Return to the start and end of module address space
void get_module_bounds(
	const char *name,
	uintptr_t *start,
	uintptr_t *end);

// Get the address of a virtual function
const void *get_vfunc(
	const void *object,
	const int idx);

// Scan from start to end of a module for a pattern. A wildcard in sig is
// denoted by a ? in the corresponding index in mask.
uintptr_t sigscan(
	const char *name,
	const char *sig,
	const char *mask);