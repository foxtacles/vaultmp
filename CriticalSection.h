#ifndef CRITICALSECTION_H
#define CRITICALSECTION_H

#include "vaultmp.h"

#include <mutex>
#include <chrono>
#include <thread>
#include <typeinfo>
#include <sstream>

#define CS_TIMEOUT     5000

class Debug;

using namespace std;

class CriticalSection
{

	private:
        recursive_timed_mutex cs;

#ifdef VAULTMP_DEBUG
		Debug* debug;
#endif

		bool finalize;
		unsigned int locks;

		CriticalSection( const CriticalSection& );
		CriticalSection& operator=( const CriticalSection& );

	public:
		CriticalSection();
		virtual ~CriticalSection();

		CriticalSection* StartSession();
		void EndSession();
		void Finalize();

        static string thread_id(thread&);
        static string thread_id();

#ifdef VAULTMP_DEBUG
		void SetDebugHandler( Debug* debug );
#endif

};

#endif
