#pragma once

#include <memory>
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

class vmt_hook
{
	void ***target;
	void **orig_vtable;
	void **new_vtable;

public:
	vmt_hook() : target(nullptr), orig_vtable(nullptr) {}

	// Replace a target's vtable with a copy
	void init(
		void *hook_target,
		void *user_data);

	/**
	 * hook - Hook a virtual function
	 * @idx:	Index to hook
	 * @hook_func:	Pointer to insert
	 *
	 * Replace the specified index in the copied vtable with hook_func.
	 */
	void hook(
		const int idx,
		void *hook_func)
	{
		new_vtable[idx + 3] = hook_func;
	}

	/**
	 * unhook - Unhook a virtual function
	 * @idx:	Index to unhook
	 *
	 * Replace the specified index in the copied vtable with the
	 * contents of the corresponding index in the original vtable.
	 */
	void unhook(
		const int idx)
	{
		new_vtable[idx + 3] = orig_vtable[idx];
	}

	/**
	 * get_orig - Get a virtual function from the original table
	 * @idx:	Index to grab for
	 *
	 * Return the contents of the specified index in the original vtable.
	 */
	template<typename func_t>
	func_t get_orig(
		const int idx)
	{
		return (func_t)(orig_vtable[idx]);
	}
};