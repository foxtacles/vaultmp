#ifndef __SIGNALED_EVENT_H
#define __SIGNALED_EVENT_H

#if defined(_XBOX) || defined(X360)
                            
#elif defined(_WIN32)
#include <windows.h>
#else
	#include <pthread.h>
	#include <sys/types.h>
	#include "SimpleMutex.h"
	#if defined(_PS3) || defined(__PS3__) || defined(SN_TARGET_PS3)
                                                   
	#endif
#endif

#include "Export.h"

namespace RakNet
{

class RAK_DLL_EXPORT SignaledEvent
{
public:
	SignaledEvent();
	~SignaledEvent();

	void InitEvent(void);
	void CloseEvent(void);
	void SetEvent(void);
	void WaitOnEvent(int timeoutMs);

protected:
#ifdef _WIN32
	HANDLE eventList;
#else
	SimpleMutex isSignaledMutex;
	bool isSignaled;
	pthread_condattr_t condAttr;
	pthread_cond_t eventList;
	pthread_mutex_t hMutex;
	pthread_mutexattr_t mutexAttr;
#endif
};

} // namespace RakNet

#endif
