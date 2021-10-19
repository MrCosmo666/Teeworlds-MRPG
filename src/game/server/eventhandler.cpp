/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <base/system.h>

#include "eventhandler.h"
#include "gamecontext.h"

//////////////////////////////////////////////////
// Event handler
//////////////////////////////////////////////////
CEventHandler::CEventHandler()
{
	m_pGameServer = nullptr;
	Clear();
}

void CEventHandler::SetGameServer(CGS *pGameServer)
{
	m_pGameServer = pGameServer;
}

void *CEventHandler::Create(int Type, int Size, int64 Mask)
{
	if(m_NumEvents == MAX_EVENTS)
		return nullptr;
	if(m_CurrentOffset+Size >= MAX_DATASIZE)
		return nullptr;

	void *p = &m_aData[m_CurrentOffset];
	m_aOffsets[m_NumEvents] = m_CurrentOffset;
	m_aTypes[m_NumEvents] = Type;
	m_aSizes[m_NumEvents] = Size;
	m_aClientMasks[m_NumEvents] = Mask;
	m_CurrentOffset += Size;
	m_NumEvents++;
	return p;
}

void CEventHandler::Clear()
{
	m_NumEvents = 0;
	m_CurrentOffset = 0;
}

void CEventHandler::Snap(int SnappingClient)
{
	for(int i = 0; i < m_NumEvents; i++)
	{
		if(m_aTypes[i] > NETEVENTTYPE_DAMAGE && !GS()->IsMmoClient(SnappingClient))
			continue;

		if(SnappingClient == -1 || CmaskIsSet(m_aClientMasks[i], SnappingClient))
		{
			CNetEvent_Common *ev = (CNetEvent_Common *)&m_aData[m_aOffsets[i]];
			if(SnappingClient == -1 || distance(GS()->m_apPlayers[SnappingClient]->m_ViewPos, vec2(ev->m_X, ev->m_Y)) < 1500.0f)
			{
				void *d = GS()->Server()->SnapNewItem(m_aTypes[i], i, m_aSizes[i]);
				if(d)
					mem_copy(d, &m_aData[m_aOffsets[i]], m_aSizes[i]);
			}
		}
	}
}
