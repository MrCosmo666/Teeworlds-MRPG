#include "updater.h"
#include <base/system.h>
#include <engine/engine.h>
#include <engine/storage.h>
#include <engine/client.h>
#include <engine/external/json-parser/json.h>
#include <game/version.h>

#include <stdlib.h> // system

using std::string;
using std::map;

class CUpdaterFetchTask : public CGetFile
{
	char m_aBuf[256];
	char m_aBuf2[256];
	CUpdater *m_pUpdater;

	void OnCompletion();
	void OnProgress();

public:
	CUpdaterFetchTask(CUpdater *pUpdater, const char *pFile, const char *pDestPath);
};

static const char *GetUpdaterUrl(char *pBuf, int BufSize, const char *pFile)
{
	str_format(pBuf, BufSize, "http://176.31.208.223/update/%s", pFile);
	return pBuf;
}

static const char *GetUpdaterDestPath(char *pBuf, int BufSize, const char *pFile, const char *pDestPath)
{
	if(!pDestPath)
	{
		pDestPath = pFile;
	}
	str_format(pBuf, BufSize, "update/%s", pDestPath);
	return pBuf;
}

CUpdaterFetchTask::CUpdaterFetchTask(CUpdater *pUpdater, const char *pFile, const char *pDestPath) :
	CGetFile(pUpdater->m_pStorage, GetUpdaterUrl(m_aBuf, sizeof(m_aBuf), pFile), GetUpdaterDestPath(m_aBuf2, sizeof(m_aBuf), pFile, pDestPath), -2, false),
	m_pUpdater(pUpdater)
{
}

void CUpdaterFetchTask::OnProgress()
{
	lock_wait(m_pUpdater->m_Lock);
	str_copy(m_pUpdater->m_aStatus, Dest(), sizeof(m_pUpdater->m_aStatus));
	m_pUpdater->m_Percent = Progress();
	lock_unlock(m_pUpdater->m_Lock);
}

void CUpdaterFetchTask::OnCompletion()
{
	const char *b = 0;
	for(const char *a = Dest(); *a; a++)
		if(*a == '/')
			b = a + 1;
	b = b ? b : Dest();
	if(!str_comp(b, "update.json"))
	{
		if(State() == HTTP_DONE)
			m_pUpdater->SetCurrentState(IUpdater::GOT_MANIFEST);
		else if(State() == HTTP_ERROR)
			m_pUpdater->SetCurrentState(IUpdater::FAIL);
	}
	else if(!str_comp(b, m_pUpdater->m_aLastFile))
	{
		if(State() == HTTP_DONE)
			m_pUpdater->SetCurrentState(IUpdater::MOVE_FILES);
		else if(State() == HTTP_ERROR)
			m_pUpdater->SetCurrentState(IUpdater::FAIL);
	}
}

CUpdater::CUpdater()
{
	m_pClient = NULL;
	m_pStorage = NULL;
	m_pEngine = NULL;
	m_State = CLEAN;
	m_Percent = 0;
	m_Lock = lock_create();
}

void CUpdater::Init()
{
	m_pClient = Kernel()->RequestInterface<IClient>();
	m_pStorage = Kernel()->RequestInterface<IStorage>();
	m_pEngine = Kernel()->RequestInterface<IEngine>();
	m_IsWinXP = os_is_winxp_or_lower();
}

CUpdater::~CUpdater()
{
	lock_destroy(m_Lock);
}

void CUpdater::SetCurrentState(int NewState)
{
	lock_wait(m_Lock);
	m_State = NewState;
	lock_unlock(m_Lock);
}

int CUpdater::GetCurrentState()
{
	lock_wait(m_Lock);
	int Result = m_State;
	lock_unlock(m_Lock);
	return Result;
}

void CUpdater::GetCurrentFile(char *pBuf, int BufSize)
{
	lock_wait(m_Lock);
	str_copy(pBuf, m_aStatus, BufSize);
	lock_unlock(m_Lock);
}

int CUpdater::GetCurrentPercent()
{
	lock_wait(m_Lock);
	int Result = m_Percent;
	lock_unlock(m_Lock);
	return Result;
}

void CUpdater::FetchFile(const char *pFile, const char *pDestPath)
{
	m_pEngine->AddJob(std::make_shared<CUpdaterFetchTask>(this, pFile, pDestPath));
}

bool CUpdater::MoveFile(const char *pFile)
{
	char aBuf[256];
	size_t len = str_length(pFile);
	bool Success = true;

#if !defined(CONF_FAMILY_WINDOWS)
	if(!str_comp_nocase(pFile + len - 4, ".dll"))
		return Success;
#endif

	if(!str_comp_nocase(pFile + len - 4, ".dll") || !str_comp_nocase(pFile + len - 4, ".ttf"))
	{
		str_format(aBuf, sizeof(aBuf), "%s.old", pFile);
		Success &= m_pStorage->RenameBinaryFile(pFile, aBuf);

		str_format(aBuf, sizeof(aBuf), "update/%s", pFile);
		Success &= m_pStorage->RenameBinaryFile(aBuf, pFile);
	}
	else
	{
		m_pStorage->RemoveBinaryFile(pFile);

		str_format(aBuf, sizeof(aBuf), "update/%s", pFile);
		Success &= m_pStorage->RenameBinaryFile(aBuf, pFile);
	}

	return Success;
}

void CUpdater::Update()
{
	switch(m_State)
	{
		case IUpdater::GOT_MANIFEST:
			PerformUpdate();
			break;
		case IUpdater::MOVE_FILES:
			CommitUpdate();
			break;
		default:
			return;
	}
}

void CUpdater::AddFileJob(const char *pFile, bool Job)
{
	m_FileJobs[string(pFile)] = Job;
}

bool CUpdater::ReplaceClient()
{
	dbg_msg("updater", "replacing " PLAT_CLIENT_EXEC);
	bool Success = true;

	// Replace running executable by renaming twice...
	if(!m_IsWinXP)
	{
		m_pStorage->RemoveBinaryFile("mmoclient.old");
		Success &= m_pStorage->RenameBinaryFile(PLAT_CLIENT_EXEC, "mmoclient.old");
		Success &= m_pStorage->RenameBinaryFile("update/mmoclient.tmp", PLAT_CLIENT_EXEC);
	}
	#if !defined(CONF_FAMILY_WINDOWS)
		char aPath[512];
		m_pStorage->GetBinaryPath(PLAT_CLIENT_EXEC, aPath, sizeof aPath);
		char aBuf[512];
		str_format(aBuf, sizeof aBuf, "chmod +x %s", aPath);
		if(system(aBuf))
		{
			dbg_msg("updater", "ERROR: failed to set client executable bit");
			Success &= false;
		}
	#endif
	return Success;
}

void CUpdater::ParseUpdate()
{
	char aPath[512];
	IOHANDLE File = m_pStorage->OpenFile(m_pStorage->GetBinaryPath("update/update.json", aPath, sizeof aPath), IOFLAG_READ, IStorage::TYPE_ALL);
	if(!File)
		return;

	char aBuf[4096*4];
	mem_zero(aBuf, sizeof (aBuf));
	io_read(File, aBuf, sizeof(aBuf));
	io_close(File);

	json_value *pVersions = json_parse(aBuf, sizeof(aBuf));
	if(pVersions && pVersions->type == json_array)
	{
		for(int i = 0; i < json_array_length(pVersions); i++)
		{
			const json_value *pTemp;
			const json_value *pCurrent = json_array_get(pVersions, i);
			if(str_comp(json_string_get(json_object_get(pCurrent, "version")), GAME_RELEASE_VERSION))
			{
				if(json_boolean_get(json_object_get(pCurrent, "client")))
					m_ClientUpdate = true;
				if((pTemp = json_object_get(pCurrent, "download"))->type == json_array)
				{
					for(int j = 0; j < json_array_length(pTemp); j++)
						AddFileJob(json_string_get(json_array_get(pTemp, j)), true);
				}
				if((pTemp = json_object_get(pCurrent, "remove"))->type == json_array)
				{
					for(int j = 0; j < json_array_length(pTemp); j++)
						AddFileJob(json_string_get(json_array_get(pTemp, j)), false);
				}
			}
			else
				break;
		}
	}
}

void CUpdater::InitiateUpdate()
{
	m_State = GETTING_MANIFEST;
	FetchFile("update.json");
}

void CUpdater::PerformUpdate()
{
	m_State = PARSING_UPDATE;
	dbg_msg("updater", "parsing update.json");
	ParseUpdate();
	m_State = DOWNLOADING;

	const char *aLastFile;
	aLastFile = "";
	for(map<string, bool>::reverse_iterator it = m_FileJobs.rbegin(); it != m_FileJobs.rend(); ++it)
	{
		if(it->second)
		{
			aLastFile = it->first.c_str();
			break;
		}
	}

	for(map<string, bool>::iterator it = m_FileJobs.begin(); it != m_FileJobs.end(); ++it)
	{
		if(it->second)
		{
			const char *pFile = it->first.c_str();
			size_t len = str_length(pFile);
			if(!str_comp_nocase(pFile + len - 4, ".dll"))
			{
#if defined(CONF_FAMILY_WINDOWS)
				char aBuf[512];
				str_copy(aBuf, pFile, sizeof(aBuf)); // SDL
				str_copy(aBuf + len - 4, "-" PLAT_NAME, sizeof(aBuf) - len + 4); // -win32
				str_append(aBuf, pFile + len - 4, sizeof(aBuf)); // .dll
				FetchFile(aBuf, pFile);
#endif
				// Ignore DLL downloads on other platforms, on Linux we statically link anyway
			}
			else
			{
				FetchFile(pFile);
			}
			aLastFile = pFile;
		}
		else
			m_pStorage->RemoveBinaryFile(it->first.c_str());
	}

	if(m_ClientUpdate)
	{
		FetchFile(PLAT_CLIENT_DOWN, "mmoclient.tmp");
		aLastFile = "mmoclient.tmp";
	}

	str_copy(m_aLastFile, aLastFile, sizeof(m_aLastFile));
}

void CUpdater::CommitUpdate()
{
	bool Success = true;

	for(map<std::string, bool>::iterator it = m_FileJobs.begin(); it != m_FileJobs.end(); ++it)
		if(it->second)
			Success &= MoveFile(it->first.c_str());

	if(m_ClientUpdate)
		Success &= ReplaceClient();
	if(!Success)
		m_State = FAIL;
	else if(m_pClient->State() == IClient::STATE_ONLINE || m_pClient->EditorHasUnsavedData())
		m_State = NEED_RESTART;
	else
		m_State = NEED_RESTART;
}