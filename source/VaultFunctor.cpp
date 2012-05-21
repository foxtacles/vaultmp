#include "VaultFunctor.h"

VaultFunctor::VaultFunctor() : next(NULL), func(NULL)
{

}

VaultFunctor::VaultFunctor(unsigned int flags) : next(NULL), func(NULL), flags(flags)
{

}

VaultFunctor::VaultFunctor(vector<string>(*func)()) : next(NULL), func(func)
{

}

VaultFunctor::~VaultFunctor()
{
	if (this->next)
		delete next;
}

void VaultFunctor::_next(vector<string>& result)
{
	if (this->next)
	{
		vector<string> __next = (*this->next)();
		result.insert(result.end(), __next.begin(), __next.end());
		sort(result.begin(), result.end());
		result.erase(unique(result.begin(), result.end()), result.end());
	}
}

VaultFunctor* VaultFunctor::connect(VaultFunctor* next)
{
	if (!this->next)
		return (this->next = next);

	else
		return NULL;
}

inline vector<string> VaultFunctor::operator()()
{
	vector<string> result;

	if (func)
		result = func();

	_next(result);
	return result;
}
