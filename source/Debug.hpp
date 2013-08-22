#ifndef DEBUG_H
#define DEBUG_H

#include "vaultmp.hpp"
#include "CriticalSection.hpp"

#include <fstream>

template<typename T>
struct DebugInput;

class Debug : private CriticalSection
{
		template<typename T>
		friend struct DebugInput;

	private:
		Debug(const char* module);

		std::fstream vaultmplog;
		static std::unique_ptr<Debug> debug;

		static void GetTimeString(char* buf, unsigned int size, bool file);

		template <typename T, typename... Args>
		void Note(T&& arg, Args&&... args)
		{
			vaultmplog << std::forward<T>(arg);
			Note(std::forward<Args>(args)...);
		}

		void Note()
		{
			vaultmplog << std::endl;
		}

		template <typename... Args>
		void Print(Args&&... args)
		{
			char buf[32];
			Debug::GetTimeString(buf, sizeof(buf), false);
			vaultmplog << "[" << buf << "] ";

			Note(std::forward<Args>(args)...);
		}

		Debug(const Debug&) = delete;
		Debug& operator=(const Debug&) = delete;

	public:
		~Debug() noexcept;

		static void SetDebugHandler(const char* module);
};

template<typename T>
struct DebugInput
{
	public:
		template <typename... Args>
		const DebugInput& note(Args&&... args) const
		{
			if (Debug::debug)
			{
				Debug::debug->StartSession();
				Debug::debug->Note(std::forward<Args>(args)...);
				Debug::debug->EndSession();
			}

			return *this;
		}

		template <typename... Args>
		const DebugInput& print(Args&&... args) const
		{
			if (Debug::debug)
			{
				Debug::debug->StartSession();
				Debug::debug->Print(std::forward<Args>(args)...);
				Debug::debug->EndSession();
			}

			return *this;
		}
};

#endif
