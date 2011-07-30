#ifndef CRITICALSECTION_H
#define CRITICALSECTION_H

#include <windows.h>

class CriticalSection {

private:
    CRITICAL_SECTION cs;

protected:
    CriticalSection();
    virtual ~CriticalSection();

public:
    void StartSession();
    void EndSession();

};

#endif
