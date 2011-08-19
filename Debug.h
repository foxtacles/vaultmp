#ifndef DEBUG_H
#define DEBUG_H

#ifdef __WIN32__
#include <windows.h>
#endif
#include <time.h>
#include <stdio.h>
#include <string.h>

#include <string>

#include "CriticalSection.h"

using namespace std;

class Debug : private CriticalSection
{

private:
    string logfile;
    FILE* vaultmplog;

    static void GetTimeFormat(char* buf, int size, bool file);

public:
    Debug(char* module);
    ~Debug();

    void Print(const char* text, bool timestamp);
    void PrintFormat(const char* format, bool timestamp, ...);
    void PrintSystem();

};

#endif
