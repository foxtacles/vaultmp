#ifndef VAULTEXCEPTION_H
#define VAULTEXCEPTION_H

#ifdef __WIN32__
#include <windows.h>
#endif

#include "vaultmp.h"
#include <stdio.h>
#include <string>
#include <typeinfo>

#ifdef VAULTMP_DEBUG
    #ifndef DONT_NEED_DEBUG
    #include "Debug.h"
    #else
    class Debug;
    #undef DONT_NEED_DEBUG
    #endif
#endif

using namespace std;

class VaultException : public exception
{
private:
    string error;

#ifdef VAULTMP_DEBUG
    static Debug* debug;
#endif

public:

    VaultException(string error);
    VaultException(const char* format, ...);
    virtual ~VaultException() throw() {}

    void Message();
    void Console();

#ifdef VAULTMP_DEBUG
    static void SetDebugHandler(Debug* debug);
    static void FinalizeDebug();
#endif

};

#endif
