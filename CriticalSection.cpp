#include "CriticalSection.h"

CriticalSection::CriticalSection()
{
    InitializeCriticalSection(&cs);
}

CriticalSection::~CriticalSection()
{
    DeleteCriticalSection(&cs);
}

void CriticalSection::StartSession()
{
    EnterCriticalSection(&cs);
}

void CriticalSection::EndSession()
{
    LeaveCriticalSection(&cs);
}
