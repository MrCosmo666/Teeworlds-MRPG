/* (c) Magnus Auvinen. See licence.txt in the root of the distribution for more information. */
/* If you are missing that file, acquire a complete release at teeworlds.com.                */
#include <engine/map.h>
#include <engine/storage.h>
#include <game/mapitems.h>
#include "datafile.h"

class CMap : public IEngineMap
{
	int m_CurrentMapSize;
	unsigned char* m_pCurrentMapData;

	CDataFileReader m_DataFile;
public:
	CMap() : m_CurrentMapSize(0), m_pCurrentMapData(0x0) {}
	~CMap()
	{
		mem_free(m_pCurrentMapData);
		m_pCurrentMapData = 0x0;
	}

	virtual void *GetData(int Index) { return m_DataFile.GetData(Index); }
	virtual void *GetDataSwapped(int Index) { return m_DataFile.GetDataSwapped(Index); }
	virtual void UnloadData(int Index) { m_DataFile.UnloadData(Index); }
	virtual void *GetItem(int Index, int *pType, int *pID) { return m_DataFile.GetItem(Index, pType, pID); }
	virtual void GetType(int Type, int *pStart, int *pNum) { m_DataFile.GetType(Type, pStart, pNum); }
	virtual void *FindItem(int Type, int ID) { return m_DataFile.FindItem(Type, ID); }
	virtual int NumItems() { return m_DataFile.NumItems(); }

	virtual void SetCurrentMapSize(int Size) { m_CurrentMapSize = Size; }
	virtual int GetCurrentMapSize() { return m_CurrentMapSize; }

	virtual void SetCurrentMapData(unsigned char* CurrentMapData) { m_pCurrentMapData = CurrentMapData; }
	virtual unsigned char* GetCurrentMapData() { return m_pCurrentMapData; }

	virtual void Unload()
	{
		m_DataFile.Close();

		m_CurrentMapSize = 0;
		mem_free(m_pCurrentMapData);
		m_pCurrentMapData = 0x0;
	}

	virtual bool Load(const char *pMapName, IStorageEngine *pStorage)
	{
		if (!pStorage)
			pStorage = Kernel()->RequestInterface<IStorageEngine>();
		if (!pStorage)
			return false;
		if (!m_DataFile.Open(pStorage, pMapName, IStorageEngine::TYPE_ALL))
			return false;
		// check version
		CMapItemVersion *pItem = (CMapItemVersion *)m_DataFile.FindItem(MAPITEMTYPE_VERSION, 0);
		if (!pItem || pItem->m_Version != CMapItemVersion::CURRENT_VERSION)
			return false;

		// replace compressed tile layers with uncompressed ones
		int GroupsStart, GroupsNum, LayersStart, LayersNum;
		m_DataFile.GetType(MAPITEMTYPE_GROUP, &GroupsStart, &GroupsNum);
		m_DataFile.GetType(MAPITEMTYPE_LAYER, &LayersStart, &LayersNum);
		for (int g = 0; g < GroupsNum; g++)
		{
			CMapItemGroup *pGroup = static_cast<CMapItemGroup *>(m_DataFile.GetItem(GroupsStart + g, 0, 0));
			for (int l = 0; l < pGroup->m_NumLayers; l++)
			{
				CMapItemLayer *pLayer = static_cast<CMapItemLayer *>(m_DataFile.GetItem(LayersStart + pGroup->m_StartLayer + l, 0, 0));

				if (pLayer->m_Type == LAYERTYPE_TILES)
				{
					CMapItemLayerTilemap *pTilemap = reinterpret_cast<CMapItemLayerTilemap *>(pLayer);

					if (pTilemap->m_Version > 3)
					{
						const int TilemapCount = pTilemap->m_Width * pTilemap->m_Height;
						const int TilemapSize = TilemapCount * sizeof(CTile);

						if ((TilemapCount / pTilemap->m_Width != pTilemap->m_Height) || (TilemapSize / (int)sizeof(CTile) != TilemapCount))
						{
							dbg_msg("engine", "map layer too big (%d * %d * %u causes an integer overflow)", pTilemap->m_Width, pTilemap->m_Height, unsigned(sizeof(CTile)));
							return false;
						}
						CTile *pTiles = static_cast<CTile *>(mem_alloc(TilemapSize, 1));
						if (!pTiles)
							return false;

						// extract original tile data
						int i = 0;
						CTile *pSavedTiles = static_cast<CTile *>(m_DataFile.GetData(pTilemap->m_Data));
						while (i < TilemapCount)
						{
							for (unsigned Counter = 0; Counter <= pSavedTiles->m_Skip && i < TilemapCount; Counter++)
							{
								pTiles[i] = *pSavedTiles;
								pTiles[i++].m_Skip = 0;
							}

							pSavedTiles++;
						}

						m_DataFile.ReplaceData(pTilemap->m_Data, reinterpret_cast<char *>(pTiles));
					}
				}
			}

		}

		return true;
	}

	virtual bool IsLoaded()
	{
		return m_DataFile.IsOpen();
	}

	virtual SHA256_DIGEST Sha256()
	{
		return m_DataFile.Sha256();
	}

	virtual unsigned Crc()
	{
		return m_DataFile.Crc();
	}
};

extern IEngineMap *CreateEngineMap() { return new CMap; }
