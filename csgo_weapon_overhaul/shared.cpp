#include <cstdint>
#include "cbase.h"
#include <lua.hpp>
#include <string>
#include <sstream>
#include <io.h>
#include "memutil.h"
#include "gameutil.h"

template<bool sv> static void __fastcall CSBaseGunFire(void *thisptr)
{
	auto **vtable = *(void***)(thisptr);
	auto *L = (lua_State*)(vtable[-2]);

	lua_getglobal(L, "Fire");
	if (lua_pcall(L, 0, 1, 0) != 0)
	{
		Warning("%s\n", lua_tostring(L, -1));
		return;
	}

	const auto result = lua_tonumber(L, -1);
	lua_pop(L, 1);
	orig_base_gun_fire<sv>(thisptr);
}

template<bool sv> static float __fastcall GetInaccuracy(void *thisptr)
{
	auto **vtable = *(void***)(thisptr);
	auto *L = (lua_State*)(vtable[-2]);

	auto *owner = get_weapon_owner<sv>(thisptr);

	lua_getglobal(L, "GetInaccuracy");
	lua_pushboolean(L, get_player_flags<sv>(owner) & FL_DUCKING);
	lua_pushboolean(L, get_player_flags<sv>(owner) & FL_ONGROUND);
	lua_pushnumber(L, get_player_speed<sv>(owner));
	if (lua_pcall(L, 3, 1, 0) != 0)
	{
		Warning("%s\n", lua_tostring(L, -1));
		return 0.F;
	}

	const auto result = lua_tonumber(L, -1);
	lua_pop(L, 1);
	return result;
}

static void __fastcall Jump(void *thisptr, const float zspeed)
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

static void __fastcall Land(void *thisptr, const float zspeed)
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

static float __fastcall GetSpread(void *thisptr)
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

template<bool sv> static void __fastcall UpdateAccuracyPenalty(void *thisptr)
{
	auto **vtable = *(void***)(thisptr);
	auto *L = (lua_State*)(vtable[-2]);

	auto *owner = get_weapon_owner<sv>(thisptr);

	lua_getglobal(L, "Update");
	lua_pushnumber(L, get_globals<sv>()->interval_per_tick);
	lua_pushboolean(L, get_player_flags<sv>(owner) & FL_DUCKING);
	lua_pushboolean(L, get_player_flags<sv>(owner) & FL_ONGROUND);
	if (lua_pcall(L, 3, 0, 0) != 0)
	{
		Warning("%s\n", lua_tostring(L, -1));
		return;
	}
}

template <bool sv> void hook_entity(void *ent)
{
	const auto *name = get_ent_classname<sv>(ent);

	// Check if the string begins with weapon_, this is performance critical
	static const std::hash<std::string> hasher;
	static const auto weapon_hash = hasher(std::string("weapon_"));

	if (hasher(std::string(name, 7)) != weapon_hash)
		return;

	std::stringstream path;
	path << "csgo/addons/csgo_weapon_overhaul/" << name << ".lua";

	// Don't error spew if the file doesn't exist
	if (_access(path.str().c_str(), 4 /*read*/) == -1)
		return;

	auto *L = luaL_newstate();
	luaL_openlibs(L);

	if (luaL_dofile(L, path.str().c_str()) != 0)
	{
		Warning("%s\n", lua_tostring(L, -1));
		lua_close(L);
		return;
	}

	vmt_hook vhook;
	vhook.init(ent, L);
	hook_weapon<sv>(
		&vhook,
		CSBaseGunFire<sv>,
		Jump,
		Land,
		GetInaccuracy<sv>,
		GetSpread,
		UpdateAccuracyPenalty<sv>);

	Msg("%s: Hooked 0x%X %s\n", sv ? "Server" : "Client", ent, name);
}

class cl_entity_listener
{
public:
	virtual void OnEntityCreated(void *ent)
	{
		hook_entity<false>(ent);
	}

	virtual void OnEntityDeleted(void *ent) {}
};

class sv_entity_listener
{
public:
	virtual void OnEntityCreated(void *ent) {}

	virtual void OnEntitySpawned(void *ent)
	{
		hook_entity<true>(ent);
	}

	virtual void OnEntityDeleted(void *ent) {}
};

void add_entity_listeners()
{
	using AddListenerEntity_t = void(__stdcall*)(void*);

	const auto cl_AddListenerEntity = (AddListenerEntity_t)(sigscan(
		"client.dll",
		"\x55\x8B\xEC\x8B\x0D\x00\x00\x00\x00\x33\xC0",
		"xxxxx????xx"));

	if (cl_AddListenerEntity != nullptr)
	{
		static cl_entity_listener cl_listener;
		cl_AddListenerEntity(&cl_listener);
	}

	const auto sv_AddListenerEntity = (AddListenerEntity_t)(sigscan(
		"server.dll",
		"\x55\x8B\xEC\x8B\x0D\x00\x00\x00\x00\x33\xC0\x56",
		"xxxxx????xxx"));

	if (sv_AddListenerEntity != nullptr)
	{
		static sv_entity_listener sv_listener;
		sv_AddListenerEntity(&sv_listener);
	}
}