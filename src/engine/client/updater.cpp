#include "updater.h"
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
	str_format(pBuf, BufSize, "https://mrpg.teeworlds.dev/update/%s", pFile);
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
	if(!str_comp(b, "mmotee-info.json"))
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
	m_pStorage = Kernel()->RequestInterface<IStorageEngine>();
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
	bool Success = true;
	m_pStorage->RemoveBinaryFile(pFile);

	str_format(aBuf, sizeof(aBuf), "update/%s", pFile);
	Success &= m_pStorage->RenameBinaryFile(aBuf, pFile);
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
	if(!m_IsWinXP)
	{
		m_pStorage->RemoveBinaryFile("mmoteeworlds.old");
		Success &= m_pStorage->RenameBinaryFile(PLAT_CLIENT_EXEC, "mmoteeworlds.old");
		Success &= m_pStorage->RenameBinaryFile("update/mmoteeworlds.tmp", PLAT_CLIENT_EXEC);
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
	IOHANDLE File = m_pStorage->OpenFile(m_pStorage->GetBinaryPath("mmotee-info.json", aPath, sizeof aPath), IOFLAG_READ, IStorageEngine::TYPE_ALL);
	if(!File)
		return;

	char aBuf[4096*4];
	mem_zero(aBuf, sizeof (aBuf));
	io_read(File, aBuf, sizeof(aBuf));
	io_close(File);

	json_value *pVersion = json_parse(aBuf, sizeof(aBuf));
	if(pVersion && pVersion->type == json_object)
	{
		const json_value* pVersionString = json_object_get(pVersion, "version");
		if(str_comp(json_string_get(pVersionString), GAME_RELEASE_VERSION))
			m_ArchiveUpdate = true;
		else
			m_State = FAIL;
	}
}

void CUpdater::InitiateUpdate()
{
	m_State = GOT_MANIFEST;
}

void CUpdater::PerformUpdate()
{
	m_State = PARSING_UPDATE;
	dbg_msg("updater", "parsing mmotee-info.json");
	ParseUpdate();
	m_State = DOWNLOADING;

	const char *aLastFile;
	aLastFile = "";
	if(m_ArchiveUpdate)
	{
		FetchFile(PLAT_CLIENT_DOWN, "mmoteeworlds.tmp");
		aLastFile = "mmoteeworlds.tmp";
	}
	str_copy(m_aLastFile, aLastFile, sizeof(m_aLastFile));
}

void CUpdater::CommitUpdate()
{
	bool Success = true;

	for(map<std::string, bool>::iterator it = m_FileJobs.begin(); it != m_FileJobs.end(); ++it)
		if(it->second)
			Success &= MoveFile(it->first.c_str());

	if(m_ArchiveUpdate)
		Success &= ReplaceClient();
	if(!Success)
		m_State = FAIL;
	else
		m_State = NEED_RESTART;
}