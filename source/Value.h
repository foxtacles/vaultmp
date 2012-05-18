#ifndef VALUE_H
#define VALUE_H

#include "vaultmp.h"
#include "Lockable.h"

#include <chrono>
#include <future>

#ifdef VAULTMP_DEBUG
#include "Debug.h"
#endif

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
		promise<T> async;

#ifdef VAULTMP_DEBUG
		static Debug* debug;
#endif

		Value& operator=(const Value&);

	public:

		Value() : value(T()) {};
		Value(T t) : value(t) {};
		Value(Value &&) = default;
		Value& operator=(Value &&) = default;
		virtual ~Value() {}

		/**
		 * \brief Sets the value
		 */
		bool set(T value);
		/**
		 * \brief Gets the value
		 */
		T get() const { return value; };
		/**
		 * \brief Sets the current value as promise
		 */
		bool set_promise();
		/**
		 * \brief Waits for the future value
		 */
		T get_future(chrono::milliseconds timeout = chrono::milliseconds(0));


#ifdef VAULTMP_DEBUG
		static void SetDebugHandler(Debug* debug);
#endif

};

#endif
