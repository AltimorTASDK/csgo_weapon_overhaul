#include <cstdint>
#include "cbase.h"
#include "memutil.h"
#include <lua.hpp>
#include <string>
#include <map>
#include <sstream>

#define CS_BASE_GUN_FIRE_IDX 307
#define JUMP_IDX 447
#define LAND_IDX 448
#define GET_INACCURACY_IDX 459
#define GET_SPREAD_IDX 460
#define UPDATE_ACCURACY_PENALTY_IDX 461

const void *hook_ent_factory(
	const char *classname,
	const void *new_ctor);

const CGlobalVarsBase *get_globals();
void *get_weapon_owner(
	const void *weap);

int get_player_flags(
	const void *player);

float get_player_speed(
	const void *player);

using ctor_t = void*(__thiscall*)(
	void*,
	const char*);

std::map<std::string, ctor_t> old_ctors;

static void __fastcall CSBaseGunFire(
	void *thisptr)
{
	auto **vtable = *(void***)(thisptr);
	auto *L = (lua_State*)(vtable[-2]);

	lua_getglobal(L, "Fire");
	if (lua_pcall(L, 0, 0, 0) != 0)
	{
		Warning("%s\n", lua_tostring(L, -1));
		return;
	}

	const auto result = lua_tonumber(L, -1);
	lua_pop(L, 1);

	using CSBaseGunFire_t = void(__thiscall*)(
		void*);

	auto **old_vtable = (void**)(vtable[-3]);
	((CSBaseGunFire_t)(old_vtable[CS_BASE_GUN_FIRE_IDX]))(thisptr);
}

static float __fastcall GetInaccuracy(
	void *thisptr)
{
	auto **vtable = *(void***)(thisptr);
	auto *L = (lua_State*)(vtable[-2]);

	auto *owner = get_weapon_owner(thisptr);

	lua_getglobal(L, "GetInaccuracy");
	lua_pushboolean(L, get_player_flags(owner) & FL_DUCKING);
	lua_pushboolean(L, get_player_flags(owner) & FL_ONGROUND);
	lua_pushnumber(L, get_player_speed(owner));
	if (lua_pcall(L, 3, 1, 0) != 0)
	{
		Warning("%s\n", lua_tostring(L, -1));
		return 0.F;
	}

	const auto result = lua_tonumber(L, -1);
	lua_pop(L, 1);
	return result;
}

static void __fastcall Jump(
	void *thisptr,
	const float zspeed)
{
	auto **vtable = *(void***)(thisptr);
	auto *L = (lua_State*)(vtable[-2]);

	lua_getglobal(L, "Jump");
	lua_pushnumber(L, zspeed);
	if (lua_pcall(L, 1, 0, 0) != 0)
	{
		Warning("%s\n", lua_tostring(L, -1));
		return;
	}
}

static void __fastcall Land(
	void *thisptr,
	const float zspeed)
{
	auto **vtable = *(void***)(thisptr);
	auto *L = (lua_State*)(vtable[-2]);

	lua_getglobal(L, "Land");
	lua_pushnumber(L, zspeed);
	if (lua_pcall(L, 1, 0, 0) != 0)
	{
		Warning("%s\n", lua_tostring(L, -1));
		return;
	}
}

static float __fastcall GetSpread(
	void *thisptr)
{
	auto **vtable = *(void***)(thisptr);
	auto *L = (lua_State*)(vtable[-2]);

	lua_getglobal(L, "GetSpread");
	if (lua_pcall(L, 0, 1, 0) != 0)
	{
		Warning("%s\n", lua_tostring(L, -1));
		return 0.F;
	}

	const auto result = lua_tonumber(L, -1);
	lua_pop(L, 1);
	return result;
}

static void __fastcall UpdateAccuracyPenalty(
	void *thisptr)
{
	auto **vtable = *(void***)(thisptr);
	auto *L = (lua_State*)(vtable[-2]);

	auto *owner = get_weapon_owner(thisptr);

	lua_getglobal(L, "Update");
	lua_pushnumber(L, get_globals()->interval_per_tick);
	lua_pushboolean(L, get_player_flags(owner) & FL_DUCKING);
	lua_pushboolean(L, get_player_flags(owner) & FL_ONGROUND);
	if (lua_pcall(L, 3, 0, 0) != 0)
	{
		Warning("%s\n", lua_tostring(L, -1));
		return;
	}
}

static void __fastcall dtor_hook(
	void *thisptr)
{
	auto **vtable = *(void***)(thisptr);
	auto *L = (lua_State*)(vtable[-2]);
	lua_close(L);

	auto **old_vtable = (void**)(vtable[-3]);

	delete[] &vtable[-3];

	using dtor_t = void(__thiscall*)(
		void*);

	((dtor_t)(old_vtable[0]))(thisptr);
}

static void *__fastcall ctor_hook(
	void *thisptr,
	const void*,
	const char *classname)
{
	auto *ent = old_ctors[classname](thisptr, classname);

	auto *L = luaL_newstate();
	luaL_openlibs(L);

	std::stringstream path;
	path << "csgo/addons/csgo_weapon_overhaul/" << classname << ".lua";

	if (luaL_dofile(L, path.str().c_str()) != 0)
	{
		Warning("%s\n", lua_tostring(L, -1));
		return ent;
	}

	vmt_hook vhook;
	// ent is an IServerNetworkable, -0xC for CBaseEntity
	vhook.init((char*)(ent) - 0xC, L);
	//vhook.hook(0, dtor_hook);
	vhook.hook(CS_BASE_GUN_FIRE_IDX, CSBaseGunFire);
	vhook.hook(JUMP_IDX, Jump);
	vhook.hook(LAND_IDX, Land);
	vhook.hook(GET_INACCURACY_IDX, GetInaccuracy);
	vhook.hook(GET_SPREAD_IDX, GetSpread);
	vhook.hook(UPDATE_ACCURACY_PENALTY_IDX, UpdateAccuracyPenalty);

	Msg("Hooked 0x%X, %s\n", ent, classname);

	return ent;
}

void hook_weapons_server()
{
	const auto add_hook = [](const char *name)
	{
		old_ctors[name] = (ctor_t)(hook_ent_factory(name, ctor_hook));
	};

	add_hook("weapon_ak47");
}
