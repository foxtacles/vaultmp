#ifndef CRITICALSECTION_H
#define CRITICALSECTION_H

#include "vaultmp.h"

#include <mutex>
#include <chrono>
#include <thread>
#include <typeinfo>
#include <atomic>

#ifdef VAULTMP_DEBUG
// only in debug, streams blow the executable size
#include <sstream>
#endif

#define CS_TIMEOUT     5000

class CriticalSection
{
	private:
		std::recursive_timed_mutex cs;
		std::atomic<bool> finalize;

		CriticalSection(const CriticalSection&) = delete;
		CriticalSection& operator=(const CriticalSection&) = delete;

	public:
		CriticalSection() : finalize(false) {}
		virtual ~CriticalSection() {}

		CriticalSection* StartSession();
		void EndSession();
		void Finalize();

#ifdef VAULTMP_DEBUG
		static std::string thread_id(std::thread&);
		static std::string thread_id();
#endif

};

#endif
