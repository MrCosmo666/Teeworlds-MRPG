/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_CLIENT_COMPONENTS_PLAYERS_H
#define GAME_CLIENT_COMPONENTS_PLAYERS_H
#include <map>
#include <game/client/component.h>

class CPlayers : public CComponent
{
	struct EquipSlot
	{
		vec4 Color;
		vec2 Position;
		vec2 Size;
		int SpriteID;
	};
	std::map < int, EquipSlot > EquipInformation;

	CTeeRenderInfo m_aRenderInfo[MAX_CLIENTS];
	void RenderPlayer(
		const CNetObj_Character *pPrevChar,
		const CNetObj_Character *pPlayerChar,
		const CNetObj_PlayerInfo *pPrevInfo,
		const CNetObj_PlayerInfo *pPlayerInfo,
		int ClientID
	);
	void RenderHook(
		const CNetObj_Character *pPrevChar,
		const CNetObj_Character *pPlayerChar,
		const CNetObj_PlayerInfo *pPrevInfo,
		const CNetObj_PlayerInfo *pPlayerInfo,
		int ClientID
	);

	bool RenderWeaponsMRPG(const CNetObj_Character Player, CAnimState* pAnim, float Angle, vec2 Position, int ClientID);

	void RenderHammer(CAnimState* pAnim, float Angle, vec2 Position, int SpriteID, float Size);
	void RenderGun(const CNetObj_Character Player, CAnimState* pAnim, float Angle, vec2 Position, int SpriteID);
	void RenderShotgun(const CNetObj_Character Player, CAnimState* pAnim, float Angle, vec2 Position, int SpriteID);
	void RenderGrenade(CAnimState* pAnim, float Angle, vec2 Position, int SpriteID);
	void RenderRifle(CAnimState* pAnim, float Angle, vec2 Position, int SpriteID);
	void RenderWings(const CNetObj_Character Player, CAnimState* pAnimWings, vec2 Position, vec2 Direction, int ClientID);
public:
	virtual void OnRender();
	virtual void OnMessage(int MsgType, void* pRawMsg);
};

#endif
