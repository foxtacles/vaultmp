#include "Shared.h"

#ifdef VAULTMP_DEBUG
template <typename T>
Debug* Shared<T>::debug;
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
	try
	{
		this->async.set_value(this->get());
	}
	catch (exception& e)
	{
		throw VaultException("Failed setting promise (%08X -> %08X: %s)", this, &this->async, e.what());
	}

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
template class Shared<unsigned short>;
template class Shared<signed char>;
template class Shared<bool>;
template class Shared<pair<set<unsigned int>, set<unsigned int>>>;
