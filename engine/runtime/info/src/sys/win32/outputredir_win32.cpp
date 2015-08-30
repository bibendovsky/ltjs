
#ifndef __LIBLTINFO_H__
#include "libltinfo.h"
#endif


#ifdef LIBLTINFO_OUTPUT_REDIRECTION


#ifndef __OUTPUTREDIR_H__
#include "outputredir.h"
#endif


#ifdef _WIN32
#include <windows.h>
#include <crtdbg.h>
#endif

#include <ltassert.h>
#include <dsys.h>


//	-------------------------------------------------------------------------
bool COutputRedir::OpenLogFile(ECOutput_ReDir logfile, const char* pFilename)
{
	// open a file and associate it with one of the logfile enums
	FILE* f = fopen(pFilename, "w");

	m_pLogFiles[logfile - OUTPUT_REDIR_FILE0] = (uint32)f;
	
	return (f != NULL);
}


//	-------------------------------------------------------------------------
bool COutputRedir::CloseLogFile(ECOutput_ReDir logfile)
{
	// close a file associated with one of the logfile enums
	int32 result;
	FILE* f = (FILE*)m_pLogFiles[logfile - OUTPUT_REDIR_FILE0];
	
	result = fclose(f);

	m_pLogFiles[logfile - OUTPUT_REDIR_FILE0] = 0;
	
	return (result == 0);
}


//	-------------------------------------------------------------------------
void COutputRedir::OutputToFile(uint32 index)
{
	// outputs to the specified logfile
	FILE* f = (FILE*)m_pLogFiles[index];

	if (f) {
		fputs(m_pPrintBuffer, f);
	}
}


//	-------------------------------------------------------------------------
void COutputRedir::OutputToCONIO()
{
	// outputs to the system console (printf)
	printf(m_pPrintBuffer);
}


//	-------------------------------------------------------------------------
void COutputRedir::OutputToDLL()
{
	// NOT YET IMPLEMENTED
	ASSERT(!"Output redirection to a logging DLL is not yet implemented.  Sorry."); 
}


//	-------------------------------------------------------------------------
void COutputRedir::OutputToASSERT()
{
	// putputs to the new & improved LT_ASSERTX
	//assert
	//(void)( (0) || (_assert(m_pPrintBuffer, __FILE__, __LINE__), 0) );

#ifndef __MINGW32__
	//_assert(m_pPrintBuffer, __FILE__, __LINE__);
    _CrtDbgReport(_CRT_ASSERT, NULL, 0, "server.dll", "%s", m_pPrintBuffer);
#else
	ASSERT(!m_pPrintBuffer);
#endif

}


//	-------------------------------------------------------------------------
void COutputRedir::OutputToConsole()
{
	// prints to the LithTech in-game console
	dsi_PrintToConsole(m_pPrintBuffer);
}


//	-------------------------------------------------------------------------
void COutputRedir::OutputToIDE()
{
	// outputs to MSDEV

#ifdef _WIN32
	OutputDebugString(m_pPrintBuffer);
#endif

#ifdef __LINUX
	printf( m_pPrintBuffer);
#endif

}


#endif // LIBLTINFO_OUTPUT_REDIRECTION

