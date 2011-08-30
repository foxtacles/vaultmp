#ifndef VALUE_H
#define VALUE_H

#include "vaultmp.h"
#include "Lockable.h"

#ifdef VAULTMP_DEBUG
#include "Debug.h"
#endif

using namespace std;

class Object;
class Container;

template <typename T>
class Value : public Lockable
{

private:

    unsigned char id;
    T value;

#ifdef VAULTMP_DEBUG
    static Debug* debug;
#endif

public:

    Value();
    Value(unsigned char id);
    virtual ~Value();

    bool Set(T value);
    T Get();

    void SetID(unsigned char id);
    unsigned char GetID();

#ifdef VAULTMP_DEBUG
    static void SetDebugHandler(Debug* debug);
#endif

};

#endif
