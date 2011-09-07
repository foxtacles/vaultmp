#include "VaultFunctor.h"

VaultFunctor::VaultFunctor()
{
    func = NULL;
}

VaultFunctor::VaultFunctor(unsigned int flags) : flags(flags)
{
    func = NULL;
}

VaultFunctor::VaultFunctor(vector<string>(*func)()) : func(func)
{

}

VaultFunctor::~VaultFunctor()
{

}
