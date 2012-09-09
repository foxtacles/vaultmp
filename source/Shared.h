#ifndef SHARED_H
#define SHARED_H

#include "vaultmp.h"
#include "Value.h"

#include <chrono>
#include <future>

#include <set>

#ifdef VAULTMP_DEBUG
#include "Debug.h"
#endif

using namespace std;

/**
 * \brief An extension of the Value class to enable sharing of the value
 *
 * Derives from Value
 */

template <typename T>
class Shared : public Value<T>
{
	private:
		promise<T> async;

#ifdef VAULTMP_DEBUG
		static Debug* debug;
#endif

		Shared& operator=(const Shared&);

	public:
		Shared() : Value<T>() {};
		Shared(T t) : Value<T>(t) {};
		Shared(Shared&&) = default;
		Shared& operator=(Shared&&) = default;
		virtual ~Shared() {}

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
