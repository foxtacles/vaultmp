#include "VaultFunctor.h"

VaultFunctor::~VaultFunctor()
{
	delete this->next;
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
		return nullptr;
}
