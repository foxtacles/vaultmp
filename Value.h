#ifndef VALUE_H
#define VALUE_H

#include "vaultmp.h"
#include "Lockable.h"

#ifdef VAULTMP_DEBUG
#include "Debug.h"
#endif

using namespace std;

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

    Value& operator=(const Value&);

public:

    Value();
    virtual ~Value();

    bool Set(T value);
    T Get() const;

#ifdef VAULTMP_DEBUG
    static void SetDebugHandler(Debug* debug);
#endif

};

#endif
