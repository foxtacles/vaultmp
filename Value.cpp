#include "Value.h"

#ifdef VAULTMP_DEBUG
template <typename T>
Debug* Value<T>::debug = NULL;
#endif

template <typename T>
Value<T>::Value(unsigned char id)
{
    this->id = id;
    value = 0;
}

template <>
Value<string>::Value(unsigned char id)
{
    this->id = id;
    value = "";
}

template <typename T>
Value<T>::Value()
{
    value = 0;
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
bool Value<T>::Set(T value)
{
    if (this->IsLocked())
        return false;

    this->value = value;

    return true;
}

template <typename T>
T Value<T>::Get()
{
    return value;
}

template <typename T>
void Value<T>::SetID(unsigned char id)
{
    this->id = id;
}

template <typename T>
unsigned char Value<T>::GetID()
{
    return id;
}

template class Value<unsigned int>;
template class Value<bool>;
template class Value<double>;
template class Value<string>;
