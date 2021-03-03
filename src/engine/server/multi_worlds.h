#ifndef ENGINE_SERVER_MULTIWORLDS_CONTEXT_H
#define ENGINE_SERVER_MULTIWORLDS_CONTEXT_H

#include <engine/shared/protocol.h>

class CMultiWorlds
{
	bool Add(int WorldID, class IKernel* pKernel, class IServer* pServer);

public:
	struct CWorldGameServer
	{
		char m_aName[64];
		char m_aPath[512];
		class IGameServer* m_pGameServer;
		class IEngineMap* m_pLoadedMap;
	};

	CMultiWorlds()
	{
		for(int i = 0; i < ENGINE_MAX_WORLDS; i++)
		{
			m_Worlds[i].m_pGameServer = nullptr;
			m_Worlds[i].m_pLoadedMap = nullptr;
		}
		m_WasInitilized = 0;
	}
	~CMultiWorlds()
	{
		Clear();
	}
	CWorldGameServer* GetWorld(int WorldID) { return &m_Worlds[WorldID]; };
	bool IsValid(int WorldID) { return (bool)(WorldID >= 0 && WorldID < ENGINE_MAX_WORLDS && m_Worlds[WorldID].m_pGameServer); }
	int GetSizeInitilized() const { return m_WasInitilized; }

	bool LoadWorlds(class IServer* pServer, class IKernel* pKernel, class IStorageEngine* pStorage, class IConsole* pConsole);
	void Clear();

private:
	int m_WasInitilized;
	CWorldGameServer m_Worlds[ENGINE_MAX_WORLDS];
};


#endif