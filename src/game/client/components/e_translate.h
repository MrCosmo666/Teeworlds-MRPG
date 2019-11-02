#ifndef E_TRANSLATE_H
#define E_TRANSLATE_H

struct TranslateTextThreadData;
typedef void TranslatorCallback(TranslateTextThreadData * Data);

typedef struct TranslateTextThreadData
{
	const char * Text;
	TranslatorCallback * Callback;
	void * Param;
	const char * Translated;
} TranslateTextThreadData;

unsigned long TranslateText(const char * Text, TranslatorCallback * Callback, void * Param);

#endif 