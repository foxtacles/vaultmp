#include "CriticalSection.h"

CriticalSection::CriticalSection()
{
    #ifdef __WIN32__
    InitializeCriticalSection(&cs);
    #else
    cs = PTHREAD_MUTEX_INITIALIZER;
    #endif
}

CriticalSection::~CriticalSection()
{
    #ifdef __WIN32__
    DeleteCriticalSection(&cs);
    #endif
}

void CriticalSection::StartSession()
{
    #ifdef __WIN32__
    EnterCriticalSection(&cs);
    #else
    pthread_mutex_lock(&cs);
    #endif
}

void CriticalSection::EndSession()
{
    #ifdef __WIN32__
    LeaveCriticalSection(&cs);
    #else
    pthread_mutex_unlock(&cs);
    #endif
}
