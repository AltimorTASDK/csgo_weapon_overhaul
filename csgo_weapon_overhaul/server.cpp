#include "cbase.h"
#include "filesystem.h"
#include <lua.hpp>
#include <string>
#include <map>
#include <regex>
#include <algorithm>

const void **get_weap_vtable(
	const char *classname);

struct weap_callbacks
{
	// GetSpread()
	// returns spread (shotgun cone)
	int GetSpread;

	// GetInaccuracy(base_inaccuracy)
	// returns final inaccuracy used for shot
	int GetInaccuracy;

	// UpdateAccuracyPenalty(dt, old_inaccuracy)
	// returns new inaccuracy
	int UpdateAccuracyPenalty;

	// Jump(old_inaccuracy)
	// returns new inaccuracy
	int Jump;

	// Land(old_inaccuracy)
	// returns new inaccuracy
	int Land;
};

static std::map<const void**, weap_callbacks> callback_map;

/**
 * hook_weapon(
 *	classname,
 *	GetSpread,
 *	GetInaccuracy,
 *	UpdateAccuracyPenalty,
 *	Jump,
 *	Land)
 */
static int lua_hook_weapon(
	lua_State *L)
{
	if (lua_gettop(L) != 6)
	{
		Warning("Wrong number of arguments supplied to hook_weapon\n");
		return 0;
	}

	const auto *classname = luaL_checkstring(L, -1);
	if (strncmp(classname, "weapon_", strlen("weapon_")) != 0)
	{
		Warning("%s is not a weapon entity class\n", classname);
		return 0;
	}

	const auto **vtable = get_weap_vtable(classname);
	if (vtable == nullptr)
	{
		Warning("Couldn't find entity factory for %s\n", classname);
		return 0;
	}

	weap_callbacks cb;
	cb.GetSpread = luaL_ref(L, LUA_REGISTRYINDEX);
	cb.GetInaccuracy = luaL_ref(L, LUA_REGISTRYINDEX);
	cb.UpdateAccuracyPenalty = luaL_ref(L, LUA_REGISTRYINDEX);
	cb.Jump = luaL_ref(L, LUA_REGISTRYINDEX);
	cb.Land = luaL_ref(L, LUA_REGISTRYINDEX);

	callback_map[vtable] = cb;

	return 0;
}

void lua_init()
{
	auto *L = luaL_newstate();
	luaL_openlibs(L);

	lua_pushcfunction(L, lua_hook_weapon);
	lua_setglobal(L, "hook_weapon");

	FileFindHandle_t h;
	const auto *name = g_pFullFileSystem->FindFirst(
		"addons/csgo_weapon_overhaul/*.lua", &h);

	for (; name != nullptr; name = g_pFullFileSystem->FindNext(h))
	{
		if (luaL_dofile(L, name) != 0)
		{
			Warning("Failed to load %s: %s\n", name, lua_tostring(L, -1));
			continue;
		}

		const std::regex regex(".*\\/(.*)\\.lua");
		std::cmatch result;
		std::regex_search(name, result, regex);

		auto main = std::string(result[0]) + "_main";
		std::transform(main.begin(), main.end(), main.begin(), tolower);

		lua_getglobal(L, main.c_str());
		if (lua_pcall(L, 0, 0, 0) != 0)
			Warning("Couldn't run main function %s\n", main.c_str());

		Msg("Loaded %s\n", name);
	}
}