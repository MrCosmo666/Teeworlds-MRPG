#include "e_translate.h"

#include <base/system.h>
#include <engine/shared/config.h>
#include <engine/external/json-parser/json.h>

#include <curl/curl.h>

const unsigned long TranslationBufferSize = 4096;

size_t curlWriteFunc(char *data, size_t size, size_t nmemb, char * buffer)  
{  
    size_t result = 0;  
    if (buffer != NULL)  
    {
		result = size * nmemb;
		memcpy(buffer + strlen(buffer), data, result <= TranslationBufferSize - strlen(buffer) - 1 ? result : TranslationBufferSize - strlen(buffer) - 1);
    }
    return result;  
}

char *UnescapeStep1(char * From)
{
	unsigned long Len = str_length(From);
	char * Result = (char *)mem_alloc(Len * 4 + 1, 1);
	memset(Result, 0, Len * 4 + 1);

	unsigned long i = 0;
	unsigned long j = 0;
	while (i < Len)
	{
		if (From[i] == '\\')
		{
			if (From[i + 1] == '\\')
			{
				Result[j++] = '\\';
				i += 2;
			} else {
				if (From[i + 2] == 'u')
				{
					unsigned long Code = 0;
					int q;
					for (q = 0; q < 4; q++)
					{
						if (From[i + 3 + q] >= '0' && From[i + 3 + q] <= '9')
							q = (q * 16) + From[i + 3 + q] - '0';
						else if (From[i + 3 + q] >= 'a' && From[i + 3 + q] <= 'f')
							q = (q * 16) + 10 + From[i + 3 + q] - 'a';
						else if (From[i + 3 + q] >= 'A' && From[i + 3 + q] <= 'F')
							q = (q * 16) + 10 + From[i + 3 + q] - 'A';
						else break;
					}
					j += str_utf8_encode(Result, Code);
					i += 2 + q;
				}
			}
		} else {
			Result[j++] = From[i++];
		}
	}

	return Result;
}

char *UnescapeStep2(char * From)
{
	unsigned long Len = str_length(From);
	char * Result = (char *)mem_alloc(Len * 4 + 1, 1);
	memset(Result, 0, Len * 4 + 1);

	unsigned long i = 0;
	unsigned long j = 0;
	while (i < Len)
	{
		if (From[i] == '%')
		{
			if (From[i + 1] != '&')
			{
				Result[j++] = From[i++];
			} 
			else 
			{
				unsigned long Code = 0;
				int q;
				for (q = 0; q < 4; q++)
				{
					if (From[i + 3 + q] >= '0' && From[i + 3 + q] <= '9')
						q = (q * 16) + From[i + 3 + q] - '0';
					else if (From[i + 3 + q] >= 'a' && From[i + 3 + q] <= 'f')
						q = (q * 16) + 10 + From[i + 3 + q] - 'a';
					else if (From[i + 3 + q] >= 'A' && From[i + 3 + q] <= 'F')
						q = (q * 16) + 10 + From[i + 3 + q] - 'A';
					else break;
				}
				j += str_utf8_encode(Result, Code);
				i += 2 + q;
				if (From[i] == ';') i++;
			}
		} 
		else 
		{
			Result[j++] = From[i++];
		}
	}
	return Result;
}

char *UnescapeStr(char * From)
{
	char * First = UnescapeStep1(From);
	char * Second = UnescapeStep2(First);
	delete First;
	First = NULL;

	return Second;
}

void TranslateTextThreadFunc(void * Param)
{		
	CURL * curl = NULL;
	char * Result = NULL;
	TranslateTextThreadData * Data = (TranslateTextThreadData *)Param;

	try
	{		

		(*(Data->Callback))(Data);

		delete (void *)Data;
		Data = NULL;
	}
	catch(...)
	{
		if (Data)
		{
			delete (void *)Data;
			Data = NULL;
		}
		if (Result)
		{
			delete Result;
			Result = NULL;
		}
	}
}

unsigned long TranslateText(const char * Text, TranslatorCallback * Callback, void * Param)
{
	TranslateTextThreadData * Data = (TranslateTextThreadData *)mem_alloc(sizeof(TranslateTextThreadData), 1);
	Data->Text = strdup(Text);
	Data->Callback = Callback;
	Data->Param = Param;

	return (unsigned long)thread_init(TranslateTextThreadFunc, (void *)Data, "Translate");
}