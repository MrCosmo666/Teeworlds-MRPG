/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#ifndef GAME_CLIENT_COMPONENTS_SKINCHANGER_H
#define GAME_CLIENT_COMPONENTS_SKINCHANGER_H
#include <game/client/component.h>

class CCSkinChanger : public CComponent
{
public:
	CCSkinChanger();
	void DelayedInit(); // forwards to subcomponents

	// abstract texture-type entity
	class CTextureEntity : public CComponent
	{
	protected:
		enum
		{
			MAX_TEXTURES = 64,
		};
		struct
		{
			IGraphics::CTextureHandle m_aTextures;
			char m_aName[256];
		} m_Info[MAX_TEXTURES];
		IGraphics::CTextureHandle m_DefaultTexture;
		IGraphics::CTextureHandle m_InitialTexture; // store the texture at init there if it is custom (redundant)
		int m_Count;

		static int EntityScan(const char *pName, int IsDir, int DirType, void *pUser);
		virtual bool LoadEntity(const char *pName, int DirType, IGraphics::CTextureHandle *pTexture) = 0;

	public:
		virtual void DelayedInit() = 0;
		virtual void LoadEntities() = 0;
		virtual void Reload(int id) = 0; // < 0 = default
		IGraphics::CTextureHandle Get(int Index) const;
		IGraphics::CTextureHandle GetDefault() const;
		const char* GetName(int Index) const;
		int Num() const;
	};

	// game skins
	class CGameSkins : public CTextureEntity
	{
		virtual bool LoadEntity(const char *pName, int DirType, IGraphics::CTextureHandle *pTexture);
	public:
		bool m_IsLoaded;

		CGameSkins();
		bool IsLoaded() const { return m_IsLoaded; };
		virtual void DelayedInit();
		virtual void LoadEntities();
		virtual void Reload(int id);
	} m_GameSkins;

	// particles
	class CParticles : public CTextureEntity
	{
		virtual bool LoadEntity(const char *pName, int DirType, IGraphics::CTextureHandle *pTexture);
	public:
		bool m_IsLoaded;

		CParticles();
		bool IsLoaded() const { return m_IsLoaded; };
		virtual void DelayedInit();
		virtual void LoadEntities();
		virtual void Reload(int id);
	} m_Particles;

	// cursors
	class CCursors : public CTextureEntity
	{
		virtual bool LoadEntity(const char *pName, int DirType, IGraphics::CTextureHandle *pTexture);
	public:
		bool m_IsLoaded;

		CCursors();
		bool IsLoaded() const { return m_IsLoaded; };
		virtual void DelayedInit();
		virtual void LoadEntities();
		virtual void Reload(int id);
	} m_Cursors;

	// emoticons
	class CEmoticons : public CTextureEntity
	{
		virtual bool LoadEntity(const char *pName, int DirType, IGraphics::CTextureHandle *pTexture);
	public:
		bool m_IsLoaded;

		CEmoticons();
		bool IsLoaded() const { return m_IsLoaded; };
		virtual void DelayedInit();
		virtual void LoadEntities();
		virtual void Reload(int id);
	} m_Emoticons;

	// entities
	class CEntities : public CTextureEntity
	{
		virtual bool LoadEntity(const char *pName, int DirType, IGraphics::CTextureHandle *pTexture);
	public:
		bool m_IsLoaded;

		CEntities();
		bool IsLoaded() const { return m_IsLoaded; };
		virtual void DelayedInit();
		virtual void LoadEntities();
		virtual void Reload(int id);
	} m_Entities;
};

#endif