#include "Shared.h"

#ifdef VAULTMP_DEBUG
template <typename T>
Debug* Shared<T>::debug = nullptr;
#endif

#ifdef VAULTMP_DEBUG
template <typename T>
void Shared<T>::SetDebugHandler(Debug* debug)
{
	Shared::debug = debug;

	if (debug)
		debug->Print("Attached debug handler to Shared class", true);
}
#endif

template <typename T>
bool Shared<T>::set_promise()
{
	this->async.set_value(this->get());

	return true;
}

template <typename T>
T Shared<T>::get_future(chrono::milliseconds timeout)
{
	future<T> f = this->async.get_future();

	if (timeout > chrono::milliseconds(0))
		if (f.wait_for(timeout) == future_status::timeout)
			throw VaultException("Timeout of %d reached for future value retrieval", timeout.count());

	T value = f.get();
	this->async = promise<T>();

	return value;
}

template class Shared<unsigned int>;
template class Shared<bool>;
