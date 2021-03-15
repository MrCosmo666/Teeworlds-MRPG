#include "multi_worlds.h"

#include <base/system.h>
#include <engine/console.h>
#include <engine/map.h>
#include <engine/server.h>
#include <engine/storage.h>
#include <engine/external/json-parser/json.h>

bool CMultiWorlds::Add(int WorldID, IKernel* pKernel, IServer *pServer)
{
	dbg_assert(WorldID < ENGINE_MAX_WORLDS, "exceeded pool of allocated memory for worlds");

	CWorldGameServer& pNewWorld = m_Worlds[WorldID];
	pNewWorld.m_pGameServer = CreateGameServer();
	pNewWorld.m_pLoadedMap = CreateEngineMap();
	m_WasInitilized++;

	bool RegisterFail = false;
	RegisterFail = RegisterFail || !pKernel->RegisterInterface(pNewWorld.m_pLoadedMap, WorldID);
	RegisterFail = RegisterFail || !pKernel->RegisterInterface(static_cast<IMap*>(pNewWorld.m_pLoadedMap), WorldID);
	RegisterFail = RegisterFail || !pKernel->RegisterInterface(pNewWorld.m_pGameServer, WorldID);
	return RegisterFail;
}

bool CMultiWorlds::LoadWorlds(IServer* pServer, IKernel* pKernel, IStorageEngine* pStorage, IConsole* pConsole)
{
	// read file data into buffer
	char aFileBuf[512];
	str_format(aFileBuf, sizeof(aFileBuf), "maps/worlds.json");
	IOHANDLE File = pStorage->OpenFile(aFileBuf, IOFLAG_READ, IStorageEngine::TYPE_ALL);
	if(!File)
		return false;

	const int FileSize = (int)io_length(File);
	char* pFileData = (char*)malloc(FileSize);
	io_read(File, pFileData, FileSize);
	io_close(File);

	// parse json data
	json_settings JsonSettings;
	mem_zero(&JsonSettings, sizeof(JsonSettings));
	char aError[256];
	json_value* pJsonData = json_parse_ex(&JsonSettings, pFileData, FileSize, aError);
	free(pFileData);
	if(pJsonData == 0)
	{
		pConsole->Print(IConsole::OUTPUT_LEVEL_ADDINFO, "Error on reading \"worlds.json\"", aError);
		return false;
	}

	// extract data
	const json_value& rStart = (*pJsonData)["worlds"];
	if(rStart.type == json_array)
	{
		for(unsigned i = 0; i < rStart.u.array.length; ++i)
		{
			const char* pWorldName = rStart[i]["name"];
			const char* pPath = rStart[i]["path"];

			// here set worlds name
			Add(i, pKernel, pServer);
			str_copy(m_Worlds[i].m_aName, pWorldName, sizeof(m_Worlds[i].m_aName));
			str_copy(m_Worlds[i].m_aPath, pPath, sizeof(m_Worlds[i].m_aPath));
		}
	}

	// clean up
	json_value_free(pJsonData);
	return true;
}

void CMultiWorlds::Clear()
{
	for(int i = 0; i < m_WasInitilized; i++)
	{
		m_Worlds[i].m_pLoadedMap->Unload();
		delete m_Worlds[i].m_pLoadedMap;
		delete m_Worlds[i].m_pGameServer;
	}
	m_WasInitilized = 0;
}
