#include "gameutil.h"
#include "memutil.h"

#define CL_CS_BASE_GUN_FIRE_IDX 325
#define CL_JUMP_IDX 460
#define CL_LAND_IDX 461
#define CL_GET_INACCURACY_IDX 481
#define CL_GET_SPREAD_IDX 482
#define CL_UPDATE_ACCURACY_PENALTY_IDX 483

#define SV_CS_BASE_GUN_FIRE_IDX 307
#define SV_JUMP_IDX 447
#define SV_LAND_IDX 448
#define SV_GET_INACCURACY_IDX 459
#define SV_GET_SPREAD_IDX 460
#define SV_UPDATE_ACCURACY_PENALTY_IDX 461

template<> const CGlobalVarsBase *get_globals<false>()
{
	static auto *g_pGlobals = **(CGlobalVarsBase***)(sigscan(
		"client.dll",
		"\xA1\x00\x00\x00\x00\xF3\x0F\x5E\xC8\xF3\x0F\x10\x40\x00\xF3\x0F\x59\xC1\x0F\x57\x05",
		"x????xxxxxxxx?xxxxxxx") + 1);

	return g_pGlobals;
}
template<> const CGlobalVarsBase *get_globals<true>()
{
	static auto *g_pGlobals = **(CGlobalVarsBase***)(sigscan(
		"server.dll",
		"\xA1\x00\x00\x00\x00\xF3\x0F\x5E\xC8\xF3\x0F\x10\x40\x00\xF3\x0F\x59\xC1",
		"x????xxxxxxxx?xxxx") + 1);

	return g_pGlobals;
}

template<> void *get_weapon_owner<false>(const void *weap)
{
	using GetOwner_t = void*(__thiscall*)(const void *thisptr);
	static auto GetOwner = (GetOwner_t)(sigscan(
		"client.dll",
		"\x74\x08\x8B\x01\xFF\xA0\x68\x02\x00\x00\x33\xC0\xC3",
		"xxxxxxxxxxxxx") - 0x22);

	return GetOwner(weap);
}
template<> void *get_weapon_owner<true>(const void *weap)
{
	using GetOwner_t = void*(__thiscall*)(const void *thisptr);
	static auto GetOwner = (GetOwner_t)(sigscan(
		"server.dll",
		"\x8B\x89\x00\x00\x00\x00\x83\xF9\xFF\x74\x27",
		"xx????xxxxx"));

	return GetOwner(weap);
}

template<> int get_player_flags<false>(const void *player)
{
	return *(int*)((char*)(player)+0x100);
}
template<> int get_player_flags<true>(const void *player)
{
	return *(int*)((char*)(player)+0xCC);
}

template<> float get_player_speed<false>(const void *player)
{
	return sqrtf(
		*(float*)((char*)(player)+0x94) *
		*(float*)((char*)(player)+0x94) +
		*(float*)((char*)(player)+0x98) *
		*(float*)((char*)(player)+0x98));
}
template<> float get_player_speed<true>(const void *player)
{
	return sqrtf(
		*(float*)((char*)(player)+0x170) *
		*(float*)((char*)(player)+0x170) +
		*(float*)((char*)(player)+0x174) *
		*(float*)((char*)(player)+0x174));
}

template<> const char *get_ent_classname<false>(const void *entity)
{
	using GetClassname_t = const char*(__thiscall*)(const void*);
	return ((GetClassname_t)(get_vfunc(entity, 139)))(entity);
}
template<> const char *get_ent_classname<true>(const void *entity)
{
	return *(char**)((char*)(entity)+0x5C);
}

template<> void hook_weapon<false>(
	vmt_hook *vhook,
	void(__fastcall *CSBaseGunFire)(void*),
	void(__fastcall *Jump)(void*, float),
	void(__fastcall *Land)(void*, float),
	float(__fastcall *GetInaccuracy)(void*),
	float(__fastcall *GetSpread)(void*),
	void(__fastcall *UpdateAccuracyPenalty)(void*))
{
		vhook->hook(CL_CS_BASE_GUN_FIRE_IDX, CSBaseGunFire);
		vhook->hook(CL_JUMP_IDX, Jump);
		vhook->hook(CL_LAND_IDX, Land);
		vhook->hook(CL_GET_INACCURACY_IDX, GetInaccuracy);
		vhook->hook(CL_GET_SPREAD_IDX, GetSpread);
		vhook->hook(CL_UPDATE_ACCURACY_PENALTY_IDX, UpdateAccuracyPenalty);
}

template<> void hook_weapon<true>(
	vmt_hook *vhook,
	void(__fastcall *CSBaseGunFire)(void*),
	void(__fastcall *Jump)(void*, float),
	void(__fastcall *Land)(void*, float),
	float(__fastcall *GetInaccuracy)(void*),
	float(__fastcall *GetSpread)(void*),
	void(__fastcall *UpdateAccuracyPenalty)(void*))
{
		vhook->hook(SV_CS_BASE_GUN_FIRE_IDX, CSBaseGunFire);
		vhook->hook(SV_JUMP_IDX, Jump);
		vhook->hook(SV_LAND_IDX, Land);
		vhook->hook(SV_GET_INACCURACY_IDX, GetInaccuracy);
		vhook->hook(SV_GET_SPREAD_IDX, GetSpread);
		vhook->hook(SV_UPDATE_ACCURACY_PENALTY_IDX, UpdateAccuracyPenalty);
}

template<> void orig_base_gun_fire<false>(void *weap)
{
	using CSBaseGunFire_t = void(__thiscall*)(void*);
	auto **vtable = *(void***)(weap);
	auto **old_vtable = (void**)(vtable[-3]);
	((CSBaseGunFire_t)(old_vtable[CL_CS_BASE_GUN_FIRE_IDX]))(weap);
}

template<> void orig_base_gun_fire<true>(void *weap)
{
	using CSBaseGunFire_t = void(__thiscall*)(void*);
	auto **vtable = *(void***)(weap);
	auto **old_vtable = (void**)(vtable[-3]);
	((CSBaseGunFire_t)(old_vtable[SV_CS_BASE_GUN_FIRE_IDX]))(weap);
}