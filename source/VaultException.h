#ifndef VAULTEXCEPTION_H
#define VAULTEXCEPTION_H

#ifdef __WIN32__
#include <winsock2.h>
#endif

#include <cstdio>
#include <string>
#include <typeinfo>

#include "vaultmp.h"
#include "Debug.h"

#ifdef VAULTMP_DEBUG
#include "dbg/stack.hpp"
#include <ostream>
#endif

using namespace std;

/**
 * \brief The exception class of vaultmp
 *
 * Stores an exception message and is able to print it to either a MessageBox or to the console
 */

class VaultException : public exception
{
	private:
		string error;

#ifdef VAULTMP_DEBUG
		static Debug* debug;
		static void stacktrace();
#endif

		VaultException& operator=(const VaultException&) = delete;

	public:
		VaultException(string error);
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

		virtual const char* what() const throw();

#ifdef VAULTMP_DEBUG
		static void SetDebugHandler(Debug* debug);
		static void FinalizeDebug();
#endif

};

#endif
