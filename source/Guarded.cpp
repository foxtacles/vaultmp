#include "Guarded.h"

#ifdef VAULTMP_DEBUG
template <typename T>
Debug* Guarded<T>::debug;
#endif

#ifdef VAULTMP_DEBUG
template <typename T>
void Guarded<T>::SetDebugHandler(Debug* debug)
{
	Guarded::debug = debug;

	if (debug)
		debug->PrintFormat("Attached debug handler to Guarded<%s> class", true, typeid(T).name());
}
#endif
