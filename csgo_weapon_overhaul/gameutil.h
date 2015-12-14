#include <cstdint>
#include "globalvars_base.h"
#include "memutil.h"

template<bool sv> const CGlobalVarsBase *get_globals();
template<bool sv> void *get_weapon_owner(const void *weap);
template<bool sv> int get_player_flags(const void *player);
template<bool sv> float get_player_speed(const void *player);
template<bool sv> const char *get_ent_classname(const void *entity);

template<bool sv> void hook_weapon(
	vmt_hook *vhook,
	void(__fastcall *CSBaseGunFire)(void*),
	void(__fastcall *Jump)(void*, float),
	void(__fastcall *Land)(void*, float),
	float(__fastcall *GetInaccuracy)(void*),
	float(__fastcall *GetSpread)(void*),
	void(__fastcall *UpdateAccuracyPenalty)(void*));

template<bool sv> void orig_base_gun_fire(void *weap);