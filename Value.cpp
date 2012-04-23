#include "Value.h"

#ifdef VAULTMP_DEBUG
template <typename T>
Debug* Value<T>::debug = NULL;
#endif

template <typename T>
Value<T>::Value()
{
	value = 0;
}

template <typename T>
Value<T>::Value(T t) : value(t)
{

}

template <>
Value<string>::Value()
{
	value = "";
}

template <typename T>
Value<T>::~Value()
{

}

#ifdef VAULTMP_DEBUG
template <typename T>
void Value<T>::SetDebugHandler(Debug* debug)
{
	Value::debug = debug;

	if (debug != NULL)
		debug->Print("Attached debug handler to Value class", true);
}
#endif

template <typename T>
bool Value<T>::set(T value)
{
	if (this->IsLocked())
		return false;

	this->value = value;

	return true;
}

template <typename T>
T Value<T>::get() const
{
	return value;
}

template <typename T>
bool Value<T>::set_promise()
{
	this->async.set_value(this->get());

	return true;
}

template <typename T>
T Value<T>::get_future(chrono::milliseconds timeout)
{
	future<T> f = this->async.get_future();

	if (timeout > chrono::milliseconds(0))
		if (f.wait_for(timeout) == future_status::timeout)
			throw VaultException("Timeout of %d reached for future value retrieval", timeout.count());

	T value = f.get();
	this->async = promise<T>();

	return value;
}

template class Value<unsigned int>;
template class Value<unsigned char>;
template class Value<bool>;
template class Value<double>;
template class Value<string>;
template class Value<Container*>;
