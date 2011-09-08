#ifndef VAULTFUNCTOR_H
#define VAULTFUNCTOR_H

#include <string>
#include <vector>
#include <algorithm>
#include "vaultmp.h"

using namespace std;

class VaultFunctor
{
private:
    VaultFunctor* next;
    vector<string>(*func)();

protected:
    unsigned int flags;
    void _next(vector<string>& result);
    VaultFunctor(unsigned int flags);

public:
    VaultFunctor();
    VaultFunctor(vector<string>(*func)());
    virtual ~VaultFunctor();

    VaultFunctor* connect(VaultFunctor* next);
    virtual vector<string> operator()();

};

#endif
