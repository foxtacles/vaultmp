#ifndef LOCKABLE_H
#define LOCKABLE_H

#include <map>
#include <list>
#include <string>
#include <algorithm>
#include <limits.h>

#include "vaultmp.h"
#include "CriticalSection.h"

#ifdef VAULTMP_DEBUG
#include "Debug.h"
#endif

#define ISFLAT(key) (key > 0x00)
#define ISDEEP(key) (key < 0x00)

using namespace std;

class Lockable
{

private:
    static signed int flat_key;
    static signed int deep_key;
    static map<signed int, Lockable*> keymap;
    static CriticalSection cs;

    static signed int NextKey(bool flat);

    list<signed int> locks;

#ifdef VAULTMP_DEBUG
    static Debug* debug;
#endif

protected:

    Lockable();

public:

    virtual ~Lockable();

    static Lockable* BlindUnlock(signed int key);

    signed int Lock(bool flat);
    Lockable* Unlock(signed int key);
    bool IsLocked();

#ifdef VAULTMP_DEBUG
    static void SetDebugHandler(Debug* debug);
#endif

};

#endif
