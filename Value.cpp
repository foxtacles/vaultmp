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
T Value<T>::Get() const
{
    return value;
}

template class Value<unsigned int>;
template class Value<unsigned char>;
template class Value<bool>;
template class Value<double>;
template class Value<string>;
template class Value<Container*>;
