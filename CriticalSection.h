#ifndef CRITICALSECTION_H
#define CRITICALSECTION_H

#include "vaultmp.h"

#include <mutex>
#include <chrono>
#include <thread>
#include <typeinfo>

#define CS_TIMEOUT     5000

class Debug;

using namespace std;

class CriticalSection
{

	private:
        recursive_timed_mutex cs;

#ifdef VAULTMP_DEBUG
		static Debug* debug;
		bool ndebug;
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

#ifdef VAULTMP_DEBUG
		void ToggleSectionDebug( bool toggle );
		static void SetDebugHandler( Debug* debug );
#endif

};

#endif
