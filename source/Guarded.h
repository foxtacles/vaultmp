#ifndef GUARDED_H
#define GUARDED_H

#include "vaultmp.h"
#include "Value.h"

#ifdef VAULTMP_DEBUG
#include "Debug.h"
#endif

/**
 * \brief A class for guarding a Value with a CriticalSection
 *
 * Derives from Value and CriticalSection
 */

template <typename T>
class Guarded : public Value<T>, public CriticalSection
{
	private:
#ifdef VAULTMP_DEBUG
		static Debug* debug;
#endif

	public:
#ifdef VAULTMP_DEBUG
		static void SetDebugHandler(Debug* debug);
#endif

};

#endif
