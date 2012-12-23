#ifndef GUARDED_H
#define GUARDED_H

#include "vaultmp.h"
#include "Value.h"

/**
 * \brief A class for guarding a Value with a CriticalSection
 *
 * Derives from Value and CriticalSection
 */

template <typename T>
class Guarded : public Value<T>, public CriticalSection
{

};

#endif
