#ifndef VAULTFUNCTOR_H
#define VAULTFUNCTOR_H

#include <string>
#include <vector>
#include "vaultmp.h"

using namespace std;

class VaultFunctor
{
protected:
    unsigned int flags;
    vector<string>(*func)();

    VaultFunctor(unsigned int flags);

public:
    VaultFunctor();
    VaultFunctor(vector<string>(*func)());
    virtual ~VaultFunctor();

    virtual inline vector<string> operator()() { if (func) return func(); else return vector<string>(); };

};

#endif
