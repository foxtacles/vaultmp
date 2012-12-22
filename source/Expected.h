#ifndef EXPECTED_H
#define EXPECTED_H

#include "vaultmp.h"
#include "VaultException.h"

/**
 * \brief http://channel9.msdn.com/Shows/Going+Deep/C-and-Beyond-2012-Andrei-Alexandrescu-Systematic-Error-Handling-in-C
 */

template <typename T>
class Expected
{
	private:
		union {
			T value;
			std::exception_ptr exception;
		};

		bool valid;

	public:
		Expected() : value(T()), valid(true) {}
		Expected(const T& value) : value(value), valid(true) {}
		Expected(T&& value) : value(std::move(value)), valid(true) {}
		Expected(const Expected& expected) : valid(expected.valid)
		{
			if (valid)
				new(&value) T(expected.value);
			else
				new(&exception) std::exception_ptr(expected.exception);
		}
		Expected(Expected&& expected) : valid(expected.valid)
		{
			if (valid)
				new(&value) T(std::move(expected.value));
			else
				new(&exception) std::exception_ptr(std::move(expected.exception));
		}
		Expected& operator=(const Expected& expected)
		{
			Expected tmp(expected);
			tmp.swap(*this);
			return *this;
		}
		Expected(const VaultException& exception) : exception(std::make_exception_ptr(exception)), valid(false) {}
		~Expected()
		{
			using std::exception_ptr;

			if (valid)
				value.~T();
			else
				exception.~exception_ptr();
		}

		void swap(Expected& expected)
		{
			if (valid)
			{
				if (expected.valid)
				{
					using std::swap;
					swap(value, expected.value);
				}
				else
				{
					auto t = std::move(expected.exception);
					new(&expected.value) T(std::move(value));
					new(&exception) std::exception_ptr(t);
					std::swap(valid, expected.valid);
				}
			}
			else
			{
				if (expected.valid)
					expected.swap(*this);
				else
					exception.swap(expected.exception);
			}
		}

		T& get()
		{
			if (valid)
				return value;
			else
				std::rethrow_exception(exception);
		}

		template<typename U>
		struct result_star_operator
		{
			template<typename Q> static decltype(Q().operator*()) test(decltype(&Q::operator*));
			template<typename> static void test(...);
			typedef decltype(test<U>(0)) type;
		};

		template<typename U>
		struct result_pointer_operator
		{
			template<typename Q> static decltype(Q().operator->()) test(decltype(&Q::operator->));
			template<typename> static void test(...);
			typedef decltype(test<U>(0)) type;
		};

		explicit operator bool() const { return valid && const_cast<Expected*>(this)->get(); }

		template<typename U> typename std::enable_if<std::is_class<U>::value, typename result_star_operator<U>::type>::type star_operator(U& u) { return u.operator*(); }
		template<typename U> typename std::enable_if<!std::is_class<U>::value, U&>::type star_operator(U& u) { return u; }
		auto operator*() -> decltype(this->star_operator<T>(value)) { return star_operator<T>(get()); }

		template<typename U> typename std::enable_if<std::is_class<U>::value, typename result_pointer_operator<U>::type>::type pointer_operator(U& u) { return u.operator->(); }
		template<typename U> typename std::enable_if<!std::is_class<U>::value && std::is_pointer<U>::value, U>::type pointer_operator(U& u) { return u; }
		auto operator->() -> decltype(this->pointer_operator<T>(value)) { return pointer_operator<T>(get()); }
};

#endif
