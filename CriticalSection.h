#ifndef CRITICALSECTION_H
#define CRITICALSECTION_H

#ifdef __WIN32__
#include <windows.h>
#else
#include <pthread.h>
#endif

#include <typeinfo>
#define CS_TIMEOUT     5000

class Debug;

class CriticalSection
{

private:
#ifdef __WIN32__
    CRITICAL_SECTION cs;
#else
    pthread_mutex_t cs;
    pthread_mutexattr_t mta;
#endif

#ifdef VAULTMP_DEBUG
    static Debug* debug;
    bool ndebug;
#endif

    bool finalize;
    int locks;

    CriticalSection(const CriticalSection&);
    CriticalSection& operator=(const CriticalSection&);

public:
    CriticalSection();
    virtual ~CriticalSection();

    CriticalSection* StartSession();
    void EndSession();
    void Finalize();

#ifdef VAULTMP_DEBUG
    void ToggleSectionDebug(bool toggle);
    static void SetDebugHandler(Debug* debug);
#endif

};

#endif
