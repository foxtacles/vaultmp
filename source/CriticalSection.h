#ifndef CRITICALSECTION_H
#define CRITICALSECTION_H

#include "vaultmp.h"

#include <mutex>
#include <chrono>
#include <thread>
#include <typeinfo>

#ifdef VAULTMP_DEBUG
// only in debug, streams blow the executable size
#include <sstream>
#endif

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

		CriticalSection(const CriticalSection&);
		CriticalSection& operator=(const CriticalSection&);

	public:
#ifdef VAULTMP_DEBUG
		CriticalSection() : debug(nullptr), finalize(false) {}
#else
		CriticalSection() : finalize(false) {}
#endif
		virtual ~CriticalSection() {}

		CriticalSection* StartSession();
		void EndSession();
		void Finalize();

#ifdef VAULTMP_DEBUG
		static string thread_id(thread&);
		static string thread_id();
		void PrintStatus();
		void SetDebugHandler(Debug* debug);
#endif

};

#endif
