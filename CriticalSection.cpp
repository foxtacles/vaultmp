#include "CriticalSection.h"

CriticalSection::CriticalSection()
{
    finalize = false;
    locks = 0;
    #ifdef __WIN32__
    InitializeCriticalSection(&cs);
    #else
    pthread_mutex_init(&cs, PTHREAD_MUTEX_RECURSIVE);
    #endif
}

CriticalSection::~CriticalSection()
{
    if (locks != 0)
        throw VaultException("Lock count of CriticalSection is not zero, but some thread invoked a delete (%s)", typeid(*this).name());
    #ifdef __WIN32__
    DeleteCriticalSection(&cs);
    #else
    pthread_mutex_destroy(&cs);
    #endif
}

CriticalSection* CriticalSection::StartSession()
{
    int success = 0;
    #ifdef __WIN32__
    for (int i = 0; i < CS_TIMEOUT && !finalize && (success = TryEnterCriticalSection(&cs)) == 0; i++)
        Sleep(1);
    #else
    for (int i = 0; i < CS_TIMEOUT && !finalize && (success = pthread_mutex_trylock(&cs)) != 0; i++)
        Sleep(1);
    #endif
    if (success && !finalize)
    {
        locks++;
        return this;
    }
    else if (finalize)
        return NULL;
    else
        throw VaultException("Could not enter CriticalSection, timeout of %dms reached (%s)", CS_TIMEOUT, typeid(*this).name());
}

void CriticalSection::EndSession()
{
    if (locks < 1)
        throw VaultException("Lock count of CriticalSection is zero, but some thread tried to leave (%s)", typeid(*this).name());
    locks--;
    #ifdef __WIN32__
    LeaveCriticalSection(&cs);
    #else
    pthread_mutex_unlock(&cs);
    #endif
}

void CriticalSection::Finalize() // must be called by the thread which wants to delete this object
{
    finalize = true;
    EndSession();
}

