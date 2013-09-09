#ifndef CRITICALSECTION_H
#define CRITICALSECTION_H

#include "vaultmp.hpp"

#include <mutex>
#include <atomic>

#ifdef VAULTMP_DEBUG
#include <thread>
#endif

class CriticalSection
{
	private:
		std::recursive_mutex cs;
		std::atomic<bool> finalize;

		CriticalSection(const CriticalSection&) = delete;
		CriticalSection& operator=(const CriticalSection&) = delete;

	public:
		CriticalSection() noexcept : finalize(false) {}
		~CriticalSection() noexcept {}

		CriticalSection* StartSession() noexcept
		{
			if (finalize)
				return nullptr;

			cs.lock();

			if (!finalize)
				return this;

			cs.unlock();

			return nullptr;
		}

		void EndSession() noexcept { cs.unlock(); }

		void Finalize() noexcept // must be called by the thread which wants to delete this object
		{
			finalize = true;
			EndSession();
		}

#ifdef VAULTMP_DEBUG
		static std::string thread_id(std::thread&);
		static std::string thread_id();
#endif
};

class CriticalLock
{
	private:
		CriticalSection* lock;

		CriticalLock(const CriticalLock&) = delete;
		CriticalLock& operator=(const CriticalLock&) = delete;

	public:
		CriticalLock(CriticalSection& lock) noexcept : lock(lock.StartSession()) {}
		~CriticalLock() noexcept { if (lock) lock->EndSession(); }
};

#endif
