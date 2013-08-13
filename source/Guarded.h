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
class Guarded : private Value<T>, private CriticalSection
{
	public:
		virtual ~Guarded() noexcept {};

		template<typename F>
		typename std::enable_if<!std::is_same<typename std::result_of<F(T&)>::type, void>::value, typename std::result_of<F(T&)>::type>::type Operate(F function) {
			CriticalLock lock(*this);
			auto&& result = function(**this);
			return std::forward<typename std::remove_reference<typename std::result_of<F(T&)>::type>::type>(result);
		}

		template<typename F>
		typename std::enable_if<std::is_same<typename std::result_of<F(T&)>::type, void>::value, void>::type Operate(F function) {
			CriticalLock lock(*this);
			function(**this);
		}
};

#endif
