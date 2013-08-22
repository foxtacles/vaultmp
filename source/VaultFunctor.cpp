#include "VaultFunctor.hpp"

#include <algorithm>

using namespace std;

VaultFunctor::~VaultFunctor()
{
	delete this->next;
}

void VaultFunctor::_next(vector<string>& result)
{
	if (this->next)
	{
		vector<string> _next_ = (*this->next)();
		result.insert(result.end(), _next_.begin(), _next_.end());
		sort(result.begin(), result.end());
		result.erase(unique(result.begin(), result.end()), result.end());
	}
}

VaultFunctor* VaultFunctor::connect(VaultFunctor* next)
{
	if (!this->next)
		return (this->next = next);
	else
		return this->next->connect(next);
}
