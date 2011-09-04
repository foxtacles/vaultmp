#ifndef VAULTEXCEPTION_H
#define VAULTEXCEPTION_H

#ifdef __WIN32__
#include <windows.h>
#endif

#include <cstdio>
#include <string>
#include <typeinfo>

#include "vaultmp.h"
#include "Debug.h"

using namespace std;

class VaultException : public exception
{
private:
    string error;

#ifdef VAULTMP_DEBUG
    static Debug* debug;
#endif

    VaultException& operator=(const VaultException&);

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
