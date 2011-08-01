#ifndef CRITICALSECTION_H
#define CRITICALSECTION_H

#ifdef __WIN32__
#include <windows.h>
#else
#include <pthread.h>
#endif

class CriticalSection
{

private:
#ifdef __WIN32__
    CRITICAL_SECTION cs;
#else
    pthread_mutex_t cs;
#endif

protected:
    CriticalSection();
    virtual ~CriticalSection();

public:
    void StartSession();
    void EndSession();

};

#endif
