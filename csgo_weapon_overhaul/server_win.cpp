#include <Windows.h>
#include <cstdint>
#include "memutil.h"
#include "cbase.h"

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

const void *hook_ent_factory(
	const char *classname,
	const void *new_ctor)
{
	auto *factory = EntityFactoryDictionary()->FindFactory(classname);
	if (factory == nullptr)
		return nullptr;

	auto **vtable = *(const void***)(factory);
	DWORD old_protect;
	VirtualProtect(&vtable[0], sizeof(void*), PAGE_READWRITE, &old_protect);
	const auto *old = vtable[0];
	vtable[0] = new_ctor;
	VirtualProtect(&vtable[0], sizeof(void*), old_protect, &old_protect);
	
	return old;
}

const CGlobalVarsBase *get_globals()
{
	static auto *g_pGlobals = **(CGlobalVarsBase***)(sigscan(
		"server.dll",
		"\xA1\x00\x00\x00\x00\xF3\x0F\x5E\xC8\xF3\x0F\x10\x40\x00\xF3\x0F\x59\xC1",
		"x????xxxxxxxx?xxxx") + 1);

	return g_pGlobals;
}

void *get_weapon_owner(
	const void *weap)
{
	using GetOwner_t = void*(__thiscall*)(const void *thisptr);
	static auto GetOwner = (GetOwner_t)(sigscan(
		"server.dll",
		"\x8B\x89\x00\x00\x00\x00\x83\xF9\xFF\x74\x27",
		"xx????xxxxx"));

	return GetOwner(weap);
}

int get_player_flags(
	const void *player)
{
	return *(int*)((char*)(player) + 0xCC);
}

float get_player_speed(
	const void *player)
{
	return sqrtf(
		*(float*)((char*)(player)+0x170) *
		*(float*)((char*)(player)+0x170) +
		*(float*)((char*)(player)+0x174) *
		*(float*)((char*)(player)+0x174));
}