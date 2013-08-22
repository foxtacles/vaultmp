#ifndef SHARED_H
#define SHARED_H

#include "vaultmp.hpp"
#include "Value.hpp"
#include "VaultException.hpp"

#ifdef VAULTMP_DEBUG
#include "Debug.hpp"
#endif

#include <future>

/**
 * \brief An extension of the Value class to enable sharing of the value
 *
 * Derives from Value
 */

template <typename T>
class Shared : public Value<T>
{
	private:
		std::promise<T> async;

#ifdef VAULTMP_DEBUG
		static DebugInput<Shared<T>> debug;
#endif

		Shared& operator=(const Shared&) = delete;

	public:
		Shared() : Value<T>() {};
		Shared(const T& t) : Value<T>(t) {};
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
				this->async.set_value(std::move(**this));

#ifdef VAULTMP_DEBUG
				debug.print("Satisfied promise (", std::hex, this, " -> ", &this->async, ")");
#endif
			}
			catch (std::exception& e)
			{
				throw VaultException("Failed setting promise (%08X -> %08X: %s)", this, &this->async, e.what()).stacktrace();
			}

			return true;
		}
		/**
		 * \brief Waits for the future value
		 */
		T get_future(std::chrono::milliseconds timeout = std::chrono::milliseconds(0))
		{
			std::future<T> f = this->async.get_future();

			if (timeout > std::chrono::milliseconds(0))
				if (f.wait_for(timeout) == std::future_status::timeout)
					throw VaultException("Timeout of %d reached for future value retrieval", timeout.count()).stacktrace();

			T value = f.get();
			this->async = std::promise<T>();

			return value;
		}
};

#ifdef VAULTMP_DEBUG
template <typename T>
DebugInput<Shared<T>> Shared<T>::debug;
#endif

#endif
