#include <cstdint>
#include <Windows.h>
#include <Psapi.h>

/**
 * get_module_bounds - Get the boundaries of a module
 * @name:	Name of module
 * @start:	Pointer to copy start of module bounds to
 * @end:	Pointer to copy end of module bounds to
 *
 * Get the module handle and use GetModuleInformation to get its bounds
 */
void get_module_bounds(const char *name, uintptr_t *start, uintptr_t *end)
{
	const auto module = GetModuleHandle(name);
	if(module == nullptr)
		return;

	MODULEINFO info;
	GetModuleInformation(GetCurrentProcess(), module, &info, sizeof(info));
	*start = (uintptr_t)(info.lpBaseOfDll);
	*end = *start + info.SizeOfImage;
}

/**
 * get_vfunc - Get the address of a virtual function
 * @object:	Object to get virtual function for
 * @idx:	Virtual function index to get
 *
 * Return the pointer to the object's vtable and return the function at the
 * specified index.
 */
const void *get_vfunc(
	const void *object,
	const int idx)
{
	const auto **vtable = *(const void***)(object);
	return vtable[idx];
}

/**
 * sigscan - Scan for a code pattern
 * @name:	Name of module to scan
 * @sig:	Byte sequence to scan for
 * @mask:	Wildcard mask, ?s will make the corresponding index in sig be
 *		ignored
 *
 * Check if the pattern matches starting at each byte from start to end. Will
 * throw an exception if the signature isn't found.
 */
uintptr_t sigscan(
	const char *name,
	const char *sig,
	const char *mask)
{
	uintptr_t start, end;
	get_module_bounds(name, &start, &end);

	const auto last_scan = end - strlen(mask) + 1;
	for (auto addr = start; addr < last_scan; addr++) {
		for (size_t i = 0;; i++) {
			if (mask[i] == '\0')
				return addr;
			if (mask[i] != '?' && sig[i] != *(char*)(addr + i))
				break;
		}
	}
	return 0;
}