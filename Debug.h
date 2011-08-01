#ifndef DEBUG_H
#define DEBUG_H

#ifdef __WIN32__
#include <windows.h>
#endif
#include <time.h>
#include <stdio.h>
#include <string.h>

#include <string>

using namespace std;

class Debug
{

private:
    string logfile;
    FILE* vaultmplog;
    bool filemutex;

    static void GetTimeFormat(char* buf, int size, bool file);

public:
    Debug(char* module);
    ~Debug();

    void Print(char* text, bool timestamp);
    void PrintSystem();

};

#endif
