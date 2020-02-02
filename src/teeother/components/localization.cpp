#include "localization.h"

#include <engine/external/json-parser/json.h>
#include <engine/storage.h>
#include <unicode/ushape.h>
#include <unicode/ubidi.h>

CLocalization::CLanguage::CLanguage() 
: m_Loaded(false), m_Direction(CLocalization::DIRECTION_LTR), m_pPluralRules(NULL),
m_pNumberFormater(NULL), m_pPercentFormater(NULL), m_pTimeUnitFormater(NULL), m_pValueFormater(NULL)
{
	m_aName[0] = 0;
	m_aFilename[0] = 0;
	m_aParentFilename[0] = 0;
}

CLocalization::CLanguage::CLanguage(const char* pName, const char* pFilename, const char* pParentFilename) 
: m_Loaded(false), m_Direction(CLocalization::DIRECTION_LTR), m_pPluralRules(NULL),
m_pNumberFormater(NULL), m_pPercentFormater(NULL), m_pValueFormater(NULL)
{
	str_copy(m_aName, pName, sizeof(m_aName));
	str_copy(m_aFilename, pFilename, sizeof(m_aFilename));
	str_copy(m_aParentFilename, pParentFilename, sizeof(m_aParentFilename));
	
	// - - - - - �������������� �������������� ICU
	UErrorCode Status;
	Status = U_ZERO_ERROR;
	m_pValueFormater = unum_open(UNUM_DECIMAL_COMPACT_SHORT, NULL, -1, m_aFilename, NULL, &Status);
	if(U_FAILURE(Status))
	{
		if(m_pValueFormater)
		{
			unum_close(m_pValueFormater);
			m_pValueFormater = NULL;
		}
		dbg_msg("Localization", "Can't create value formater for %s (error #%d)", m_aFilename, Status);
	}	

	// - - - - - �������������� �������������� ICU
	Status = U_ZERO_ERROR;
	m_pNumberFormater = unum_open(UNUM_DECIMAL, NULL, -1, m_aFilename, NULL, &Status);
	if(U_FAILURE(Status))
	{
		if(m_pNumberFormater)
		{
			unum_close(m_pNumberFormater);
			m_pNumberFormater = NULL;
		}
		dbg_msg("Localization", "Can't create number formater for %s (error #%d)", m_aFilename, Status);
	}

	// - - - - - �������������� �������������� ICU
	Status = U_ZERO_ERROR;
	m_pPercentFormater = unum_open(UNUM_PERCENT, NULL, -1, m_aFilename, NULL, &Status);
	if(U_FAILURE(Status))
	{
		if(m_pPercentFormater)
		{
			unum_close(m_pPercentFormater);
			m_pPercentFormater = NULL;
		}
		dbg_msg("Localization", "Can't create percent formater for %s (error #%d)", m_aFilename, Status);
	}
	
	// - - - - - �������������� �������������� ICU
	Status = U_ZERO_ERROR;
	m_pPluralRules = uplrules_openForType(m_aFilename, UPLURAL_TYPE_CARDINAL, &Status);
	if(U_FAILURE(Status))
	{
		if(m_pPluralRules)
		{
			uplrules_close(m_pPluralRules);
			m_pPluralRules = NULL;
		}
		dbg_msg("Localization", "Can't create plural rules for %s (error #%d)", m_aFilename, Status);
	}
	
	// - - - - - �������������� �������������� ICU
	Status = U_ZERO_ERROR;
	m_pTimeUnitFormater = new icu::TimeUnitFormat(m_aFilename,  UTMUTFMT_ABBREVIATED_STYLE, Status);
	if(U_FAILURE(Status))
	{
		dbg_msg("Localization", "Can't create timeunit formater %s (error #%d)", pFilename, Status);
		delete m_pTimeUnitFormater;
		m_pTimeUnitFormater = NULL;
	}
}

CLocalization::CLanguage::~CLanguage()
{
	hashtable< CEntry, 128 >::iterator Iter = m_Translations.begin();
	while(Iter != m_Translations.end())
	{
		if(Iter.data())
			Iter.data()->Free();
		
		++Iter;
	}
	
	if(m_pNumberFormater)
		unum_close(m_pNumberFormater);
	
	if (m_pValueFormater)
		unum_close(m_pValueFormater);

	if(m_pPercentFormater)
		unum_close(m_pPercentFormater);
		
	if(m_pPluralRules)
		uplrules_close(m_pPluralRules);
		
	if(m_pTimeUnitFormater)
		delete m_pTimeUnitFormater;
}

bool CLocalization::CLanguage::Load(CLocalization* pLocalization, CStorage* pStorage)
{
	// read file data into buffer
	char aBuf[256];
	str_format(aBuf, sizeof(aBuf), "./server_lang/%s.json", m_aFilename);
	IOHANDLE File = pStorage->OpenFile(aBuf, IOFLAG_READ, CStorage::TYPE_ALL);
	if(!File)
		return false;

	int FileSize = (int)io_length(File);
	char* pFileData = (char*)mem_alloc(FileSize, 1);
	io_read(File, pFileData, FileSize);
	io_close(File);

	// parse json data
	json_settings JsonSettings;
	mem_zero(&JsonSettings, sizeof(JsonSettings));
	char aError[256];
	json_value* pJsonData = json_parse_ex(&JsonSettings, pFileData, FileSize, aError);
	mem_free(pFileData);

	if(pJsonData == 0)
	{
		dbg_msg("Localization", "Can't load the localization file %s : %s", aBuf, aError);
		return false;
	}
	
	dynamic_string Buffer;
	int Length;

	// extract data
	const json_value &rStart = (*pJsonData)["translation"];
	if(rStart.type == json_array)
	{
		for(unsigned i = 0; i < rStart.u.array.length; ++i)
		{
			const char* pKey = rStart[i]["key"];
			if(pKey && pKey[0])
			{
				CEntry* pEntry = m_Translations.set(pKey);
				
				const char* pSingular = rStart[i]["value"];
				if(pSingular && pSingular[0])
				{
					Length = str_length(pSingular)+1;
					pEntry->m_apVersions[PLURALTYPE_NONE] = new char[Length];
					str_copy(pEntry->m_apVersions[PLURALTYPE_NONE], pSingular, Length);
				}
				else
				{
					const char* pPlural;
					
					//Zero
					pPlural = rStart[i]["zero"];
					if(pPlural && pPlural[PLURALTYPE_ZERO])
					{
						Length = str_length(pPlural)+1;
						pEntry->m_apVersions[PLURALTYPE_ZERO] = new char[Length];
						str_copy(pEntry->m_apVersions[PLURALTYPE_ZERO], pPlural, Length);
					}
					//One
					pPlural = rStart[i]["one"];
					if(pPlural && pPlural[PLURALTYPE_ONE])
					{
						Length = str_length(pPlural)+1;
						pEntry->m_apVersions[PLURALTYPE_ONE] = new char[Length];
						str_copy(pEntry->m_apVersions[PLURALTYPE_ONE], pPlural, Length);
					}
					//Two
					pPlural = rStart[i]["two"];
					if(pPlural && pPlural[PLURALTYPE_TWO])
					{
						Length = str_length(pPlural)+1;
						pEntry->m_apVersions[PLURALTYPE_TWO] = new char[Length];
						str_copy(pEntry->m_apVersions[PLURALTYPE_TWO], pPlural, Length);
					}
					//Few
					pPlural = rStart[i]["few"];
					if(pPlural && pPlural[PLURALTYPE_FEW])
					{
						Length = str_length(pPlural)+1;
						pEntry->m_apVersions[PLURALTYPE_FEW] = new char[Length];
						str_copy(pEntry->m_apVersions[PLURALTYPE_FEW], pPlural, Length);
					}
					//Many
					pPlural = rStart[i]["many"];
					if(pPlural && pPlural[PLURALTYPE_MANY])
					{
						Length = str_length(pPlural)+1;
						pEntry->m_apVersions[PLURALTYPE_MANY] = new char[Length];
						str_copy(pEntry->m_apVersions[PLURALTYPE_MANY], pPlural, Length);
					}
					//Other
					pPlural = rStart[i]["other"];
					if(pPlural && pPlural[PLURALTYPE_OTHER])
					{
						Length = str_length(pPlural)+1;
						pEntry->m_apVersions[PLURALTYPE_OTHER] = new char[Length];
						str_copy(pEntry->m_apVersions[PLURALTYPE_OTHER], pPlural, Length);
					}
				}
			}
		}
	}

	// clean up
	json_value_free(pJsonData);
	m_Loaded = true;
	
	return true;
}

const char* CLocalization::CLanguage::Localize(const char* pText) const
{	
	const CEntry* pEntry = m_Translations.get(pText);
	if(!pEntry)
		return NULL;
	
	return pEntry->m_apVersions[PLURALTYPE_NONE];
}

const char* CLocalization::CLanguage::Localize_P(int Number, const char* pText) const
{
	const CEntry* pEntry = m_Translations.get(pText);
	if(!pEntry)
		return NULL;
	
	UChar aPluralKeyWord[6];
	UErrorCode Status = U_ZERO_ERROR;
	uplrules_select(m_pPluralRules, static_cast<double>(Number), aPluralKeyWord, 6, &Status);
	
	if(U_FAILURE(Status))
		return NULL;
	
	int PluralCode = PLURALTYPE_NONE;
	
	if(aPluralKeyWord[0] == 0x007A) //z
		PluralCode = PLURALTYPE_ZERO;
	else if(aPluralKeyWord[0] == 0x0074) //t
		PluralCode = PLURALTYPE_TWO;
	else if(aPluralKeyWord[0] == 0x0066) //f
		PluralCode = PLURALTYPE_FEW;
	else if(aPluralKeyWord[0] == 0x006D) //m
		PluralCode = PLURALTYPE_MANY;
	else if(aPluralKeyWord[0] == 0x006F) //o
	{
		if(aPluralKeyWord[1] == 0x0074) //t
			PluralCode = PLURALTYPE_OTHER;
		else if(aPluralKeyWord[1] == 0x006E) //n
			PluralCode = PLURALTYPE_ONE;
	}
	
	return pEntry->m_apVersions[PluralCode];
}

/* LOCALIZATION *******************************************************/

/* BEGIN EDIT *********************************************************/
CLocalization::CLocalization(class CStorage* pStorage) :
	m_pStorage(pStorage),
	m_pMainLanguage(NULL),
	m_pUtf8Converter(NULL)
{
	
}
/* END EDIT ***********************************************************/

CLocalization::~CLocalization()
{
	for(int i=0; i<m_pLanguages.size(); i++)
		delete m_pLanguages[i];
	
	if(m_pUtf8Converter)
		ucnv_close(m_pUtf8Converter);
}

/* BEGIN EDIT *********************************************************/
bool CLocalization::InitConfig(int argc, const char** argv)
{
	m_Cfg_MainLanguage.copy("en");
	
	return true;
}

/* END EDIT ***********************************************************/

bool CLocalization::Init()
{
	UErrorCode Status = U_ZERO_ERROR;
	m_pUtf8Converter = ucnv_open("utf8", &Status);
	if(U_FAILURE(Status))
	{
		dbg_msg("Localization", "Can't create UTF8/UTF16 convertert");
		return false;
	}
	
	// read file data into buffer
	const char *pFilename = "./server_lang/index.json";
	IOHANDLE File = Storage()->OpenFile(pFilename, IOFLAG_READ, CStorage::TYPE_ALL);
	if(!File)
	{
		dbg_msg("Localization", "can't open ./server_lang/index.json");
		return false;
	}
	

	int FileSize = (int)io_length(File);
	char* pFileData = (char*)mem_alloc(FileSize, 1);
	io_read(File, pFileData, FileSize);
	io_close(File);

	// parse json data
	json_settings JsonSettings;
	mem_zero(&JsonSettings, sizeof(JsonSettings));
	char aError[256];
	json_value* pJsonData = json_parse_ex(&JsonSettings, pFileData, FileSize, aError);
	mem_free(pFileData);
	if(pJsonData == 0)
	{
		delete[] pFileData;
		return true; //return true because it's not a critical error
	}

	// extract data
	m_pMainLanguage = 0;
	const json_value &rStart = (*pJsonData)["language indices"];
	if(rStart.type == json_array)
	{
		for(unsigned i = 0; i < rStart.u.array.length; ++i)
		{
			CLanguage*& pLanguage = m_pLanguages.increment();
			pLanguage = new CLanguage((const char *)rStart[i]["name"], (const char *)rStart[i]["file"], (const char *)rStart[i]["parent"]);
				
			if((const char *)rStart[i]["direction"] && str_comp((const char *)rStart[i]["direction"], "rtl") == 0)
				pLanguage->SetWritingDirection(DIRECTION_RTL);
				
			if(m_Cfg_MainLanguage == pLanguage->GetFilename())
			{
				pLanguage->Load(this, Storage());
				
				m_pMainLanguage = pLanguage;
			}
		}
	}

	// clean up
	json_value_free(pJsonData);
	return true;
}
	
void CLocalization::AddListener(IListener* pListener)
{
	m_pListeners.increment() = pListener;
}

void CLocalization::RemoveListener(IListener* pListener)
{
	for(int i=0; i<m_pListeners.size(); i++)
	{
		if(m_pListeners[i] == pListener)
			m_pListeners.remove_index(i);
	}
}

bool CLocalization::PreUpdate()
{
	if(!m_pMainLanguage || m_Cfg_MainLanguage != m_pMainLanguage->GetFilename())
	{
		CLanguage* pLanguage = 0;
		
		for(int i=0; i<m_pLanguages.size(); i++)
		{
			if(m_Cfg_MainLanguage == m_pLanguages[i]->GetFilename())
			{
				pLanguage = m_pLanguages[i];
				break;
			}
		}
		
		if(m_pMainLanguage != pLanguage)
		{
			m_pMainLanguage = pLanguage;
			
			for(int i=0; i<m_pListeners.size(); i++)
				m_pListeners[i]->OnLocalizationModified();
		}
	}
	
	return true;
}

const char* CLocalization::LocalizeWithDepth(const char* pLanguageCode, const char* pText, int Depth)
{
	CLanguage* pLanguage = m_pMainLanguage;
	if(pLanguageCode)
	{
		for(int i=0; i<m_pLanguages.size(); i++)
		{
			if(str_comp(m_pLanguages[i]->GetFilename(), pLanguageCode) == 0)
			{
				pLanguage = m_pLanguages[i];
				break;
			}
		}
	}
	
	if(!pLanguage)
		return pText;
	
	if(!pLanguage->IsLoaded())
		pLanguage->Load(this, Storage());
	
	const char* pResult = pLanguage->Localize(pText);
	if(pResult)
		return pResult;
	else if(pLanguage->GetParentFilename()[0] && Depth < 4)
		return LocalizeWithDepth(pLanguage->GetParentFilename(), pText, Depth+1);
	else
		return pText;
}

const char* CLocalization::Localize(const char* pLanguageCode, const char* pText)
{
	return LocalizeWithDepth(pLanguageCode, pText, 0);
}

const char* CLocalization::LocalizeWithDepth_P(const char* pLanguageCode, int Number, const char* pText, int Depth)
{
	CLanguage* pLanguage = m_pMainLanguage;
	if(pLanguageCode)
	{
		for(int i=0; i<m_pLanguages.size(); i++)
		{
			if(str_comp(m_pLanguages[i]->GetFilename(), pLanguageCode) == 0)
			{
				pLanguage = m_pLanguages[i];
				break;
			}
		}
	}
	
	if(!pLanguage)
		return pText;
	
	if(!pLanguage->IsLoaded())
		pLanguage->Load(this, Storage());
	
	const char* pResult = pLanguage->Localize_P(Number, pText);
	if(pResult)
		return pResult;
	else if(pLanguage->GetParentFilename()[0] && Depth < 4)
		return LocalizeWithDepth_P(pLanguage->GetParentFilename(), Number, pText, Depth+1);
	else
		return pText;
}

const char* CLocalization::Localize_P(const char* pLanguageCode, int Number, const char* pText)
{
	return LocalizeWithDepth_P(pLanguageCode, Number, pText, 0);
}

void CLocalization::AppendNumber(dynamic_string& Buffer, int& BufferIter, CLanguage* pLanguage, int Number)
{
	UChar aBufUtf16[128];
	
	UErrorCode Status = U_ZERO_ERROR;
	unum_format(pLanguage->m_pNumberFormater, Number, aBufUtf16, sizeof(aBufUtf16), NULL, &Status);

	if(U_FAILURE(Status))
		BufferIter = Buffer.append_at(BufferIter, "_NUMBER_");
	else
	{
		//Update buffer size
		int SrcLength = u_strlen(aBufUtf16);
		int NeededSize = UCNV_GET_MAX_BYTES_FOR_STRING(SrcLength, ucnv_getMaxCharSize(m_pUtf8Converter));
		
		while(Buffer.maxsize() - BufferIter <= NeededSize)
			Buffer.resize_buffer(Buffer.maxsize()*2);
		
		int Length = ucnv_fromUChars(m_pUtf8Converter, Buffer.buffer()+BufferIter, Buffer.maxsize() - BufferIter, aBufUtf16, SrcLength, &Status);
		if(U_FAILURE(Status))
			BufferIter = Buffer.append_at(BufferIter, "_NUMBER_");
		else
			BufferIter += Length;
	}
}

void CLocalization::AppendValue(dynamic_string& Buffer, int& BufferIter, CLanguage* pLanguage, int Number)
{
	UChar aBufUtf16[128];
	
	UErrorCode Status = U_ZERO_ERROR;
	unum_format(pLanguage->m_pValueFormater, Number, aBufUtf16, sizeof(aBufUtf16), NULL, &Status);
	if(U_FAILURE(Status))
		BufferIter = Buffer.append_at(BufferIter, "_VALUE_");
	else
	{
		//Update buffer size
		int SrcLength = u_strlen(aBufUtf16);
		int NeededSize = UCNV_GET_MAX_BYTES_FOR_STRING(SrcLength, ucnv_getMaxCharSize(m_pUtf8Converter));
		
		while(Buffer.maxsize() - BufferIter <= NeededSize)
			Buffer.resize_buffer(Buffer.maxsize()*2);
		
		int Length = ucnv_fromUChars(m_pUtf8Converter, Buffer.buffer()+BufferIter, Buffer.maxsize() - BufferIter, aBufUtf16, SrcLength, &Status);
		if(U_FAILURE(Status))
			BufferIter = Buffer.append_at(BufferIter, "_VALUE_");
		else
			BufferIter += Length;
	}
}

void CLocalization::AppendPercent(dynamic_string& Buffer, int& BufferIter, CLanguage* pLanguage, double Number)
{
	UChar aBufUtf16[128];
	
	UErrorCode Status = U_ZERO_ERROR;
	unum_formatDouble(pLanguage->m_pPercentFormater, Number, aBufUtf16, sizeof(aBufUtf16), NULL, &Status);
	if(U_FAILURE(Status))
		BufferIter = Buffer.append_at(BufferIter, "_PERCENT_");
	else
	{
		//Update buffer size
		int SrcLength = u_strlen(aBufUtf16);
		int NeededSize = UCNV_GET_MAX_BYTES_FOR_STRING(SrcLength, ucnv_getMaxCharSize(m_pUtf8Converter));
		
		while(Buffer.maxsize() - BufferIter <= NeededSize)
			Buffer.resize_buffer(Buffer.maxsize()*2);
		
		int Length = ucnv_fromUChars(m_pUtf8Converter, Buffer.buffer()+BufferIter, Buffer.maxsize() - BufferIter, aBufUtf16, SrcLength, &Status);
		if(U_FAILURE(Status))
			BufferIter = Buffer.append_at(BufferIter, "_PERCENT_");
		else
			BufferIter += Length;
	}
}

void CLocalization::AppendDuration(dynamic_string& Buffer, int& BufferIter, CLanguage* pLanguage, int Number, icu::TimeUnit::UTimeUnitFields Type)
{
	UErrorCode Status = U_ZERO_ERROR;
	icu::UnicodeString BufUTF16;
	
	icu::TimeUnitAmount* pAmount = new icu::TimeUnitAmount((double) Number, Type, Status);
	icu::Formattable Formattable;
	Formattable.adoptObject(pAmount);
	pLanguage->m_pTimeUnitFormater->format(Formattable, BufUTF16, Status);
	
	if(U_FAILURE(Status))
		BufferIter = Buffer.append_at(BufferIter, "_DURATION_");
	else
	{
		int SrcLength = BufUTF16.length();
		
		int NeededSize = UCNV_GET_MAX_BYTES_FOR_STRING(SrcLength, ucnv_getMaxCharSize(m_pUtf8Converter));
		
		while(Buffer.maxsize() - BufferIter <= NeededSize)
			Buffer.resize_buffer(Buffer.maxsize()*2);
		
		Status = U_ZERO_ERROR;
		int Length = ucnv_fromUChars(m_pUtf8Converter, Buffer.buffer()+BufferIter, Buffer.maxsize() - BufferIter, BufUTF16.getBuffer(), SrcLength, &Status);
		
		if(U_FAILURE(Status))
			BufferIter = Buffer.append_at(BufferIter, "_DURATION_");
		else
			BufferIter += Length;
	}
}

void CLocalization::Format_V(dynamic_string& Buffer, const char* pLanguageCode, const char* pText, va_list VarArgs)
{
	CLanguage* pLanguage = m_pMainLanguage;	
	if(pLanguageCode)
	{
		for(int i=0; i<m_pLanguages.size(); i++)
		{
			if (str_comp(m_pLanguages[i]->GetFilename(), pLanguageCode) != 0)
				continue;
			
			pLanguage = m_pLanguages[i];
			break;
		}
	}
	if(!pLanguage)
	{
		Buffer.append(pText);
		return;
	}
	
	// ��������� ������ ����� ������ ����� � ����
	int ParamTypeStart = -1;
	int BufferStart = Buffer.length();
	int BufferIter = BufferStart;

	// ������� ����������
	va_list VarArgsIter;
	va_copy(VarArgsIter, VarArgs);

	// ������� ��������
	int Iter = 0;
	int Start = Iter;

	// ������ ����� ��� ������ �������
	while(pText[Iter])
	{

		if(ParamTypeStart >= 0)
		{
			if (pText[Iter] != '}')
			{
				Iter = str_utf8_forward(pText, Iter);
				continue;
			}

			// �������� ������ � ��������� ������ ���������
			const void* pVarArgValue = va_arg(VarArgsIter, const void*);
			if(str_comp_num("STR", pText + ParamTypeStart, 3) == 0)
			{
				BufferIter = Buffer.append_at(BufferIter, (const char*) pVarArgValue);
			}
			else if(str_comp_num("INT", pText + ParamTypeStart, 3) == 0)
			{
				int Number = *((const int*) pVarArgValue);
				AppendNumber(Buffer, BufferIter, pLanguage, Number);
			}
			else if(str_comp_num("VAL", pText + ParamTypeStart, 3) == 0)
			{
				int Number = *((const int*) pVarArgValue);
				AppendValue(Buffer, BufferIter, pLanguage, Number);
			}
			else if(str_comp_num("PRC", pText + ParamTypeStart, 3) == 0)
			{
				float Number = (*((const float*) pVarArgValue));
				AppendPercent(Buffer, BufferIter, pLanguage, Number);
			}

			// 
			Start = Iter+1;
			ParamTypeStart = -1;
		}

		// ������ �������� ���������
		else
		{
			if(pText[Iter] == '{')
			{
				BufferIter = Buffer.append_at_num(BufferIter, pText+Start, Iter-Start);
				Iter++;
				ParamTypeStart = Iter;
			}
		}

		Iter = str_utf8_forward(pText, Iter);
	}

	// ��������� ������ ����������
	va_end(VarArgsIter);

	if(Iter > 0 && ParamTypeStart == -1)
		BufferIter = Buffer.append_at_num(BufferIter, pText+Start, Iter-Start);
	
	if(pLanguage && pLanguage->GetWritingDirection() == DIRECTION_RTL)
		ArabicShaping(Buffer, BufferStart);
}

void CLocalization::Format(dynamic_string& Buffer, const char* pLanguageCode, const char* pText, ...)
{
	va_list VarArgs;
	va_start(VarArgs, pText);
	
	Format_V(Buffer, pLanguageCode, pText, VarArgs);
	
	va_end(VarArgs);
}

void CLocalization::Format_VL(dynamic_string& Buffer, const char* pLanguageCode, const char* pText, va_list VarArgs)
{
	const char* pLocalText = Localize(pLanguageCode, pText);
	
	Format_V(Buffer, pLanguageCode, pLocalText, VarArgs);
}

void CLocalization::Format_L(dynamic_string& Buffer, const char* pLanguageCode, const char* pText, ...)
{
	va_list VarArgs;
	va_start(VarArgs, pText);
	
	Format_VL(Buffer, pLanguageCode, pText, VarArgs);
	
	va_end(VarArgs);
}

void CLocalization::Format_VLP(dynamic_string& Buffer, const char* pLanguageCode, int Number, const char* pText, va_list VarArgs)
{
	const char* pLocalText = Localize_P(pLanguageCode, Number, pText);
	
	Format_V(Buffer, pLanguageCode, pLocalText, VarArgs);
}

void CLocalization::Format_LP(dynamic_string& Buffer, const char* pLanguageCode, int Number, const char* pText, ...)
{
	va_list VarArgs;
	va_start(VarArgs, pText);
	
	Format_VLP(Buffer, pLanguageCode, Number, pText, VarArgs);
	
	va_end(VarArgs);
}

void CLocalization::ArabicShaping(dynamic_string& Buffer, int BufferStart)
{
	UErrorCode Status = U_ZERO_ERROR;
	
	int Length = (Buffer.length() - BufferStart + 1);
	int LengthUTF16 = Length*2;
	UChar* pBuf0 = new UChar[LengthUTF16];
	UChar* pBuf1 = new UChar[LengthUTF16];
	
	ucnv_toUChars(m_pUtf8Converter, pBuf0, LengthUTF16, Buffer.buffer() + BufferStart, Length, &Status);
	
	UBiDi* pBiDi = ubidi_openSized(LengthUTF16, 0, &Status);
	ubidi_setPara(pBiDi, pBuf0, -1, UBIDI_DEFAULT_LTR, 0, &Status);
	ubidi_writeReordered(pBiDi, pBuf1, LengthUTF16, UBIDI_DO_MIRRORING, &Status);
	ubidi_close(pBiDi);
	
	u_shapeArabic(
		pBuf1, LengthUTF16,
		pBuf0, LengthUTF16,
			U_SHAPE_LETTERS_SHAPE |
			U_SHAPE_PRESERVE_PRESENTATION |
			U_SHAPE_TASHKEEL_RESIZE |
			U_SHAPE_LENGTH_GROW_SHRINK |
			U_SHAPE_TEXT_DIRECTION_VISUAL_LTR |
			U_SHAPE_LAMALEF_RESIZE,
		&Status
	);
	
	int ShapedLength = u_strlen(pBuf0);
	int NeededSize = UCNV_GET_MAX_BYTES_FOR_STRING(ShapedLength, ucnv_getMaxCharSize(m_pUtf8Converter));
	
	while(Buffer.maxsize() - BufferStart <= NeededSize)
		Buffer.resize_buffer(Buffer.maxsize()*2);
	
	ucnv_fromUChars(m_pUtf8Converter, Buffer.buffer() + BufferStart, Buffer.maxsize() - BufferStart, pBuf0, ShapedLength, &Status);
	
	delete[] pBuf0;
	delete[] pBuf1;
}
