#ifndef CRITICALSECTION_H
#define CRITICALSECTION_H

#ifdef __WIN32__
#include <windows.h>
#else
#include <pthread.h>
#endif

#include <typeinfo>

#define DONT_NEED_DEBUG
#include "VaultException.h"

#define CS_TIMEOUT     5000

class CriticalSection
{

private:
#ifdef __WIN32__
    CRITICAL_SECTION cs;
#else
    pthread_mutex_t cs;
#endif

    bool finalize;
    int locks;

public:
    CriticalSection();
    virtual ~CriticalSection();

    CriticalSection* StartSession();
    void EndSession();
    void Finalize();

};

#endif
