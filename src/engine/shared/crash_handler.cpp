#include "crash_handler.h"
#include <base/system.h>

#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <stdarg.h>

// windows platform
#if defined(CONF_FAMILY_WINDOWS)
	#include <intrin.h>
	#include <windows.h>

	#include <dbghelp.h>
	#pragma comment(lib, "dbghelp.lib")

// unix platform
#elif defined(CONF_FAMILY_UNIX)
	#include <execinfo.h>
	#include <cxxabi.h>
#endif

static IOHANDLE s_ioStackTraceFile = nullptr;
static void WriteLineFile(const char* pLine, ...)
{
	if(!s_ioStackTraceFile)
	{
		if(!fs_is_dir("crashes"))
			fs_makedir("crashes");

		char aFile[128];
		char aTimeBuf[80];
		str_timestamp_format(aTimeBuf, sizeof(aTimeBuf), FORMAT_NOSPACE);
		str_format(aFile, sizeof(aFile), "crashes/Crash-%s.txt", aTimeBuf);

		s_ioStackTraceFile = io_open(aFile, IOFLAG_WRITE);
		if(!s_ioStackTraceFile)
			dbg_msg("stacktrace", "failed to to write in %s", aFile);
	}

	va_list args;
	va_start(args, pLine);
	char aBuf[1024];
#if defined(CONF_FAMILY_WINDOWS) && !defined(__GNUC__)
	_vsprintf_p(aBuf, sizeof(aBuf), pLine, args);
#else
	vsnprintf(aBuf, sizeof(aBuf), pLine, args);
#endif
	aBuf[sizeof(aBuf) - 1] = '\0';
	va_end(args);

	io_write(s_ioStackTraceFile, aBuf, (int)str_length(aBuf));
	io_write_newline(s_ioStackTraceFile);
	io_flush(s_ioStackTraceFile);
}

CrashHandler::CrashHandler()
{
	// register the signals that will call the SignalHandler function
	signal(SIGABRT, SignalHandler);
	signal(SIGSEGV, SignalHandler);
	signal(SIGFPE, SignalHandler);
	signal(SIGILL, SignalHandler);
}

void CrashHandler::SignalHandler(int Signal)
{
	const char* pName = "Unsupport signal for now.";
	if(Signal == SIGABRT) pName = "SIGABRT";
	else if(Signal == SIGSEGV) pName = "SIGSEGV";
	else if(Signal == SIGFPE) pName = "SIGFPE";
	else if(Signal == SIGILL) pName = "SIGILL";

	char aTimeBuf[80];
	str_timestamp_format(aTimeBuf, sizeof(aTimeBuf), FORMAT_SPACE);
	WriteLineFile("==========================================================================================");
	WriteLineFile("Crash time: %s\t\t\t\t\tSignal caught: %s(%d)", aTimeBuf, pName, Signal);
	WriteLineFile("==========================================================================================");
	WriteLineFile("");
	WriteStackTrace();

	exit(Signal);
}

void CrashHandler::WriteStackTrace()
{
	int TraceLines = 0;
	WriteLineFile("Stack trace:");

#if defined(CONF_FAMILY_WINDOWS)
	const DWORD Machine = IMAGE_FILE_MACHINE_AMD64; // TODO: idk
	const HANDLE Process = GetCurrentProcess();
	const HANDLE Thread = GetCurrentThread();
	SymInitialize(Process, nullptr, TRUE);
	SymSetOptions(SYMOPT_LOAD_LINES);

	CONTEXT context;
	context.ContextFlags = CONTEXT_FULL;
	RtlCaptureContext(&context);

	STACKFRAME FrameShot;
	FrameShot.AddrPC.Mode = AddrModeFlat;
	FrameShot.AddrFrame.Mode = AddrModeFlat;
	FrameShot.AddrStack.Mode = AddrModeFlat;
#if defined(CONF_PLATFORM_WIN32) // https://docs.microsoft.com/en-us/windows/win32/api/dbghelp/ns-dbghelp-stackframe
	FrameShot.AddrPC.Offset = context.Eip;
	FrameShot.AddrFrame.Offset = context.Ebp;
	FrameShot.AddrStack.Offset = context.Esp;
#else
	FrameShot.AddrPC.Offset = context.Rip;
	FrameShot.AddrFrame.Offset = context.Rbp;
	FrameShot.AddrStack.Offset = context.Rsp;
#endif

	while(StackWalk(Machine, Process, Thread, &FrameShot, &context, nullptr, SymFunctionTableAccess, SymGetModuleBase, nullptr))
	{
		DWORD64 Offset = 0;
		char aSymbolBuffer[sizeof(IMAGEHLP_SYMBOL) + 255];
		PIMAGEHLP_SYMBOL Symbol = (PIMAGEHLP_SYMBOL)aSymbolBuffer;
		Symbol->SizeOfStruct = (sizeof IMAGEHLP_SYMBOL) + 255;
		Symbol->MaxNameLength = 254;

		const char* pFuncName = "Unknown Function";
		if(SymGetSymFromAddr(Process, FrameShot.AddrPC.Offset, &Offset, Symbol))
			pFuncName = Symbol->Name;

		int LineNum = 0;
		DWORD OffsetLine = 0;
		IMAGEHLP_LINE Line;
		Line.SizeOfStruct = sizeof(IMAGEHLP_LINE);
		const char* pFileName = "Unknown File";
		if(SymGetLineFromAddr(Process, FrameShot.AddrPC.Offset, &OffsetLine, &Line))
		{
			pFileName = Line.FileName;
			LineNum = static_cast<int>(Line.LineNumber);
		}
		TraceLines++;

		char aBuf[2048];
		str_format(aBuf, sizeof(aBuf), "%d. %s(%d) %s", TraceLines, pFileName, LineNum, pFuncName);
		WriteLineFile(aBuf);
	}
	SymCleanup(Process);

#elif defined(CONF_FAMILY_UNIX)
	void* paAddrList[MAX_FRAMES + 1];
	unsigned int Addrlen = backtrace(paAddrList, sizeof(paAddrList) / sizeof(void*));

	if(Addrlen == 0)
	{
		WriteLineFile("No stack addresses available.");
		return;
	}

	// returned symbol lines. skip the first, it is the address of this function.
	char** ppSymbollist = backtrace_symbols(paAddrList, Addrlen);
	size_t FuncNameSize = 1024;
	char aFuncName[1024];

	for(unsigned int i = 0; i < Addrlen; i++)
	{
		char* pBeginName = NULL;
		char* pBeginOffset = NULL;
		char* pEndOffset = NULL;

		// find parentheses and +address offset surrounding the mangled name
		for(char* p = ppSymbollist[i]; *p; ++p)
		{
			if(*p == '(')
				pBeginName = p;
			else if(*p == '+')
				pBeginOffset = p;
			else if(*p == ')' && (pBeginOffset || pBeginName))
				pEndOffset = p;
		}

		if(pBeginName && pEndOffset && (pBeginName < pEndOffset))
		{
			*pBeginName++ = '\0';
			*pEndOffset++ = '\0';
			if(pBeginOffset)
				*pBeginOffset++ = '\0';

			int Status = 0;
			char* pRet = abi::__cxa_demangle(pBeginName, aFuncName, &FuncNameSize, &Status);
			char* pfName = Status == 0 ? pRet : pBeginName;

			TraceLines++;
			WriteLineFile("%d. %s %s %s", TraceLines, pEndOffset, ppSymbollist[i], pfName);
		}
	}
	free(ppSymbollist);
#endif

	io_close(s_ioStackTraceFile);
}