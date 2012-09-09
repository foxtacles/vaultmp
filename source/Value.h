#ifndef VALUE_H
#define VALUE_H

#include "vaultmp.h"
#include "Lockable.h"

#ifdef VAULTMP_DEBUG
#include "Debug.h"
#endif

#include <set>

using namespace std;

class Container;

/**
 * \brief A container class which simply stores a variable of type T
 *
 * Derives from Lockable to lock / unlock the data member
 */

template <typename T>
class Value : public Lockable
{
	private:
		T value;

#ifdef VAULTMP_DEBUG
		static Debug* debug;
#endif

		Value& operator=(const Value&);

	public:
		Value() : value(T()) {};
		Value(const T& t) : value(t) {};
		Value(Value&&) = default;
		Value& operator=(Value&&) = default;
		virtual ~Value() {}

		/**
		 * \brief Sets the value
		 */
		bool set(const T& value);
		/**
		 * \brief Gets the value
		 */
		T get() const { return value; };


#ifdef VAULTMP_DEBUG
		static void SetDebugHandler(Debug* debug);
#endif

};

#endif
