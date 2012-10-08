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

class CriticalSection
{
	private:
		std::recursive_timed_mutex cs;

#ifdef VAULTMP_DEBUG
		Debug* debug;
#endif

		bool finalize;

		CriticalSection(const CriticalSection&) = delete;
		CriticalSection& operator=(const CriticalSection&) = delete;

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
		static std::string thread_id(std::thread&);
		static std::string thread_id();
		void PrintStatus();
		void SetDebugHandler(Debug* debug);
#endif

};

#endif
