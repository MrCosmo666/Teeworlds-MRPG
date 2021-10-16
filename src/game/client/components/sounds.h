/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_CLIENT_COMPONENTS_SOUNDS_H
#define GAME_CLIENT_COMPONENTS_SOUNDS_H

#include <engine/engine.h>
#include <engine/sound.h>
#include <game/client/component.h>


class CSoundLoading : public IJob
{
	CGameClient *m_pGameClient;
	bool m_Render;

public:
	CSoundLoading(CGameClient *pGameClient, bool Render);
	void Run();
};

class CSounds : public CComponent
{
	enum
	{
		QUEUE_SIZE = 32,
	};
	struct QueueEntry
	{
		int m_Channel;
		int m_SetId;
	} m_aQueue[QUEUE_SIZE];
	int m_QueuePos;
	int64 m_QueueWaitTime;
	std::shared_ptr<CSoundLoading> m_pSoundJob;
	bool m_WaitForSoundJob;

	ISound::CSampleHandle GetSampleId(int SetId) const;

public:
	// sound channels
	enum
	{
		CHN_GUI=0,
		CHN_MUSIC,
		CHN_WORLD,
		CHN_GLOBAL,
		CHN_MMORPG,
		CHN_MMORPG_ATMOSPHERE,
	};

	virtual int GetInitAmount() const;
	virtual void OnInit();
	virtual void OnReset();
	virtual void OnStateChange(int NewState, int OldState);
	virtual void OnRender();

	void ClearQueue();
	void SetChannelVolume(int Channel, float Vol);
	void Enqueue(int Channel, int SetId);
	void Play(int Channel, int SetId, float Vol);
	void PlayAt(int Channel, int SetId, float Vol, vec2 Pos);
	void Stop(int SetId);
	bool IsPlaying(int SetId) const;
};


#endif
