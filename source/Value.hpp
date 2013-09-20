#ifndef VALUE_H
#define VALUE_H

#include "vaultmp.hpp"
#include "Lockable.hpp"

#ifdef VAULTMP_DEBUG
#include "Debug.hpp"
#endif

/**
 * \brief A container class which simply stores a variable of type T
 *
 * Derives from Lockable to lock / unlock the data member
 */

template<typename T>
class Value : public Lockable
{
	private:
		T value;

#ifdef VAULTMP_DEBUG
		static DebugInput<Value<T>> debug;
#endif

		Value(const Value&) = delete;
		Value& operator=(const Value&) = delete;

	public:
		Value() : value() {};
		Value(const T& t) : value(t) {};
		Value(Value&&) = default;
		Value& operator=(Value&&) = default;
		virtual ~Value() {}

		/**
		 * \brief Sets the value
		 */
		bool set(const T& value)
		{
			if (this->IsLocked())
				return false;

			this->value = value;

			return true;
		}
		/**
		 * \brief Gets the value (a copy)
		 */
		T get() const { return value; }
		/**
		 * \brief Returns a reference to the value
		 */
		T& operator*() { return value; }
		/**
		 * \brief Returns a reference to the value (const)
		 */
		const T& operator*() const { return value; }
		/**
		 * \brief Returns a pointer to the value
		 */
		T* operator->() { return &value; }
};

#ifdef VAULTMP_DEBUG
template <typename T>
DebugInput<Value<T>> Value<T>::debug;
#endif

#endif
