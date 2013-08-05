#ifndef CRITICALSECTION_H
#define CRITICALSECTION_H

#include "vaultmp.h"

#include <mutex>
#include <atomic>

#ifdef VAULTMP_DEBUG
#include <thread>
#endif

class CriticalSection
{
	private:
		static constexpr unsigned int CS_TIMEOUT = 5000;

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
