#ifndef VAULTEXCEPTION_H
#define VAULTEXCEPTION_H

#include "vaultmp.hpp"

#ifdef VAULTMP_DEBUG
#include "Debug.hpp"
#endif

#include <stdexcept>

/**
 * \brief The exception class of vaultmp
 *
 * Stores an exception message and is able to print it to either a MessageBox or to the console
 */

class VaultException : public std::exception
{
	private:
		std::string error;

#ifdef VAULTMP_DEBUG
		static DebugInput<VaultException> debug;
		static void stacktrace_();
#endif

		VaultException& operator=(const VaultException&) = delete;

	public:
		VaultException(const std::string& error);
		VaultException(const char* format, ...);
		virtual ~VaultException() throw() {}

		/**
		 * \brief Creates a MessageBox displaying the exception message (Win32 only)
		 */
		void Message();
		/**
		 * \brief Prints the exception message to the console
		 */
		void Console();
		/**
		 * \brief Prints a stacktrace
		 */
		VaultException& stacktrace();

		virtual const char* what() const throw();
};

#endif
