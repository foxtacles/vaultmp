#ifndef SHARED_H
#define SHARED_H

#include "vaultmp.h"
#include "Value.h"

#include <chrono>
#include <future>

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
		bool set_promise()
		{
			try
			{
				this->async.set_value(move(**this));

		#ifdef VAULTMP_DEBUG
				if (debug)
					debug->PrintFormat("Satisfied promise (%08X -> %08X)", true, this, &this->async);
		#endif
			}
			catch (exception& e)
			{
				throw VaultException("Failed setting promise (%08X -> %08X: %s)", this, &this->async, e.what());
			}

			return true;
		}
		/**
		 * \brief Waits for the future value
		 */
		T get_future(chrono::milliseconds timeout = chrono::milliseconds(0))
		{
			future<T> f = this->async.get_future();

			if (timeout > chrono::milliseconds(0))
				if (f.wait_for(timeout) == future_status::timeout)
					throw VaultException("Timeout of %d reached for future value retrieval", timeout.count());

			T value = f.get();
			this->async = promise<T>();

			return value;
		}

#ifdef VAULTMP_DEBUG
		static void SetDebugHandler(Debug* debug)
		{
			Shared<T>::debug = debug;

			if (debug)
				debug->PrintFormat("Attached debug handler to Shared<%s> class", true, typeid(T).name());
		}
#endif
};

#ifdef VAULTMP_DEBUG
template <typename T>
Debug* Shared<T>::debug;
#endif

#endif
