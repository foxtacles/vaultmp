#include "Value.h"

#ifdef VAULTMP_DEBUG
template <typename T>
Debug* Value<T>::debug;
#endif

#ifdef VAULTMP_DEBUG
template <typename T>
void Value<T>::SetDebugHandler(Debug* debug)
{
	Value::debug = debug;

	if (debug)
		debug->PrintFormat("Attached debug handler to Value<%s> class", true, typeid(T).name());
}
#endif

template <typename T>
bool Value<T>::set(const T& value)
{
	if (this->IsLocked())
		return false;

	this->value = value;

	return true;
}

template class Value<unsigned int>;
template class Value<signed int>;
template class Value<unsigned long long>;
template class Value<unsigned char>;
template class Value<unsigned short>;
template class Value<signed char>;
template class Value<bool>;
template class Value<double>;
template class Value<string>;
template class Value<Container*>;
template class Value<pair<set<unsigned int>, set<unsigned int>>>;
