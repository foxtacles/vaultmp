#ifndef GUARDED_H
#define GUARDED_H

#include "vaultmp.hpp"
#include "Value.hpp"

/**
 * \brief A class for guarding a Value with a CriticalSection
 *
 * Derives from Value and CriticalSection
 */

template<typename T = void>
class Guarded : private Value<T>, private CriticalSection
{
	public:
		~Guarded() noexcept {};

		template<typename F>
		typename std::enable_if<!std::is_same<typename std::result_of<F(T&)>::type, void>::value, typename std::result_of<F(T&)>::type>::type Operate(F function) {
			CriticalLock lock(*this);
			return function(**this);
		}

		template<typename F>
		typename std::enable_if<std::is_same<typename std::result_of<F(T&)>::type, void>::value, void>::type Operate(F function) {
			CriticalLock lock(*this);
			function(**this);
		}
};

template<>
class Guarded<void> : private CriticalSection
{
	public:
		~Guarded() noexcept {};

		template<typename F>
		typename std::enable_if<!std::is_same<typename std::result_of<F()>::type, void>::value, typename std::result_of<F()>::type>::type Operate(F function) {
			CriticalLock lock(*this);
			return function();
		}

		template<typename F>
		typename std::enable_if<std::is_same<typename std::result_of<F()>::type, void>::value, void>::type Operate(F function) {
			CriticalLock lock(*this);
			function();
		}
};

static_assert(sizeof(Guarded<>) == sizeof(CriticalSection), ":(");

#endif
