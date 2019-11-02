#ifndef ENGINE_CLIENT_UPDATER_H
#define ENGINE_CLIENT_UPDATER_H

#include <engine/updater.h>
#include <engine/client/http.h>
#include <map>
#include <string>

#define CLIENT_EXEC "mmoclient"

#if defined(CONF_FAMILY_WINDOWS)
	#define PLAT_EXT ".zip"
	#define PLAT_NAME CONF_PLATFORM_STRING
#elif defined(CONF_FAMILY_UNIX)
	#define PLAT_EXT ".tar.gz"
	#if defined(CONF_ARCH_IA32)
		#define PLAT_NAME CONF_PLATFORM_STRING "-x86"
	#elif defined(CONF_ARCH_AMD64)
		#define PLAT_NAME CONF_PLATFORM_STRING "-x86_64"
	#else
		#define PLAT_NAME CONF_PLATFORM_STRING "-unsupported"
	#endif
#else
	#define PLAT_EXT ""
	#define PLAT_NAME "unsupported-unsupported"
#endif

#define PLAT_CLIENT_DOWN CLIENT_EXEC "-" PLAT_NAME PLAT_EXT
#define PLAT_CLIENT_EXEC CLIENT_EXEC PLAT_EXT

class CUpdater : public IUpdater
{
	friend class CUpdaterFetchTask;

	class IClient *m_pClient;
	class IStorage *m_pStorage;
	class IEngine *m_pEngine;

	bool m_IsWinXP;

	LOCK m_Lock;

	int m_State;
	char m_aStatus[256];
	int m_Percent;
	char m_aLastFile[256];

	bool m_ClientUpdate;

	std::map<std::string, bool> m_FileJobs;

	void AddFileJob(const char *pFile, bool Job);
	void FetchFile(const char *pFile, const char *pDestPath = 0);
	bool MoveFile(const char *pFile);

	void ParseUpdate();
	void PerformUpdate();
	void CommitUpdate();

	bool ReplaceClient();
	void SetCurrentState(int NewState);

public:
	CUpdater();
	~CUpdater();

	int GetCurrentState();
	void GetCurrentFile(char *pBuf, int BufSize);
	int GetCurrentPercent();

	virtual void InitiateUpdate();
	void Init();
	virtual void Update();
	void WinXpRestart();
};

#endif
