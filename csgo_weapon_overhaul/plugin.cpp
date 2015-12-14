#include "tier2/tier2.h"
#include "engine/iserverplugin.h"
#include "icvar.h"
#include "convar.h"
#include <Windows.h>
#undef GetClassName

#ifdef MSVC
//normally defined in libcmt which must be excluded, lua needs this
double _HUGE = std::numeric_limits<double>::infinity();
#endif

void add_entity_listeners();

class plugin : public IServerPluginCallbacks
{
public:
	virtual bool Load(CreateInterfaceFn interfaceFactory, CreateInterfaceFn gameServerFactory)
	{
		add_entity_listeners();
		return true;
	}

	virtual void Unload() {}
	virtual void Pause() {}
	virtual void UnPause() {}

	virtual const char *GetPluginDescription()
	{
		return "Weapons overhaul";
	}

	virtual void LevelInit(char const *pMapName) {}
	virtual void ServerActivate(edict_t *pEdictList, int edictCount, int clientMax) {}
	virtual void GameFrame(bool simulating) {}
	virtual void LevelShutdown() {}
	virtual void ClientActive(edict_t *pEntity) {}
	virtual void ClientFullyConnect(edict_t *pEntity) {}
	virtual void ClientDisconnect(edict_t *pEntity) {}
	virtual void ClientPutInServer(edict_t *pEntity, char const *playername) {}
	virtual void SetCommandClient(int index) {}
	virtual void ClientSettingsChanged(edict_t *pEdict) {}

	virtual PLUGIN_RESULT ClientConnect(bool *bAllowConnect, edict_t *pEntity, const char *pszName, const char *pszAddress, char *reject, int maxrejectlen)
	{
		return PLUGIN_CONTINUE;
	}

	virtual PLUGIN_RESULT ClientCommand(edict_t *pEntity, const CCommand &args)
	{
		return PLUGIN_CONTINUE;
	}

	virtual PLUGIN_RESULT NetworkIDValidated(const char *pszUserName, const char *pszNetworkID)
	{
		return PLUGIN_CONTINUE;
	}

	virtual void OnQueryCvarValueFinished(QueryCvarCookie_t iCookie, edict_t *pPlayerEntity, EQueryCvarValueStatus eStatus, const char *pCvarName, const char *pCvarValue) {}
	virtual void OnEdictAllocated(edict_t *edict) {} 
	virtual void OnEdictFreed(const edict_t *edict) {}
};

plugin inst;
EXPOSE_SINGLE_INTERFACE_GLOBALVAR(
	plugin,
	IServerPluginCallbacks,
	INTERFACEVERSION_ISERVERPLUGINCALLBACKS,
	inst);