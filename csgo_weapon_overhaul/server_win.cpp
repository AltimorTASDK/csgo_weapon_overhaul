#include "cbase.h"
#include "memutil.h"

//-----------------------------------------------------------------------------
// Singleton accessor
//-----------------------------------------------------------------------------
IEntityFactoryDictionary *EntityFactoryDictionary()
{
	static auto *s_EntityFactory = *(IEntityFactoryDictionary**)(sigscan(
		"server.dll",
		"\x83\xEC\x0C\xC7\x05\x00\x00\x00\x00\x00\x00\x00\x00\x83\xC8\x01",
		"xxxxx????????xxx") + 5);

	return s_EntityFactory;
}

const void **get_weap_vtable(
	const char *classname)
{
	auto *factory = EntityFactoryDictionary()->FindFactory(classname);
	if (factory == nullptr)
		return nullptr;

	// First vfunc is the entity constructor
	const auto ctor = (uintptr_t)(get_vfunc(factory, 0));

	// Constructors get generated in one of two ways
	const auto ctor_type = *(char*)(ctor + 0x2D) == 0xE8; // CALL

	const auto call_offset = ctor + (ctor_type ? 0x2E : 0x7);
	const auto call_target = *(char**)(call_offset) + call_offset + 4;
	// mov [esi], offset ??_7CAK47@@6BCAK47@@@
	return *(const void***)(call_target + (ctor_type ? 0xB : 0x38));
}