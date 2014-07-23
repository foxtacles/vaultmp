#ifndef __RAK_THREAD_H
#define __RAK_THREAD_H

#if defined(_WIN32_WCE)
#include "WindowsIncludes.h"
#endif





#include "Export.h"






#if defined(WINDOWS_PHONE_8) || defined(WINDOWS_STORE_RT)
#include "../DependentExtensions/WinPhone8/ThreadEmulation.h"
using namespace ThreadEmulation;
#endif

namespace RakNet
{
/// To define a thread, use RAK_THREAD_DECLARATION(functionName);
#if defined(_WIN32_WCE) || defined(WINDOWS_PHONE_8) || defined(WINDOWS_STORE_RT)
#define RAK_THREAD_DECLARATION(functionName) DWORD WINAPI functionName(LPVOID arguments)


#elif defined(_WIN32)
#define RAK_THREAD_DECLARATION(functionName) unsigned __stdcall functionName( void* arguments )


#else
#define RAK_THREAD_DECLARATION(functionName) void* functionName( void* arguments )
#endif

class RAK_DLL_EXPORT RakThread
{
public:




	/// Create a thread, simplified to be cross platform without all the extra junk
	/// To then start that thread, call RakCreateThread(functionName, arguments);
	/// \param[in] start_address Function you want to call
	/// \param[in] arglist Arguments to pass to the function
	/// \return 0=success. >0 = error code

	/*
	nice value 	Win32 Priority
	-20 to -16 	THREAD_PRIORITY_HIGHEST
	-15 to -6 	THREAD_PRIORITY_ABOVE_NORMAL
	-5 to +4 	THREAD_PRIORITY_NORMAL
	+5 to +14 	THREAD_PRIORITY_BELOW_NORMAL
	+15 to +19 	THREAD_PRIORITY_LOWEST
	*/
#if defined(_WIN32_WCE) || defined(WINDOWS_PHONE_8) || defined(WINDOWS_STORE_RT)
	static int Create( LPTHREAD_START_ROUTINE start_address, void *arglist, int priority=0);


#elif defined(_WIN32)
	static int Create( unsigned __stdcall start_address( void* ), void *arglist, int priority=0);



#else
	static int Create( void* start_address( void* ), void *arglist, int priority=0);
#endif




















};

}

#endif
